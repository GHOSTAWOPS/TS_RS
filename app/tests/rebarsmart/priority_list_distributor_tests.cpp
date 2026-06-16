#include "rebarsmart/distribute/PriorityListDistributor.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <string>
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

int expectOk(const std::string& name,
             const tsrs::rebarsmart::DistributionResult& result,
             int expectedCount,
             double expectedMarginM,
             const std::vector<double>& expectedPointLengthsM)
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

    if (!nearlyEqual(result.marginM, expectedMarginM)) {
        return fail(name + ": unexpected margin");
    }

    if (result.pointLengthsM.size() != expectedPointLengthsM.size()) {
        return fail(name + ": unexpected point count");
    }

    for (std::size_t index = 0; index < expectedPointLengthsM.size(); ++index) {
        if (!nearlyEqual(result.pointLengthsM[index], expectedPointLengthsM[index])) {
            return fail(name + ": unexpected point length at index " + std::to_string(index));
        }
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
    using tsrs::rebarsmart::distributeBySpacingList;

    if (const int code = expectOk("PL-001",
                                  distributeBySpacingList(10.0,
                                                          ZoneLengths{1.0, 1.0},
                                                          {2.0, 2.0, 2.0, 2.0}),
                                  5,
                                  0.0,
                                  {1.0, 3.0, 5.0, 7.0, 9.0})) {
        return code;
    }

    if (const int code = expectOk("PL-002",
                                  distributeBySpacingList(10.0,
                                                          ZoneLengths{1.0, 1.0},
                                                          {3.0, 3.0}),
                                  3,
                                  2.0,
                                  {1.0, 4.0, 7.0})) {
        return code;
    }

    if (const int code = expectDiagnostic("PL-003",
                                          distributeBySpacingList(10.0,
                                                                  ZoneLengths{1.0, 1.0},
                                                                  {}),
                                          "RS_SPACE_LIST_EMPTY")) {
        return code;
    }

    if (const int code = expectDiagnostic("PL-004",
                                          distributeBySpacingList(10.0,
                                                                  ZoneLengths{1.0, 1.0},
                                                                  {20.0}),
                                          "RS_POINT_COUNT_TOO_SMALL")) {
        return code;
    }

    if (const int code = expectDiagnostic("PL-005",
                                          distributeBySpacingList(0.0,
                                                                  ZoneLengths{0.0, 0.0},
                                                                  {1.0, 1.0}),
                                          "RS_CURVE_LENGTH_NON_POSITIVE")) {
        return code;
    }

    if (const int code = expectDiagnostic("PL-006",
                                          distributeBySpacingList(10.0,
                                                                  ZoneLengths{-1.0, 1.0},
                                                                  {1.0, 1.0}),
                                          "RS_ZONE_LENGTH_INVALID")) {
        return code;
    }

    if (const int code = expectDiagnostic("PL-007",
                                          distributeBySpacingList(10.0,
                                                                  ZoneLengths{1.0, 1.0},
                                                                  {1.0, 0.0}),
                                          "RS_SPACING_NON_POSITIVE")) {
        return code;
    }

    if (const int code = expectDiagnostic("PL-008",
                                          distributeBySpacingList(
                                              10.0,
                                              ZoneLengths{1.0, 1.0},
                                              {1.0, std::numeric_limits<double>::quiet_NaN()}),
                                          "RS_SPACING_NON_POSITIVE")) {
        return code;
    }

    if (const int code = expectDiagnostic("PL-009",
                                          distributeBySpacingList(10.0,
                                                                  ZoneLengths{9.0, 1.0},
                                                                  {1.0e-12}),
                                          "RS_POINT_COUNT_TOO_SMALL")) {
        return code;
    }

    if (const int code = expectDiagnostic("PL-010",
                                          distributeBySpacingList(10.0,
                                                                  ZoneLengths{10.0, 1.0},
                                                                  {1.0}),
                                          "RS_POINT_COUNT_TOO_SMALL")) {
        return code;
    }

    return 0;
}
