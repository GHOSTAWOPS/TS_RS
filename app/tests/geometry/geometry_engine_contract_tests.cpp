#include "geometry/kernel/IGeometryEngine.h"

#include <cmath>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

bool nearlyEqual(double lhs, double rhs)
{
    return std::fabs(lhs - rhs) < 1.0e-12;
}

int fail(const std::string& message)
{
    std::cerr << message << '\n';
    return 1;
}

class MockGeometryEngine final : public tsrs::geometry::IGeometryEngine {
public:
    void addStraightCurve(std::string stableId, double lengthM)
    {
        curves_.emplace(std::move(stableId), lengthM);
    }

    tsrs::geometry::GeometryLengthResult curveLength(
        const tsrs::geometry::GeometryRef& curveRef) const override
    {
        if (curveRef.kind != tsrs::geometry::GeometryEntityKind::Curve) {
            return {false, 0.0, std::string{tsrs::geometry::kGeometryDiagnosticWrongEntityKind}};
        }

        const auto found = curves_.find(curveRef.stableId);
        if (found == curves_.end()) {
            return {false, 0.0, std::string{tsrs::geometry::kGeometryDiagnosticMissingRef}};
        }

        return {true, found->second, std::string{tsrs::geometry::kGeometryDiagnosticOk}};
    }

    tsrs::geometry::GeometryPointResult pointAtLength(
        const tsrs::geometry::GeometryRef& curveRef,
        double lengthM) const override
    {
        const auto length = curveLength(curveRef);
        if (!length.ok) {
            return {false, {}, length.diagnosticCode};
        }

        if (lengthM < 0.0 || lengthM > length.lengthM) {
            return {false, {}, std::string{tsrs::geometry::kGeometryDiagnosticLengthOutOfRange}};
        }

        const auto polylineFound = polylinesByRef_.find(curveRef.stableId);
        if (polylineFound != polylinesByRef_.end()) {
            return {
                true,
                pointOnPolyline(polylineFound->second, lengthM),
                std::string{tsrs::geometry::kGeometryDiagnosticOk},
            };
        }

        return {
            true,
            tsrs::geometry::GeometryPoint3d{lengthM, 0.0, 0.0},
            std::string{tsrs::geometry::kGeometryDiagnosticOk},
        };
    }

    tsrs::geometry::GeometryVectorResult tangentAtLength(
        const tsrs::geometry::GeometryRef& curveRef,
        double lengthM) const override
    {
        const auto point = pointAtLength(curveRef, lengthM);
        if (!point.ok) {
            return {false, {}, point.diagnosticCode};
        }

        return {
            true,
            tsrs::geometry::GeometryVector3d{1.0, 0.0, 0.0},
            std::string{tsrs::geometry::kGeometryDiagnosticOk},
        };
    }

    tsrs::geometry::GeometryRefResult makePolylineCurve(
        std::vector<tsrs::geometry::GeometryPoint3d> points) override
    {
        if (points.size() < 2) {
            return {false, {}, std::string{tsrs::geometry::kGeometryDiagnosticInvalidInput}};
        }

        const double lengthM = polylineLength(points);
        polylines_.push_back(std::move(points));
        const std::string stableId = "mock-polyline-" + std::to_string(polylines_.size());
        curves_.emplace(stableId, lengthM);
        polylinesByRef_.emplace(stableId, polylines_.back());
        return {
            true,
            tsrs::geometry::GeometryRef{
                tsrs::geometry::GeometryEntityKind::Curve,
                stableId,
            },
            std::string{tsrs::geometry::kGeometryDiagnosticOk},
        };
    }

