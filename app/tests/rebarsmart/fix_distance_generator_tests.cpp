#include "rebarsmart/generators/FixDistanceGenerator.h"

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
        return {
            true,
            interpolate(found->second.start, found->second.end, ratio),
            std::string{tsrs::geometry::kGeometryDiagnosticOk},
        };
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
        return {
            true,
            tsrs::geometry::GeometryVector3d{
                vector.x / magnitude,
                vector.y / magnitude,
                vector.z / magnitude,
            },
            std::string{tsrs::geometry::kGeometryDiagnosticOk},
        };
    }

    tsrs::geometry::GeometryRefResult makePolylineCurve(
        std::vector<tsrs::geometry::GeometryPoint3d> points) override
    {
        if (points.size() < 2) {
            return {false, {}, std::string{tsrs::geometry::kGeometryDiagnosticInvalidInput}};
        }

        madePolylines_.push_back(std::move(points));
        const std::string stableId = "generated-centerline-" + std::to_string(madePolylines_.size());
        curves_.emplace(stableId, CurveRecord{madePolylines_.back().front(),
                                              madePolylines_.back().back()});
        return {
            true,
            tsrs::geometry::GeometryRef{tsrs::geometry::GeometryEntityKind::Curve, stableId},
            std::string{tsrs::geometry::kGeometryDiagnosticOk},
        };
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
};

tsrs::geometry::GeometryRef curveRef(std::string stableId)
{
    return tsrs::geometry::GeometryRef{tsrs::geometry::GeometryEntityKind::Curve,
                                       std::move(stableId)};
}

tsrs::rebarsmart::FixDistanceGeneratorInput baseInput()
{
    return tsrs::rebarsmart::FixDistanceGeneratorInput{
        curveRef("guide-main"),
        curveRef("guide-auxiliary"),
        tsrs::rebarsmart::FixDistanceParameters{
            "style-2",
            "C",
            0.028,
            0.2,
            11,
            1,
            tsrs::rebarsmart::FixDistancePriorityMode::Spacing,
            0.8,
            tsrs::rebarsmart::ZoneLengths{0.1, 0.1},
            "",
        },
    };
}

int expectOk(const std::string& name,
             const tsrs::rebarsmart::FixDistanceGenerationResult& result,
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
        return fail(name + ": unexpected generated centerline count");
    }
    if (geometry.madePolylines().size() != static_cast<std::size_t>(expectedCount)) {
        return fail(name + ": geometry engine did not receive expected centerline count");
    }
    return 0;
}

int expectDiagnostic(const std::string& name,
                     const tsrs::rebarsmart::FixDistanceGenerationResult& result,
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
                   double expectedX,
                   double expectedStartY,
                   double expectedEndY)
{
    const auto& points = geometry.madePolylines().at(index);
    if (points.size() != 2) {
        return fail(name + ": expected two-point centerline");
    }
    if (!nearlyEqual(points[0].x, expectedX) || !nearlyEqual(points[0].y, expectedStartY)
        || !nearlyEqual(points[1].x, expectedX) || !nearlyEqual(points[1].y, expectedEndY)) {
        return fail(name + ": unexpected centerline coordinates");
    }
    return 0;
}

} // namespace

