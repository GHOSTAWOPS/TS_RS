#include "rebarsmart/zone/GuideCurveZoneCalculator.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <string>

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

int expectOk(const std::string& name,
             const tsrs::rebarsmart::GuideCurveZoneResult& result,
             double expectedHeadM,
             double expectedTailM)
{
    if (!result.ok) {
        return fail(name + ": expected ok, got " + result.diagnosticCode);
    }

    if (result.diagnosticCode != "RS_OK") {
        return fail(name + ": expected RS_OK, got " + result.diagnosticCode);
    }

    if (!nearlyEqual(result.zone.headM, expectedHeadM)) {
        return fail(name + ": unexpected head zone");
    }

    if (!nearlyEqual(result.zone.tailM, expectedTailM)) {
        return fail(name + ": unexpected tail zone");
    }

    return 0;
}

int expectDiagnostic(const std::string& name,
                     const tsrs::rebarsmart::GuideCurveZoneResult& result,
                     const std::string& diagnosticCode)
{
    if (result.ok) {
        return fail(name + ": expected failure");
    }

    if (result.diagnosticCode != diagnosticCode) {
        return fail(name + ": expected " + diagnosticCode + ", got " + result.diagnosticCode);
    }

    return 0;
}

} // namespace

int main()
{
    using tsrs::rebarsmart::GuideCurveZoneInput;
    using tsrs::rebarsmart::ZoneAdjustMode;
    using tsrs::rebarsmart::ZoneLengths;
    using tsrs::rebarsmart::calculateGuideCurveZone;

    if (const int code = expectOk("GZ-001",
                                  calculateGuideCurveZone(GuideCurveZoneInput{
                                      ZoneLengths{1.0, 2.0},
                                      ZoneAdjustMode::KeepCurrent,
                                      ZoneAdjustMode::KeepCurrent,
                                      0.032,
                                      0.2,
                                      0.066,
                                  }),
                                  1.0,
                                  2.0)) {
        return code;
    }

    if (const int code = expectOk("GZ-002",
                                  calculateGuideCurveZone(GuideCurveZoneInput{
                                      ZoneLengths{1.0, 2.0},
                                      ZoneAdjustMode::Zero,
                                      ZoneAdjustMode::Zero,
                                      0.032,
                                      0.2,
                                      0.066,
                                  }),
                                  0.0,
                                  0.0)) {
        return code;
    }

    if (const int code = expectOk("GZ-003",
                                  calculateGuideCurveZone(GuideCurveZoneInput{
                                      ZoneLengths{1.0, 2.0},
                                      ZoneAdjustMode::HalfDiameter,
                                      ZoneAdjustMode::HalfDiameter,
                                      0.032,
                                      0.2,
                                      0.066,
                                  }),
                                  0.016,
                                  0.016)) {
        return code;
    }

    if (const int code = expectOk("GZ-004",
                                  calculateGuideCurveZone(GuideCurveZoneInput{
                                      ZoneLengths{1.0, 2.0},
                                      ZoneAdjustMode::HalfDiameterPlusSpacing,
                                      ZoneAdjustMode::HalfDiameterPlusSpacing,
                                      0.032,
                                      0.2,
                                      0.066,
                                  }),
                                  0.216,
                                  0.216)) {
        return code;
    }

    if (const int code = expectOk("GZ-005",
                                  calculateGuideCurveZone(GuideCurveZoneInput{
                                      ZoneLengths{1.0, 2.0},
                                      ZoneAdjustMode::GuideSurfaceOffset,
                                      ZoneAdjustMode::GuideSurfaceOffset,
                                      0.032,
                                      0.2,
                                      0.066,
                                  }),
                                  0.066,
                                  0.066)) {
        return code;
    }

    if (const int code = expectOk("GZ-006",
                                  calculateGuideCurveZone(GuideCurveZoneInput{
                                      ZoneLengths{1.0, 2.0},
                                      ZoneAdjustMode::HalfDiameter,
                                      ZoneAdjustMode::GuideSurfaceOffset,
                                      0.032,
                                      0.2,
                                      0.066,
                                  }),
                                  0.016,
                                  0.066)) {
        return code;
    }

    if (const int code = expectDiagnostic(
            "GZ-007",
            calculateGuideCurveZone(GuideCurveZoneInput{
                ZoneLengths{-1.0, 2.0},
                ZoneAdjustMode::KeepCurrent,
                ZoneAdjustMode::KeepCurrent,
                0.032,
                0.2,
                0.066,
            }),
            "RS_GUIDE_CURVE_ZONE_INVALID")) {
        return code;
    }

    if (const int code = expectDiagnostic(
            "GZ-008",
            calculateGuideCurveZone(GuideCurveZoneInput{
                ZoneLengths{1.0, 2.0},
                ZoneAdjustMode::HalfDiameter,
                ZoneAdjustMode::KeepCurrent,
                -0.032,
                0.2,
                0.066,
            }),
            "RS_GUIDE_CURVE_ZONE_INVALID")) {
        return code;
    }

    if (const int code = expectDiagnostic(
            "GZ-009",
            calculateGuideCurveZone(GuideCurveZoneInput{
                ZoneLengths{1.0, 2.0},
                ZoneAdjustMode::HalfDiameterPlusSpacing,
                ZoneAdjustMode::KeepCurrent,
                0.032,
                std::numeric_limits<double>::quiet_NaN(),
                0.066,
            }),
            "RS_GUIDE_CURVE_ZONE_INVALID")) {
        return code;
    }

    if (const int code = expectDiagnostic(
            "GZ-010",
            calculateGuideCurveZone(GuideCurveZoneInput{
                ZoneLengths{1.0, 2.0},
                ZoneAdjustMode::GuideSurfaceOffset,
                ZoneAdjustMode::KeepCurrent,
                0.032,
                0.2,
                -0.066,
            }),
            "RS_GUIDE_CURVE_ZONE_INVALID")) {
        return code;
    }

    if (const int code = expectDiagnostic(
            "GZ-011",
            calculateGuideCurveZone(GuideCurveZoneInput{
                ZoneLengths{1.0, 2.0},
                static_cast<ZoneAdjustMode>(99),
                ZoneAdjustMode::KeepCurrent,
                0.032,
                0.2,
                0.066,
            }),
            "RS_GUIDE_CURVE_ZONE_MODE_UNKNOWN")) {
        return code;
    }

    return 0;
}
