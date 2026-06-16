#include "rebarsmart/distribute/PriorityCountDistributor.h"

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
    using tsrs::rebarsmart::distributeByCount;

    if (const int code = expectOk("PC-001",
                                  distributeByCount(10.0, ZoneLengths{1.0, 1.0}, 5),
                                  5,
                                  2.0,
                                  0.0)) {
        return code;
    }

    if (const int code = expectOk("PC-002",
                                  distributeByCount(10.0, ZoneLengths{1.0, 1.0}, 1),
                                  1,
                                  16.0,
                                  0.0)) {
        return code;
    }

    if (const int code = expectOk("PC-003",
                                  distributeByCount(10.0, ZoneLengths{6.0, 6.0}, 5),
                                  5,
                                  0.0,
                                  0.0)) {
        return code;
    }

    if (const int code = expectDiagnostic("PC-004",
                                          distributeByCount(0.0, ZoneLengths{0.0, 0.0}, 5),
                                          "RS_CURVE_LENGTH_NON_POSITIVE")) {
        return code;
    }

    if (const int code = expectDiagnostic("PC-005",
                                          distributeByCount(10.0, ZoneLengths{1.0, 1.0}, 0),
                                          "RS_COUNT_NON_POSITIVE")) {
        return code;
    }

    if (const int code = expectDiagnostic("PC-006",
                                          distributeByCount(10.0,
                                                            ZoneLengths{
                                                                -1.0,
                                                                1.0,
                                                            },
                                                            5),
                                          "RS_ZONE_LENGTH_INVALID")) {
        return code;
    }

    if (const int code = expectDiagnostic("PC-007",
                                          distributeByCount(
                                              10.0,
                                              ZoneLengths{
                                                  std::numeric_limits<double>::quiet_NaN(),
                                                  1.0,
                                              },
                                              5),
                                          "RS_ZONE_LENGTH_INVALID")) {
        return code;
    }

    if (const int code = expectDiagnostic("PC-008",
                                          distributeByCount(10.0,
                                                            ZoneLengths{
                                                                1.0,
                                                                -1.0,
                                                            },
                                                            5),
                                          "RS_ZONE_LENGTH_INVALID")) {
        return code;
    }

    if (const int code = expectDiagnostic("PC-009",
                                          distributeByCount(
                                              std::numeric_limits<double>::infinity(),
                                              ZoneLengths{0.0, 0.0},
                                              5),
                                          "RS_CURVE_LENGTH_NON_POSITIVE")) {
        return code;
    }

    return 0;
}
