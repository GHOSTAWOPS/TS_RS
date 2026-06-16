#include "geometry/occt/OcctGeometryEngine.h"

#include <BRepAdaptor_Curve.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <Precision.hxx>
#include <Standard_Failure.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

#include <algorithm>
#include <cmath>
#include <sstream>
#include <utility>

namespace {

bool finite(double value)
{
    return std::isfinite(value);
}

bool finitePoint(tsrs::geometry::GeometryPoint3d point)
{
    return finite(point.x) && finite(point.y) && finite(point.z);
}

gp_Pnt toGpPoint(tsrs::geometry::GeometryPoint3d point)
{
    return gp_Pnt(point.x, point.y, point.z);
}

tsrs::geometry::GeometryPoint3d toGeometryPoint(const gp_Pnt& point)
{
    return {point.X(), point.Y(), point.Z()};
}

tsrs::geometry::GeometryVector3d toGeometryVector(const gp_Vec& vector)
{
    return {vector.X(), vector.Y(), vector.Z()};
}

double measureEdgeLength(const TopoDS_Edge& edge)
{
    if (edge.IsNull()) {
        return 0.0;
    }

    BRepAdaptor_Curve curve(edge);
    const double first = curve.FirstParameter();
    const double last = curve.LastParameter();
    if (!finite(first) || !finite(last) || first >= last) {
        return 0.0;
    }

    double lengthM = GCPnts_AbscissaPoint::Length(curve, first, last, Precision::Confusion());
    if (!finite(lengthM) || lengthM < 0.0) {
        return 0.0;
    }
    return lengthM;
}

double parameterAtLength(const TopoDS_Edge& edge, double targetLengthM)
{
    BRepAdaptor_Curve curve(edge);
    const double first = curve.FirstParameter();
    const double last = curve.LastParameter();

    if (targetLengthM <= Precision::Confusion()) {
        return first;
    }

    double low = first;
    double high = last;
    for (int iteration = 0; iteration < 64; ++iteration) {
        const double mid = low + (high - low) * 0.5;
        const double midLength =
            GCPnts_AbscissaPoint::Length(curve, first, mid, Precision::Confusion());
        if (midLength < targetLengthM) {
            low = mid;
        } else {
            high = mid;
        }
    }

    return low + (high - low) * 0.5;
}

} // namespace

