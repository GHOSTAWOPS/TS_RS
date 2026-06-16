#pragma once

#include "geometry/kernel/IGeometryEngine.h"
#include "rebarsmart/distribute/DistributionTypes.h"

#include <string>
#include <string_view>
#include <vector>

namespace tsrs::rebarsmart {

inline constexpr std::string_view kDiagnosticFixDistanceMainCurveInvalid =
    "RS_FIX_DISTANCE_MAIN_CURVE_INVALID";
inline constexpr std::string_view kDiagnosticFixDistanceAuxiliaryCurveInvalid =
    "RS_FIX_DISTANCE_AUXILIARY_CURVE_INVALID";
inline constexpr std::string_view kDiagnosticFixDistancePriorityModeUnknown =
    "RS_FIX_DISTANCE_PRIORITY_MODE_UNKNOWN";
inline constexpr std::string_view kDiagnosticFixDistanceCenterlineInvalid =
    "RS_FIX_DISTANCE_CENTERLINE_INVALID";

enum class FixDistancePriorityMode {
    Spacing = 0,
    Count = 1,
    SpacingList = 2,
};

struct FixDistanceParameters {
    std::string styleName;
    std::string grade;
    double diameterM{0.0};
    double spacingM{0.0};
    int count{0};
    int bundleCount{1};
    FixDistancePriorityMode priorityMode{FixDistancePriorityMode::Spacing};
    double spacingRatio{1.0};
    ZoneLengths zone;
    std::string spacingListText;
};

struct FixDistanceGeneratorInput {
    geometry::GeometryRef mainGuideCurve;
    geometry::GeometryRef auxiliaryGuideCurve;
    FixDistanceParameters parameters;
};

struct GeneratedCenterline {
    geometry::GeometryRef curveRef;
    double mainGuideLengthM{0.0};
};

struct FixDistanceGenerationResult {
    bool ok{false};
    std::vector<GeneratedCenterline> centerlines;
    DistributionResult distribution;
    std::string diagnosticCode;
};

FixDistanceGenerationResult generateFixDistanceCenterlines(
    const FixDistanceGeneratorInput& input,
    geometry::IGeometryEngine& geometry);

} // namespace tsrs::rebarsmart
