#include "rebarsmart/zone/GuideCurveZoneCalculator.h"

#include <cmath>

namespace tsrs::rebarsmart {
namespace {

GuideCurveZoneResult makeFailure(const char* diagnosticCode)
{
    return GuideCurveZoneResult{false, ZoneLengths{}, diagnosticCode};
}

GuideCurveZoneResult makeSuccess(ZoneLengths zone)
{
    return GuideCurveZoneResult{true, zone, std::string{kDiagnosticOk}};
}

bool isFiniteNonNegative(double value)
{
    return std::isfinite(value) && value >= 0.0;
}

bool isValidZone(ZoneLengths zone)
{
    return isFiniteNonNegative(zone.headM) && isFiniteNonNegative(zone.tailM);
}

bool tryCalculateOneZone(ZoneAdjustMode mode,
                         double currentM,
                         double diameterM,
                         double spacingM,
                         double guideSurfaceOffsetM,
                         double& outZoneM)
{
    switch (mode) {
    case ZoneAdjustMode::KeepCurrent:
        outZoneM = currentM;
        return true;
    case ZoneAdjustMode::Zero:
        outZoneM = 0.0;
        return true;
    case ZoneAdjustMode::HalfDiameter:
        outZoneM = 0.5 * diameterM;
        return true;
    case ZoneAdjustMode::HalfDiameterPlusSpacing:
        outZoneM = 0.5 * diameterM + spacingM;
        return true;
    case ZoneAdjustMode::GuideSurfaceOffset:
        outZoneM = guideSurfaceOffsetM;
        return true;
    }

    return false;
}

} // namespace

GuideCurveZoneResult calculateGuideCurveZone(GuideCurveZoneInput input)
{
    if (!isValidZone(input.current)
        || !isFiniteNonNegative(input.diameterM)
        || !isFiniteNonNegative(input.spacingM)
        || !isFiniteNonNegative(input.guideSurfaceOffsetM)) {
        return makeFailure(kDiagnosticGuideCurveZoneInvalid);
    }

    ZoneLengths zone;
    if (!tryCalculateOneZone(input.headMode,
                             input.current.headM,
                             input.diameterM,
                             input.spacingM,
                             input.guideSurfaceOffsetM,
                             zone.headM)
        || !tryCalculateOneZone(input.tailMode,
                                input.current.tailM,
                                input.diameterM,
                                input.spacingM,
                                input.guideSurfaceOffsetM,
                                zone.tailM)) {
        return makeFailure(kDiagnosticGuideCurveZoneModeUnknown);
    }

    if (!isValidZone(zone)) {
        return makeFailure(kDiagnosticGuideCurveZoneInvalid);
    }

    return makeSuccess(zone);
}

} // namespace tsrs::rebarsmart
