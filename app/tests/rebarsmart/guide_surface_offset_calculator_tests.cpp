#include "rebarsmart/zone/GuideSurfaceOffsetCalculator.h"

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
             const tsrs::rebarsmart::GuideSurfaceOffsetResult& result,
             double expectedOffsetM)
{
    if (!result.ok) {
        return fail(name + ": expected ok, got " + result.diagnosticCode);
    }

    if (result.diagnosticCode != "RS_OK") {
        return fail(name + ": expected RS_OK, got " + result.diagnosticCode);
    }

    if (!nearlyEqual(result.offsetM, expectedOffsetM)) {
        return fail(name + ": unexpected offset");
    }

    return 0;
}

int expectDiagnostic(const std::string& name,
                     const tsrs::rebarsmart::GuideSurfaceOffsetResult& result,
                     const std::string& diagnosticCode,
                     double expectedOffsetM = 0.0)
{
    if (result.ok) {
        return fail(name + ": expected failure");
    }

    if (result.diagnosticCode != diagnosticCode) {
        return fail(name + ": expected " + diagnosticCode + ", got " + result.diagnosticCode);
    }

    if (!nearlyEqual(result.offsetM, expectedOffsetM)) {
        return fail(name + ": unexpected fallback offset");
    }

    return 0;
}

} // namespace

int main()
{
    using tsrs::rebarsmart::GuideSurfaceOffsetInput;
    using tsrs::rebarsmart::calculateGuideSurfaceOffset;

    if (const int code = expectOk("GO-001",
                                  calculateGuideSurfaceOffset(
                                      GuideSurfaceOffsetInput{0, 0, 0.032, 0.05, 0.2, 0.0}),
                                  0.066)) {
        return code;
    }

    if (const int code = expectOk("GO-002",
                                  calculateGuideSurfaceOffset(
                                      GuideSurfaceOffsetInput{1, 0, 0.032, 0.05, 0.2, 0.0}),
                                  0.034)) {
        return code;
    }

    if (const int code = expectOk("GO-003",
                                  calculateGuideSurfaceOffset(
                                      GuideSurfaceOffsetInput{2, 1, 0.032, 0.05, 0.2, 0.0}),
                                  0.266)) {
        return code;
    }

    if (const int code = expectOk("GO-004",
                                  calculateGuideSurfaceOffset(
                                      GuideSurfaceOffsetInput{3, 0, 0.032, 0.05, 0.2, 0.08}),
                                  0.096)) {
        return code;
    }

    if (const int code = expectOk("GO-004A",
                                  calculateGuideSurfaceOffset(
                                      GuideSurfaceOffsetInput{4, 1, 0.032, 0.05, 0.2, 0.08}),
                                  0.296)) {
        return code;
    }

    if (const int code = expectOk("GO-005",
                                  calculateGuideSurfaceOffset(
                                      GuideSurfaceOffsetInput{5, 2, 0.032, 0.05, 0.2, 0.0}),
                                  0.45)) {
        return code;
    }

    if (const int code = expectDiagnostic(
            "GO-006",
            calculateGuideSurfaceOffset(GuideSurfaceOffsetInput{99, 0, 0.032, 0.05, 0.2, 0.0}),
            "RS_GUIDE_SURFACE_OFFSET_MODE_UNKNOWN",
            0.0)) {
        return code;
    }

    if (const int code = expectDiagnostic(
            "GO-007",
            calculateGuideSurfaceOffset(GuideSurfaceOffsetInput{0, 0, -0.032, 0.05, 0.2, 0.0}),
            "RS_GUIDE_SURFACE_OFFSET_INVALID")) {
        return code;
    }

    if (const int code = expectDiagnostic(
            "GO-008",
            calculateGuideSurfaceOffset(GuideSurfaceOffsetInput{
                0,
                0,
                0.032,
                std::numeric_limits<double>::quiet_NaN(),
                0.2,
                0.0,
            }),
            "RS_GUIDE_SURFACE_OFFSET_INVALID")) {
        return code;
    }

    if (const int code = expectDiagnostic(
            "GO-009",
            calculateGuideSurfaceOffset(GuideSurfaceOffsetInput{0, 0, 0.032, 0.05, -0.2, 0.0}),
            "RS_GUIDE_SURFACE_OFFSET_INVALID")) {
        return code;
    }

    if (const int code = expectDiagnostic(
            "GO-010",
            calculateGuideSurfaceOffset(GuideSurfaceOffsetInput{0, -1, 0.032, 0.05, 0.2, 0.0}),
            "RS_GUIDE_SURFACE_OFFSET_INVALID")) {
        return code;
    }

    if (const int code = expectDiagnostic(
            "GO-011",
            calculateGuideSurfaceOffset(GuideSurfaceOffsetInput{3, 0, 0.032, 0.05, 0.2, -0.08}),
            "RS_GUIDE_SURFACE_OFFSET_INVALID")) {
        return code;
    }

    if (const int code = expectOk("GO-012",
                                  calculateGuideSurfaceOffset(
                                      GuideSurfaceOffsetInput{1, 0, 0.032, 0.01, 0.2, 0.0}),
                                  -0.006)) {
        return code;
    }

    return 0;
}
