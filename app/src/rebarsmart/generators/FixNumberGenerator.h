#pragma once

#include "geometry/kernel/IGeometryEngine.h"
#include "rebarsmart/distribute/DistributionTypes.h"

#include <string>
#include <string_view>
#include <vector>

namespace tsrs::rebarsmart {

inline constexpr std::string_view kDiagnosticFixNumberGuideCurveCountInvalid =
    "RS_FIX_NUMBER_GUIDE_CURVE_COUNT_INVALID";
inline constexpr std::string_view kDiagnosticFixNumberGuideCurveInvalid =
    "RS_FIX_NUMBER_GUIDE_CURVE_INVALID";
inline constexpr std::string_view kDiagnosticFixNumberCenterlineInvalid =
    "RS_FIX_NUMBER_CENTERLINE_INVALID";
inline constexpr std::string_view kDiagnosticFixNumberPriorityModeUnknown =
    "RS_FIX_NUMBER_PRIORITY_MODE_UNKNOWN";
inline constexpr std::string_view kDiagnosticFixNumberDiameterInvalid =
    "RS_FIX_NUMBER_DIAMETER_INVALID";
inline constexpr std::string_view kDiagnosticFixNumberBundleCountInvalid =
    "RS_FIX_NUMBER_BUNDLE_COUNT_INVALID";

enum class FixNumberPriorityMode {
    Count = 1,
};

struct FixNumberParameters {
    std::string styleName;
    std::string grade;
    double diameterM{0.0};
    double spacingM{0.0};
    int count{0};
    int bundleCount{1};
    double coverThicknessM{0.0};
    int marginMode{0};
    FixNumberPriorityMode priorityMode{FixNumberPriorityMode::Count};
    double headDistanceM{0.0};
    double tailDistanceM{0.0};
};

struct FixNumberSelectionContext {
    std::vector<geometry::GeometryRef> mainGuideCurves;
};

struct FixNumberGeneratorInput {
    FixNumberParameters parameters;
    FixNumberSelectionContext selection;
};

struct FixNumberGeneratedCenterline {
    geometry::GeometryRef curveRef;
    std::size_t guidePairIndex{0};
    double pointLengthOnFirstGuideM{0.0};
    double pointLengthOnSecondGuideM{0.0};
};

struct FixNumberGenerationResult {
    bool ok{false};
    std::vector<FixNumberGeneratedCenterline> centerlines;
    std::string diagnosticCode;
};

FixNumberGenerationResult generateFixNumberCenterlines(
    const FixNumberGeneratorInput& input,
    geometry::IGeometryEngine& geometry);

} // namespace tsrs::rebarsmart
