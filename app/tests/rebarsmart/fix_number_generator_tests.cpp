#include "rebarsmart/generators/FixNumberGenerator.h"

#include <cmath>
#include <iostream>
#include <map>
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
    void setFailMakePolyline(bool value)
    {
        failMakePolyline_ = value;
    }

    void addStraightCurve(std::string stableId,
                          tsrs::geometry::GeometryPoint3d start,
                          tsrs::geometry::GeometryPoint3d end)
    {
        curves_.emplace(std::move(stableId), CurveRecord{start, end});
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
        return {true, distanceBetween(found->second.start, found->second.end),
                std::string{tsrs::geometry::kGeometryDiagnosticOk}};
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
        const auto found = curves_.find(curveRef.stableId);
        const double ratio = length.lengthM == 0.0 ? 0.0 : lengthM / length.lengthM;
        return {true,
                interpolate(found->second.start, found->second.end, ratio),
                std::string{tsrs::geometry::kGeometryDiagnosticOk}};
    }

    tsrs::geometry::GeometryVectorResult tangentAtLength(
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
        const auto found = curves_.find(curveRef.stableId);
        const auto vector = vectorBetween(found->second.start, found->second.end);
        const double magnitude = vectorMagnitude(vector);
        if (magnitude == 0.0) {
            return {false, {}, std::string{tsrs::geometry::kGeometryDiagnosticInvalidInput}};
        }
        return {true,
                tsrs::geometry::GeometryVector3d{
                    vector.x / magnitude,
                    vector.y / magnitude,
                    vector.z / magnitude,
                },
                std::string{tsrs::geometry::kGeometryDiagnosticOk}};
    }

    tsrs::geometry::GeometryRefResult makePolylineCurve(
        std::vector<tsrs::geometry::GeometryPoint3d> points) override
    {
        if (failMakePolyline_) {
            return {false, {}, std::string{tsrs::geometry::kGeometryDiagnosticInvalidInput}};
        }
        if (points.size() < 2) {
            return {false, {}, std::string{tsrs::geometry::kGeometryDiagnosticInvalidInput}};
        }

        madePolylines_.push_back(std::move(points));
        const std::string stableId = "generated-centerline-" + std::to_string(madePolylines_.size());
        curves_.emplace(stableId, CurveRecord{madePolylines_.back().front(),
                                              madePolylines_.back().back()});
        return {true,
                tsrs::geometry::GeometryRef{tsrs::geometry::GeometryEntityKind::Curve, stableId},
                std::string{tsrs::geometry::kGeometryDiagnosticOk}};
    }

    const std::vector<std::vector<tsrs::geometry::GeometryPoint3d>>& madePolylines() const
    {
        return madePolylines_;
    }

private:
    struct CurveRecord {
        tsrs::geometry::GeometryPoint3d start;
        tsrs::geometry::GeometryPoint3d end;
    };

    static tsrs::geometry::GeometryVector3d vectorBetween(tsrs::geometry::GeometryPoint3d start,
                                                          tsrs::geometry::GeometryPoint3d end)
    {
        return {end.x - start.x, end.y - start.y, end.z - start.z};
    }

    static double vectorMagnitude(tsrs::geometry::GeometryVector3d vector)
    {
        return std::sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
    }

    static double distanceBetween(tsrs::geometry::GeometryPoint3d lhs,
                                  tsrs::geometry::GeometryPoint3d rhs)
    {
        return vectorMagnitude(vectorBetween(lhs, rhs));
    }

    static tsrs::geometry::GeometryPoint3d interpolate(tsrs::geometry::GeometryPoint3d start,
                                                       tsrs::geometry::GeometryPoint3d end,
                                                       double ratio)
    {
        return {
            start.x + (end.x - start.x) * ratio,
            start.y + (end.y - start.y) * ratio,
            start.z + (end.z - start.z) * ratio,
        };
    }

    std::map<std::string, CurveRecord> curves_;
    std::vector<std::vector<tsrs::geometry::GeometryPoint3d>> madePolylines_;
    bool failMakePolyline_{false};
};

tsrs::geometry::GeometryRef curveRef(std::string stableId)
{
    return tsrs::geometry::GeometryRef{tsrs::geometry::GeometryEntityKind::Curve,
                                       std::move(stableId)};
}

tsrs::rebarsmart::FixNumberGeneratorInput baseInput()
{
    return tsrs::rebarsmart::FixNumberGeneratorInput{
        tsrs::rebarsmart::FixNumberParameters{
            "主筋",
            "D",
            0.036,
            0.2,
            30,
            1,
            0.082,
            4,
            tsrs::rebarsmart::FixNumberPriorityMode::Count,
            0.0,
            0.0,
        },
        tsrs::rebarsmart::FixNumberSelectionContext{
            std::vector<tsrs::geometry::GeometryRef>{
                curveRef("guide-main-a"),
                curveRef("guide-main-b"),
            },
        },
    };
}