int main()
{
    using tsrs::geometry::GeometryEntityKind;
    using tsrs::geometry::GeometryPoint3d;
    using tsrs::geometry::GeometryRef;
    using tsrs::rebarsmart::FixDistancePriorityMode;
    using tsrs::rebarsmart::generateFixDistanceCenterlines;

    {
        MockGeometryEngine geometry;
        geometry.addStraightCurve("guide-main", GeometryPoint3d{0.0, 0.0, 0.0},
                                  GeometryPoint3d{1.0, 0.0, 0.0});
        geometry.addStraightCurve("guide-auxiliary", GeometryPoint3d{0.0, 0.0, 0.0},
                                  GeometryPoint3d{0.0, 1.0, 0.0});

        auto input = baseInput();
        input.parameters.priorityMode = FixDistancePriorityMode::Spacing;
        input.parameters.spacingM = 0.2;
        input.parameters.zone = tsrs::rebarsmart::ZoneLengths{0.1, 0.1};

        const auto result = generateFixDistanceCenterlines(input, geometry);
        if (const int code = expectOk("FD-001", result, geometry, 5)) {
            return code;
        }
        if (const int code = expectPolyline("FD-001 first", geometry, 0, 0.1, 0.0, 1.0)) {
            return code;
        }
        if (const int code = expectPolyline("FD-001 last", geometry, 4, 0.9, 0.0, 1.0)) {
            return code;
        }
        if (!nearlyEqual(result.distribution.spacingM, 0.2)
            || !nearlyEqual(result.distribution.marginM, 0.0)) {
            return fail("FD-001: unexpected distribution summary");
        }
    }

    {
        MockGeometryEngine geometry;
        geometry.addStraightCurve("guide-main", GeometryPoint3d{0.0, 0.0, 0.0},
                                  GeometryPoint3d{1.0, 0.0, 0.0});
        geometry.addStraightCurve("guide-auxiliary", GeometryPoint3d{0.0, 10.0, 0.0},
                                  GeometryPoint3d{0.0, 12.0, 0.0});

        auto input = baseInput();
        input.parameters.priorityMode = FixDistancePriorityMode::Count;
        input.parameters.count = 3;
        input.parameters.zone = tsrs::rebarsmart::ZoneLengths{0.25, 0.25};

        const auto result = generateFixDistanceCenterlines(input, geometry);
        if (const int code = expectOk("FD-002", result, geometry, 3)) {
            return code;
        }
        if (const int code = expectPolyline("FD-002 first", geometry, 0, 0.25, 0.0, 2.0)) {
            return code;
        }
        if (const int code = expectPolyline("FD-002 last", geometry, 2, 0.75, 0.0, 2.0)) {
            return code;
        }
        if (!nearlyEqual(result.distribution.spacingM, 0.25)
            || !nearlyEqual(result.distribution.marginM, 0.0)) {
            return fail("FD-002: unexpected count-priority distribution summary");
        }
    }

    {
        MockGeometryEngine geometry;
        geometry.addStraightCurve("guide-main", GeometryPoint3d{0.0, 0.0, 0.0},
                                  GeometryPoint3d{1.0, 0.0, 0.0});
        geometry.addStraightCurve("guide-auxiliary", GeometryPoint3d{0.0, 0.0, 5.0},
                                  GeometryPoint3d{0.0, 0.0, 6.0});

        auto input = baseInput();
        input.parameters.priorityMode = FixDistancePriorityMode::SpacingList;
        input.parameters.zone = tsrs::rebarsmart::ZoneLengths{0.1, 0.1};
        input.parameters.spacingListText = "20*2,30";

        const auto result = generateFixDistanceCenterlines(input, geometry);
        if (const int code = expectOk("FD-003", result, geometry, 4)) {
            return code;
        }
        if (const int code = expectPolyline("FD-003 third", geometry, 2, 0.5, 0.0, 0.0)) {
            return code;
        }
        if (const int code = expectPolyline("FD-003 fourth", geometry, 3, 0.8, 0.0, 0.0)) {
            return code;
        }
        if (!nearlyEqual(geometry.madePolylines().at(0).front().z, 0.0)
            || !nearlyEqual(geometry.madePolylines().at(0).back().z, 1.0)) {
            return fail("FD-003: expected auxiliary Z span");
        }
        if (!nearlyEqual(result.distribution.marginM, 0.1)) {
            return fail("FD-003: unexpected spacing-list margin");
        }
    }

    {
        MockGeometryEngine geometry;
        geometry.addStraightCurve("guide-main", GeometryPoint3d{0.0, 0.0, 0.0},
                                  GeometryPoint3d{1.0, 0.0, 0.0});
        auto input = baseInput();
        const auto result = generateFixDistanceCenterlines(input, geometry);
        if (const int code =
                expectDiagnostic("FD-004", result, "RS_FIX_DISTANCE_AUXILIARY_CURVE_INVALID")) {
            return code;
        }
    }

    {
        MockGeometryEngine geometry;
        geometry.addStraightCurve("guide-main", GeometryPoint3d{0.0, 0.0, 0.0},
                                  GeometryPoint3d{1.0, 0.0, 0.0});
        geometry.addStraightCurve("guide-auxiliary", GeometryPoint3d{0.0, 0.0, 0.0},
                                  GeometryPoint3d{0.0, 1.0, 0.0});
        auto input = baseInput();
        input.mainGuideCurve = GeometryRef{GeometryEntityKind::Surface, "guide-main"};
        const auto result = generateFixDistanceCenterlines(input, geometry);
        if (const int code =
                expectDiagnostic("FD-005", result, "RS_FIX_DISTANCE_MAIN_CURVE_INVALID")) {
            return code;
        }
    }

    {
        MockGeometryEngine geometry;
        geometry.addStraightCurve("guide-main", GeometryPoint3d{0.0, 0.0, 0.0},
                                  GeometryPoint3d{1.0, 0.0, 0.0});
        geometry.addStraightCurve("guide-auxiliary", GeometryPoint3d{0.0, 0.0, 0.0},
                                  GeometryPoint3d{0.0, 1.0, 0.0});
        auto input = baseInput();
        input.parameters.spacingM = 0.0;
        const auto result = generateFixDistanceCenterlines(input, geometry);
        if (const int code = expectDiagnostic("FD-006", result, "RS_SPACING_NON_POSITIVE")) {
            return code;
        }
    }

    {
        MockGeometryEngine geometry;
        geometry.addStraightCurve("guide-main", GeometryPoint3d{0.0, 0.0, 0.0},
                                  GeometryPoint3d{1.0, 0.0, 0.0});
        geometry.addStraightCurve("guide-auxiliary", GeometryPoint3d{0.0, 0.0, 0.0},
                                  GeometryPoint3d{0.0, 1.0, 0.0});
        auto input = baseInput();
        input.parameters.priorityMode = static_cast<FixDistancePriorityMode>(99);
        const auto result = generateFixDistanceCenterlines(input, geometry);
        if (const int code =
                expectDiagnostic("FD-007", result, "RS_FIX_DISTANCE_PRIORITY_MODE_UNKNOWN")) {
            return code;
        }
    }

    return 0;
}