namespace tsrs::geometry::occt {

GeometryRef OcctGeometryEngine::registerEdge(std::string stableId, const TopoDS_Edge& edge)
{
    if (stableId.empty()) {
        stableId = nextGeneratedStableId();
    }

    CurveRecord record;
    const double lengthM = measureEdgeLength(edge);
    if (!edge.IsNull() && lengthM > Precision::Confusion()) {
        record.segments.push_back(Segment{edge, lengthM});
        record.totalLengthM = lengthM;
        curves_[stableId] = std::move(record);
    }

    return makeCurveRef(stableId);
}

GeometryLengthResult OcctGeometryEngine::curveLength(const GeometryRef& curveRef) const
{
    if (curveRef.kind != GeometryEntityKind::Curve) {
        return {false, 0.0, std::string{kGeometryDiagnosticWrongEntityKind}};
    }

    const auto found = curves_.find(curveRef.stableId);
    if (found == curves_.end()) {
        return {false, 0.0, std::string{kGeometryDiagnosticMissingRef}};
    }

    return {true, found->second.totalLengthM, std::string{kGeometryDiagnosticOk}};
}

GeometryPointResult OcctGeometryEngine::pointAtLength(const GeometryRef& curveRef,
                                                      double lengthM) const
{
    if (curveRef.kind != GeometryEntityKind::Curve) {
        return {false, {}, std::string{kGeometryDiagnosticWrongEntityKind}};
    }
    if (!finite(lengthM)) {
        return {false, {}, std::string{kGeometryDiagnosticInvalidInput}};
    }

    const auto found = curves_.find(curveRef.stableId);
    if (found == curves_.end()) {
        return {false, {}, std::string{kGeometryDiagnosticMissingRef}};
    }

    const CurveRecord& record = found->second;
    if (lengthM < -Precision::Confusion() || lengthM > record.totalLengthM + Precision::Confusion()) {
        return {false, {}, std::string{kGeometryDiagnosticLengthOutOfRange}};
    }

    double remainingM = std::clamp(lengthM, 0.0, record.totalLengthM);
    for (const Segment& segment : record.segments) {
        if (remainingM <= segment.lengthM + Precision::Confusion()) {
            const double parameter = parameterAtLength(segment.edge,
                                                       std::clamp(remainingM, 0.0, segment.lengthM));
            BRepAdaptor_Curve curve(segment.edge);
            return {
                true,
                toGeometryPoint(curve.Value(parameter)),
                std::string{kGeometryDiagnosticOk},
            };
        }
        remainingM -= segment.lengthM;
    }

    if (!record.segments.empty()) {
        BRepAdaptor_Curve curve(record.segments.back().edge);
        return {
            true,
            toGeometryPoint(curve.Value(curve.LastParameter())),
            std::string{kGeometryDiagnosticOk},
        };
    }

    return {false, {}, std::string{kGeometryDiagnosticMissingRef}};
}

GeometryVectorResult OcctGeometryEngine::tangentAtLength(const GeometryRef& curveRef,
                                                         double lengthM) const
{
    if (curveRef.kind != GeometryEntityKind::Curve) {
        return {false, {}, std::string{kGeometryDiagnosticWrongEntityKind}};
    }
    if (!finite(lengthM)) {
        return {false, {}, std::string{kGeometryDiagnosticInvalidInput}};
    }

    const auto found = curves_.find(curveRef.stableId);
    if (found == curves_.end()) {
        return {false, {}, std::string{kGeometryDiagnosticMissingRef}};
    }

    const CurveRecord& record = found->second;
    if (lengthM < -Precision::Confusion() || lengthM > record.totalLengthM + Precision::Confusion()) {
        return {false, {}, std::string{kGeometryDiagnosticLengthOutOfRange}};
    }

    double remainingM = std::clamp(lengthM, 0.0, record.totalLengthM);
    for (const Segment& segment : record.segments) {
        if (remainingM <= segment.lengthM + Precision::Confusion()) {
            const double parameter = parameterAtLength(segment.edge,
                                                       std::clamp(remainingM, 0.0, segment.lengthM));
            BRepAdaptor_Curve curve(segment.edge);
            gp_Pnt point;
            gp_Vec tangent;
            curve.D1(parameter, point, tangent);
            if (tangent.Magnitude() <= Precision::Confusion()) {
                return {false, {}, std::string{kGeometryDiagnosticInvalidInput}};
            }
            tangent.Normalize();
            return {true, toGeometryVector(tangent), std::string{kGeometryDiagnosticOk}};
        }
        remainingM -= segment.lengthM;
    }

    return {false, {}, std::string{kGeometryDiagnosticMissingRef}};
}

GeometryRefResult OcctGeometryEngine::makePolylineCurve(std::vector<GeometryPoint3d> points)
{
    if (points.size() < 2) {
        return {false, {}, std::string{kGeometryDiagnosticInvalidInput}};
    }

    CurveRecord record;
    for (std::size_t index = 1; index < points.size(); ++index) {
        if (!finitePoint(points[index - 1]) || !finitePoint(points[index])) {
            return {false, {}, std::string{kGeometryDiagnosticInvalidInput}};
        }

        try {
            BRepBuilderAPI_MakeEdge edgeMaker(toGpPoint(points[index - 1]), toGpPoint(points[index]));
            if (!edgeMaker.IsDone()) {
                return {false, {}, std::string{kGeometryDiagnosticInvalidInput}};
            }
            const TopoDS_Edge edge = edgeMaker.Edge();
            const double lengthM = measureEdgeLength(edge);
            if (lengthM <= Precision::Confusion()) {
                return {false, {}, std::string{kGeometryDiagnosticInvalidInput}};
            }
            record.segments.push_back(Segment{edge, lengthM});
            record.totalLengthM += lengthM;
        } catch (const Standard_Failure&) {
            return {false, {}, std::string{kGeometryDiagnosticInvalidInput}};
        } catch (...) {
            return {false, {}, std::string{kGeometryDiagnosticInvalidInput}};
        }
    }

    const std::string stableId = nextGeneratedStableId();
    curves_[stableId] = std::move(record);
    return {true, makeCurveRef(stableId), std::string{kGeometryDiagnosticOk}};
}

GeometryRef OcctGeometryEngine::makeCurveRef(const std::string& stableId) const
{
    return GeometryRef{GeometryEntityKind::Curve, stableId};
}

std::string OcctGeometryEngine::nextGeneratedStableId()
{
    std::ostringstream output;
    output << "occt-polyline-" << ++generatedCurveOrdinal_;
    return output.str();
}

} // namespace tsrs::geometry::occt
