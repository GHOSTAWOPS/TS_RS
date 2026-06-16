#include "geometry/occt/OcctGeometryEngine.h"

#include "geometry/kernel/IGeometryEngine.h"

#include <BRepBuilderAPI_MakeEdge.hxx>
#include <TopoDS_Edge.hxx>
#include <gp_Pnt.hxx>

#include <cmath>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace {

bool nearlyEqual(double lhs, double rhs, double tolerance = 1.0e-9)
{
    return std::fabs(lhs - rhs) <= tolerance;
}

int fail(const std::string& message)
{
    std::cerr << message << '\n';
    return 1;
}

TopoDS_Edge makeLineEdge(double startX, double startY, double startZ,
                         double endX, double endY, double endZ)
{
    return BRepBuilderAPI_MakeEdge(gp_Pnt(startX, startY, startZ),
                                   gp_Pnt(endX, endY, endZ));
}

template <typename TResult>
int expectDiagnostic(const std::string& name,
                     const TResult& result,
                     std::string_view expectedDiagnosticCode)
{
    if (result.ok) {
        return fail(name + ": expected failure");
    }
    if (result.diagnosticCode != expectedDiagnosticCode) {
        return fail(name + ": expected " + std::string{expectedDiagnosticCode} + ", got "
                    + result.diagnosticCode);
    }
    return 0;
}

int expectLength(const tsrs::geometry::GeometryLengthResult& result, double expectedLengthM)
{
    if (!result.ok) {
        return fail("expected length ok, got " + result.diagnosticCode);
    }
    if (result.diagnosticCode != tsrs::geometry::kGeometryDiagnosticOk) {
        return fail("expected GEOM_OK length diagnostic");
    }
    if (!nearlyEqual(result.lengthM, expectedLengthM)) {
        return fail("unexpected curve length");
    }
    return 0;
}

int expectPoint(const tsrs::geometry::GeometryPointResult& result,
                double expectedX,
                double expectedY,
                double expectedZ)
{
    if (!result.ok) {
        return fail("expected point ok, got " + result.diagnosticCode);
    }
    if (result.diagnosticCode != tsrs::geometry::kGeometryDiagnosticOk) {
        return fail("expected GEOM_OK point diagnostic");
    }
    if (!nearlyEqual(result.point.x, expectedX) ||
        !nearlyEqual(result.point.y, expectedY) ||
        !nearlyEqual(result.point.z, expectedZ)) {
        return fail("unexpected point coordinates");
    }
    return 0;
}

int expectVector(const tsrs::geometry::GeometryVectorResult& result,
                 double expectedX,
                 double expectedY,
                 double expectedZ)
{
    if (!result.ok) {
        return fail("expected vector ok, got " + result.diagnosticCode);
    }
    if (result.diagnosticCode != tsrs::geometry::kGeometryDiagnosticOk) {
        return fail("expected GEOM_OK vector diagnostic");
    }
    const double vectorLength = std::sqrt(result.vector.x * result.vector.x +
                                          result.vector.y * result.vector.y +
                                          result.vector.z * result.vector.z);
    if (!nearlyEqual(vectorLength, 1.0)) {
        return fail("expected unit tangent vector");
    }
    if (!nearlyEqual(result.vector.x, expectedX) ||
        !nearlyEqual(result.vector.y, expectedY) ||
        !nearlyEqual(result.vector.z, expectedZ)) {
        return fail("unexpected vector coordinates");
    }
    return 0;
}

} // namespace

int main()
{
    using tsrs::geometry::GeometryEntityKind;
    using tsrs::geometry::GeometryPoint3d;
    using tsrs::geometry::GeometryRef;

    tsrs::geometry::occt::OcctGeometryEngine engine;
    const GeometryRef lineRef =
        engine.registerEdge("line-x-10", makeLineEdge(0.0, 0.0, 0.0, 10.0, 0.0, 0.0));
    const GeometryRef surfaceRef{GeometryEntityKind::Surface, "line-x-10"};

    if (lineRef.kind != GeometryEntityKind::Curve || lineRef.stableId != "line-x-10") {
        return fail("registered edge should return the requested curve ref");
    }

    if (const int code = expectLength(engine.curveLength(lineRef), 10.0)) {
        return code;
    }

    if (const int code = expectPoint(engine.pointAtLength(lineRef, 4.5), 4.5, 0.0, 0.0)) {
        return code;
    }

    if (const int code = expectVector(engine.tangentAtLength(lineRef, 4.5), 1.0, 0.0, 0.0)) {
        return code;
    }

    const auto polyline = engine.makePolylineCurve({
        GeometryPoint3d{0.0, 0.0, 0.0},
        GeometryPoint3d{1.0, 0.0, 0.0},
        GeometryPoint3d{1.0, 1.0, 0.0},
    });
    if (!polyline.ok) {
        return fail("expected polyline ok, got " + polyline.diagnosticCode);
    }
    if (polyline.diagnosticCode != tsrs::geometry::kGeometryDiagnosticOk) {
        return fail("expected polyline GEOM_OK diagnostic");
    }
    if (polyline.ref.kind != GeometryEntityKind::Curve || polyline.ref.stableId.empty()) {
        return fail("expected polyline curve ref");
    }
    if (const int code = expectLength(engine.curveLength(polyline.ref), 2.0)) {
        return code;
    }
    if (const int code = expectPoint(engine.pointAtLength(polyline.ref, 1.5), 1.0, 0.5, 0.0)) {
        return code;
    }
    if (const int code = expectVector(engine.tangentAtLength(polyline.ref, 1.5), 0.0, 1.0, 0.0)) {
        return code;
    }

    if (const int code = expectDiagnostic("wrong kind",
                                          engine.curveLength(surfaceRef),
                                          tsrs::geometry::kGeometryDiagnosticWrongEntityKind)) {
        return code;
    }
    if (const int code = expectDiagnostic(
            "missing ref",
            engine.curveLength(GeometryRef{GeometryEntityKind::Curve, "missing"}),
            tsrs::geometry::kGeometryDiagnosticMissingRef)) {
        return code;
    }
    if (const int code = expectDiagnostic("length out of range",
                                          engine.pointAtLength(lineRef, 11.0),
                                          tsrs::geometry::kGeometryDiagnosticLengthOutOfRange)) {
        return code;
    }
    if (const int code = expectDiagnostic("invalid polyline",
                                          engine.makePolylineCurve({GeometryPoint3d{0.0, 0.0, 0.0}}),
                                          tsrs::geometry::kGeometryDiagnosticInvalidInput)) {
        return code;
    }

    return 0;
}