int expectOk(const std::string& name,
             const tsrs::rebarsmart::FixNumberGenerationResult& result,
             const MockGeometryEngine& geometry,
             int expectedCount)
{
    if (!result.ok) {
        return fail(name + ": expected ok, got " + result.diagnosticCode);
    }
    if (result.diagnosticCode != "RS_OK") {
        return fail(name + ": expected RS_OK, got " + result.diagnosticCode);
    }
    if (result.centerlines.size() != static_cast<std::size_t>(expectedCount)) {
        return fail(name + ": unexpected centerline count");
    }
    if (geometry.madePolylines().size() != static_cast<std::size_t>(expectedCount)) {
        return fail(name + ": geometry engine did not receive expected centerline count");
    }
    return 0;
}

int expectDiagnostic(const std::string& name,
                     const tsrs::rebarsmart::FixNumberGenerationResult& result,
                     std::string_view expectedDiagnostic)
{
    if (result.ok) {
        return fail(name + ": expected failure");
    }
    if (result.diagnosticCode != expectedDiagnostic) {
        return fail(name + ": expected " + std::string{expectedDiagnostic} + ", got "
                    + result.diagnosticCode);
    }
    return 0;
}

int expectPolyline(const std::string& name,
                   const MockGeometryEngine& geometry,
                   std::size_t index,
                   double expectedStartX,
                   double expectedStartY,
                   double expectedEndX,
                   double expectedEndY)
{
    const auto& points = geometry.madePolylines().at(index);
    if (points.size() != 2) {
        return fail(name + ": expected two-point centerline");
    }
    if (!nearlyEqual(points[0].x, expectedStartX) || !nearlyEqual(points[0].y, expectedStartY)
        || !nearlyEqual(points[1].x, expectedEndX) || !nearlyEqual(points[1].y, expectedEndY)) {
        return fail(name + ": unexpected centerline coordinates");
    }
    return 0;
}

} // namespace

