#pragma once

#include <string>

namespace tsrs::rebarsmart {

inline constexpr const char* kDiagnosticGuideSurfaceOffsetInvalid =
    "RS_GUIDE_SURFACE_OFFSET_INVALID";
inline constexpr const char* kDiagnosticGuideSurfaceOffsetModeUnknown =
    "RS_GUIDE_SURFACE_OFFSET_MODE_UNKNOWN";

struct GuideSurfaceOffsetInput {
    int offsetMode{0};
    int activeLayerIndex{0};
    double diameterM{0.0};
    double coverThicknessM{0.0};
    double layerSpacingM{0.0};
    double firstLayerProtectThicknessNetM{0.0};
};

struct GuideSurfaceOffsetResult {
    bool ok{false};
    double offsetM{0.0};
    std::string diagnosticCode;
};

GuideSurfaceOffsetResult calculateGuideSurfaceOffset(GuideSurfaceOffsetInput input);

} // namespace tsrs::rebarsmart
