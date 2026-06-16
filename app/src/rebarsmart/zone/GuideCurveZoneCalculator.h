#pragma once

#include "rebarsmart/distribute/DistributionTypes.h"

#include <string>

namespace tsrs::rebarsmart {

inline constexpr const char* kDiagnosticGuideCurveZoneInvalid = "RS_GUIDE_CURVE_ZONE_INVALID";
inline constexpr const char* kDiagnosticGuideCurveZoneModeUnknown =
    "RS_GUIDE_CURVE_ZONE_MODE_UNKNOWN";

enum class ZoneAdjustMode {
    KeepCurrent = 0,
    Zero = 1,
    HalfDiameter = 2,
    HalfDiameterPlusSpacing = 3,
    GuideSurfaceOffset = 4,
};

struct GuideCurveZoneInput {
    ZoneLengths current;
    ZoneAdjustMode headMode{ZoneAdjustMode::KeepCurrent};
    ZoneAdjustMode tailMode{ZoneAdjustMode::KeepCurrent};
    double diameterM{0.0};
    double spacingM{0.0};
    double guideSurfaceOffsetM{0.0};
};

struct GuideCurveZoneResult {
    bool ok{false};
    ZoneLengths zone;
    std::string diagnosticCode;
};

GuideCurveZoneResult calculateGuideCurveZone(GuideCurveZoneInput input);

} // namespace tsrs::rebarsmart
