#include "rebarsmart/distribute/PrioritySpaceDistributor.h"

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
             const tsrs::rebarsmart::DistributionResult& result,
             int expectedCount,
             double expectedSpacingM,
             double expectedMarginM)
{
    if (!result.ok) {
        return fail(name + ": expected ok, got " + result.diagnosticCode);
    }

    if (result.diagnosticCode != "RS_OK") {
        return fail(name + ": expected RS_OK, got " + result.diagnosticCode);
    }

    if (result.count != expectedCount) {
        return fail(name + ": unexpected count");
    }

    if (!nearlyEqual(result.spacingM, expectedSpacingM)) {
        return fail(name + ": unexpected spacing");
    }

    if (!nearlyEqual(result.marginM, expectedMarginM)) {
        return fail(name + ": unexpected margin");
    }

    if (!result.pointLengthsM.empty()) {
        return fail(name + ": TODO-008 should not emit point lengths");
    }

    return 0;
}

int expectDiagnostic(const std::string& name,
                     const tsrs::rebarsmart::DistributionResult& result,
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
    using tsrs::rebarsmart::ZoneLengths;
    using tsrs::rebarsmart::distributeBySpacing;

    if (const int code = expectOk("PS-001",
                                  distributeBySpacing(10.0, ZoneLengths{1.0, 1.0}, 2.0, 0.8),
                                  5,
                                  2.0,
                                  0.0)) {
        return code;
    }

    if (const int code = expectOk("PS-002",
                                  distributeBySpacing(10.0, ZoneLengths{1.0, 1.0}, 3.0, 0.8),
                                  3,
                                  3.0,
                                  2.0)) {
        return code;
    }

    if (const int code = expectOk("PS-003",
                                  distributeBySpacing(10.0, ZoneLengths{6.0, 6.0}, 2.0, 0.8),
                                  1,
                                  2.0,
                                  0.0)) {
        return code;
    }

    if (const int code = expectDiagnostic("PS-004",
                                          distributeBySpacing(10.0,
                                                              ZoneLengths{1.0, 1.0},
                                                              0.0,
                                                              0.8),
                                          "RS_SPACING_NON_POSITIVE")) {
        return code;
    }

    if (const int code = expectOk("PS-005",
                                  distributeBySpacing(0.8, ZoneLengths{0.1, 0.1}, 0.2, 0.8),
                                  4,
                                  0.2,
                                  0.0)) {
        return code;
    }

    if (const int code = expectDiagnostic("PS-006",
                                          distributeBySpacing(10.0,
                                                              ZoneLengths{
                                                                  -1.0,
                                                                  1.0,
                                                              },
                                                              2.0,
                                                              0.8),
                                          "RS_ZONE_LENGTH_INVALID")) {
        return code;
    }

    if (const int code = expectDiagnostic("PS-007",
                                          distributeBySpacing(
                                              10.0,
                                              ZoneLengths{
                                                  std::numeric_limits<double>::infinity(),
                                                  1.0,
                                              },
                                              2.0,
                                              0.8),
                                          "RS_ZONE_LENGTH_INVALID")) {
        return code;
    }

    if (const int code = expectDiagnostic("PS-008",
                                          distributeBySpacing(10.0,
                                                              ZoneLengths{
                                                                  1.0,
                                                                  -1.0,
                                                              },
                                                              2.0,
                                                              0.8),
                                          "RS_ZONE_LENGTH_INVALID")) {
        return code;
    }

    if (const int code = expectDiagnostic("PS-009",
                                          distributeBySpacing(
                                              10.0,
                                              ZoneLengths{1.0, 1.0},
                                              std::numeric_limits<double>::quiet_NaN(),
                                              0.8),
                                          "RS_SPACING_NON_POSITIVE")) {
        return code;
    }

    if (const int code = expectDiagnostic("PS-010",
                                          distributeBySpacing(
                                              10.0e12,
                                              ZoneLengths{0.0, 0.0},
                                              0.001,
                                              0.8),
                                          "RS_SECTION_COUNT_TOO_LARGE")) {
        return code;
    }

    return 0;
}