int main()
{
    using tsrs::geometry::GeometryPoint3d;
    using tsrs::rebarsmart::FixNumberPriorityMode;
    using tsrs::rebarsmart::generateFixNumberCenterlines;

    {
        MockGeometryEngine geometry;
        geometry.addStraightCurve("guide-main-a", GeometryPoint3d{0.0, 0.0, 0.0},
                                  GeometryPoint3d{1.0, 0.0, 0.0});
        geometry.addStraightCurve("guide-main-b", GeometryPoint3d{0.0, 1.0, 0.0},
                                  GeometryPoint3d{1.0, 1.0, 0.0});

        auto input = baseInput();
        input.parameters.priorityMode = FixNumberPriorityMode::Count;
        input.parameters.count = 3;

        const auto result = generateFixNumberCenterlines(input, geometry);
        if (const int code = expectOk("FN-001", result, geometry, 3)) {
            return code;
        }
        if (const int code = expectPolyline("FN-001 first", geometry, 0, 0.0, 0.0, 0.0, 1.0)) {
            return code;
        }
        if (const int code = expectPolyline("FN-001 middle", geometry, 1, 0.5, 0.0, 0.5, 1.0)) {
            return code;
        }
        if (const int code = expectPolyline("FN-001 last", geometry, 2, 1.0, 0.0, 1.0, 1.0)) {
            return code;
        }
        if (!nearlyEqual(result.centerlines[1].pointLengthOnFirstGuideM, 0.5)
            || !nearlyEqual(result.centerlines[1].pointLengthOnSecondGuideM, 0.5)) {
            return fail("FN-001: unexpected point length metadata");
        }
    }

    {
        MockGeometryEngine geometry;
        geometry.addStraightCurve("guide-main-a", GeometryPoint3d{0.0, 0.0, 0.0},
                                  GeometryPoint3d{1.0, 0.0, 0.0});
        geometry.addStraightCurve("guide-main-b", GeometryPoint3d{0.0, 1.0, 0.0},
                                  GeometryPoint3d{1.0, 1.0, 0.0});
        geometry.addStraightCurve("guide-main-c", GeometryPoint3d{0.0, 2.0, 0.0},
                                  GeometryPoint3d{1.0, 2.0, 0.0});

        auto input = baseInput();
        input.selection.mainGuideCurves.push_back(curveRef("guide-main-c"));
        input.parameters.count = 3;

        const auto result = generateFixNumberCenterlines(input, geometry);
        if (const int code =
                expectOk("FN-001A current-understanding adjacent pairs", result, geometry, 6)) {
            return code;
        }
        if (result.centerlines[3].guidePairIndex != 1) {
            return fail(
                "FN-001A current-understanding adjacent pairs: expected second guide pair metadata");
        }
        if (const int code = expectPolyline("FN-001A current-understanding second-pair first",
                                            geometry,
                                            3,
                                            0.0,
                                            1.0,
                                            0.0,
                                            2.0)) {
            return code;
        }
    }

    {
        MockGeometryEngine geometry;
        geometry.addStraightCurve("guide-main-a", GeometryPoint3d{0.0, 0.0, 0.0},
                                  GeometryPoint3d{1.0, 0.0, 0.0});
        geometry.addStraightCurve("guide-main-b", GeometryPoint3d{0.0, 1.0, 0.0},
                                  GeometryPoint3d{1.0, 1.0, 0.0});

        auto input = baseInput();
        input.parameters.count = 1;

        const auto result = generateFixNumberCenterlines(input, geometry);
        if (const int code = expectDiagnostic("FN-002", result, "RS_COUNT_NON_POSITIVE")) {
            return code;
        }
    }

    {
        MockGeometryEngine geometry;
        geometry.addStraightCurve("guide-main-a", GeometryPoint3d{0.0, 0.0, 0.0},
                                  GeometryPoint3d{1.0, 0.0, 0.0});

        auto input = baseInput();
        input.selection.mainGuideCurves = {curveRef("guide-main-a")};
        const auto result = generateFixNumberCenterlines(input, geometry);
        if (const int code =
                expectDiagnostic("FN-003", result, "RS_FIX_NUMBER_GUIDE_CURVE_COUNT_INVALID")) {
            return code;
        }
    }

    {
        MockGeometryEngine geometry;
        geometry.addStraightCurve("guide-main-a", GeometryPoint3d{0.0, 0.0, 0.0},
                                  GeometryPoint3d{1.0, 0.0, 0.0});

        auto input = baseInput();
        const auto result = generateFixNumberCenterlines(input, geometry);
        if (const int code =
                expectDiagnostic("FN-003A", result, "RS_FIX_NUMBER_GUIDE_CURVE_INVALID")) {
            return code;
        }
    }

    {
        MockGeometryEngine geometry;
        geometry.addStraightCurve("guide-main-a", GeometryPoint3d{0.0, 0.0, 0.0},
                                  GeometryPoint3d{1.0, 0.0, 0.0});
        geometry.addStraightCurve("guide-main-b", GeometryPoint3d{0.0, 1.0, 0.0},
                                  GeometryPoint3d{1.0, 1.0, 0.0});

        auto input = baseInput();
        input.selection.mainGuideCurves[0] = tsrs::geometry::GeometryRef{
            tsrs::geometry::GeometryEntityKind::Surface,
            "guide-main-a",
        };
        const auto result = generateFixNumberCenterlines(input, geometry);
        if (const int code =
                expectDiagnostic("FN-004", result, "RS_FIX_NUMBER_GUIDE_CURVE_INVALID")) {
            return code;
        }
    }

    {
        MockGeometryEngine geometry;
        geometry.addStraightCurve("guide-main-a", GeometryPoint3d{0.0, 0.0, 0.0},
                                  GeometryPoint3d{1.0, 0.0, 0.0});
        geometry.addStraightCurve("guide-main-b", GeometryPoint3d{0.0, 1.0, 0.0},
                                  GeometryPoint3d{1.0, 1.0, 0.0});

        auto input = baseInput();
        input.parameters.diameterM = 0.0;
        const auto result = generateFixNumberCenterlines(input, geometry);
        if (const int code =
                expectDiagnostic("FN-005", result, "RS_FIX_NUMBER_DIAMETER_INVALID")) {
            return code;
        }
    }

    {
        MockGeometryEngine geometry;
        geometry.addStraightCurve("guide-main-a", GeometryPoint3d{0.0, 0.0, 0.0},
                                  GeometryPoint3d{1.0, 0.0, 0.0});
        geometry.addStraightCurve("guide-main-b", GeometryPoint3d{0.0, 1.0, 0.0},
                                  GeometryPoint3d{1.0, 1.0, 0.0});
        geometry.setFailMakePolyline(true);

        auto input = baseInput();
        input.parameters.count = 2;
        const auto result = generateFixNumberCenterlines(input, geometry);
        if (const int code =
                expectDiagnostic("FN-006", result, "RS_FIX_NUMBER_CENTERLINE_INVALID")) {
            return code;
        }
    }

    return 0;
}