    const std::vector<std::vector<tsrs::geometry::GeometryPoint3d>>& polylines() const
    {
        return polylines_;
    }

private:
    static double distanceBetween(tsrs::geometry::GeometryPoint3d lhs,
                                  tsrs::geometry::GeometryPoint3d rhs)
    {
        const double dx = lhs.x - rhs.x;
        const double dy = lhs.y - rhs.y;
        const double dz = lhs.z - rhs.z;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    static double polylineLength(const std::vector<tsrs::geometry::GeometryPoint3d>& points)
    {
        double lengthM = 0.0;
        for (std::size_t index = 1; index < points.size(); ++index) {
            lengthM += distanceBetween(points[index - 1], points[index]);
        }
        return lengthM;
    }

    static tsrs::geometry::GeometryPoint3d pointOnPolyline(
        const std::vector<tsrs::geometry::GeometryPoint3d>& points,
        double lengthM)
    {
        double remainingM = lengthM;
        for (std::size_t index = 1; index < points.size(); ++index) {
            const auto start = points[index - 1];
            const auto end = points[index];
            const double segmentLengthM = distanceBetween(start, end);
            if (remainingM <= segmentLengthM) {
                const double ratio = segmentLengthM == 0.0 ? 0.0 : remainingM / segmentLengthM;
                return tsrs::geometry::GeometryPoint3d{
                    start.x + (end.x - start.x) * ratio,
                    start.y + (end.y - start.y) * ratio,
                    start.z + (end.z - start.z) * ratio,
                };
            }
            remainingM -= segmentLengthM;
        }

        return points.back();
    }

    std::map<std::string, double> curves_;
    std::vector<std::vector<tsrs::geometry::GeometryPoint3d>> polylines_;
    std::map<std::string, std::vector<tsrs::geometry::GeometryPoint3d>> polylinesByRef_;
};

int expectLength(const tsrs::geometry::GeometryLengthResult& result, double expectedLengthM)
{
    if (!result.ok) {
        return fail("expected length ok, got " + result.diagnosticCode);
    }

    if (result.diagnosticCode != tsrs::geometry::kGeometryDiagnosticOk) {
        return fail("expected geometry ok diagnostic");
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
        return fail("expected point GEOM_OK diagnostic");
    }

    if (!nearlyEqual(result.point.x, expectedX)
        || !nearlyEqual(result.point.y, expectedY)
        || !nearlyEqual(result.point.z, expectedZ)) {
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
        return fail("expected vector GEOM_OK diagnostic");
    }

    const double vectorLength =
        std::sqrt(result.vector.x * result.vector.x + result.vector.y * result.vector.y
                  + result.vector.z * result.vector.z);
    if (!nearlyEqual(vectorLength, 1.0)) {
        return fail("expected unit tangent vector");
    }

    if (!nearlyEqual(result.vector.x, expectedX)
        || !nearlyEqual(result.vector.y, expectedY)
        || !nearlyEqual(result.vector.z, expectedZ)) {
        return fail("unexpected vector coordinates");
    }

    return 0;
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

} // namespace

int main()
{
    using tsrs::geometry::GeometryEntityKind;
    using tsrs::geometry::GeometryPoint3d;
    using tsrs::geometry::GeometryRef;

    std::unique_ptr<tsrs::geometry::IGeometryEngine> engine =
        std::make_unique<MockGeometryEngine>();
    auto& mock = static_cast<MockGeometryEngine&>(*engine);
    mock.addStraightCurve("guide-curve-a", 10.0);

    const GeometryRef guideCurve{GeometryEntityKind::Curve, "guide-curve-a"};
    const GeometryRef guideFace{GeometryEntityKind::Surface, "guide-face-a"};

    if (const int code = expectLength(engine->curveLength(guideCurve), 10.0)) {
        return code;
    }

    if (const int code = expectPoint(engine->pointAtLength(guideCurve, 4.5), 4.5, 0.0, 0.0)) {
        return code;
    }

    if (const int code = expectVector(engine->tangentAtLength(guideCurve, 4.5), 1.0, 0.0, 0.0)) {
        return code;
    }

    const auto centerline = engine->makePolylineCurve({
        GeometryPoint3d{0.0, 0.0, 0.0},
        GeometryPoint3d{1.0, 0.0, 0.0},
        GeometryPoint3d{1.0, 1.0, 0.0},
    });
    if (!centerline.ok) {
        return fail("expected makePolylineCurve ok, got " + centerline.diagnosticCode);
    }
    if (centerline.diagnosticCode != tsrs::geometry::kGeometryDiagnosticOk) {
        return fail("expected polyline GEOM_OK diagnostic");
    }
    if (centerline.ref.kind != GeometryEntityKind::Curve
        || centerline.ref.stableId != "mock-polyline-1") {
        return fail("unexpected polyline curve ref");
    }
    if (mock.polylines().size() != 1 || mock.polylines().front().size() != 3) {
        return fail("mock did not record polyline points");
    }
    if (const int code = expectLength(engine->curveLength(centerline.ref), 2.0)) {
        return code;
    }
    if (const int code = expectPoint(engine->pointAtLength(centerline.ref, 1.5), 1.0, 0.5, 0.0)) {
        return code;
    }

    if (const int code = expectDiagnostic("wrong kind",
                                          engine->curveLength(guideFace),
                                          tsrs::geometry::kGeometryDiagnosticWrongEntityKind)) {
        return code;
    }

    if (const int code = expectDiagnostic(
            "missing ref",
            engine->curveLength(GeometryRef{GeometryEntityKind::Curve, "missing"}),
            tsrs::geometry::kGeometryDiagnosticMissingRef)) {
        return code;
    }

    if (const int code = expectDiagnostic("length out of range",
                                          engine->pointAtLength(guideCurve, 11.0),
                                          tsrs::geometry::kGeometryDiagnosticLengthOutOfRange)) {
        return code;
    }

    if (const int code = expectDiagnostic("invalid polyline",
                                          engine->makePolylineCurve({GeometryPoint3d{0.0, 0.0, 0.0}}),
                                          tsrs::geometry::kGeometryDiagnosticInvalidInput)) {
        return code;
    }

    return 0;
}
