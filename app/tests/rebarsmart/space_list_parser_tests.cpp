#include "rebarsmart/space_list/SpaceListParser.h"

#include <cmath>
#include <iostream>
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

int expectOkValues(const std::string& name,
                   const tsrs::rebarsmart::SpaceListParseResult& result,
                   const std::vector<double>& expected)
{
    if (!result.ok) {
        return fail(name + ": expected ok, got " + result.diagnosticCode);
    }

    if (result.diagnosticCode != "RS_OK") {
        return fail(name + ": expected RS_OK, got " + result.diagnosticCode);
    }

    if (result.errorOffset != -1) {
        return fail(name + ": expected no error offset");
    }

    if (result.valuesM.size() != expected.size()) {
        return fail(name + ": unexpected value count");
    }

    for (std::size_t index = 0; index < expected.size(); ++index) {
        if (!nearlyEqual(result.valuesM[index], expected[index])) {
            return fail(name + ": unexpected value at index " + std::to_string(index));
        }
    }

    return 0;
}

int expectDiagnostic(const std::string& name,
                     const tsrs::rebarsmart::SpaceListParseResult& result,
                     const std::string& diagnosticCode,
                     int expectedOffset)
{
    if (result.ok) {
        return fail(name + ": expected failure");
    }

    if (result.diagnosticCode != diagnosticCode) {
        return fail(name + ": expected " + diagnosticCode + ", got " + result.diagnosticCode);
    }

    if (result.errorOffset < 0) {
        return fail(name + ": expected diagnostic offset");
    }

    if (result.errorOffset != expectedOffset) {
        return fail(name + ": expected diagnostic offset " + std::to_string(expectedOffset)
                    + ", got " + std::to_string(result.errorOffset));
    }

    return 0;
}

} // namespace

int main()
{
    using tsrs::rebarsmart::SpaceListParseOptions;
    using tsrs::rebarsmart::parseSpaceList;

    const SpaceListParseOptions options{0.01, false};

    std::vector<double> repeated;
    repeated.insert(repeated.end(), 12, 0.20);
    repeated.insert(repeated.end(), 8, 0.25);
    if (const int code = expectOkValues("SL-001", parseSpaceList("20*12,25*8", options), repeated)) {
        return code;
    }

    if (const int code = expectOkValues("SL-002", parseSpaceList("20", options), {0.20})) {
        return code;
    }

    if (const int code = expectOkValues("SL-003", parseSpaceList("20*1", options), {0.20})) {
        return code;
    }

    if (const int code = expectDiagnostic("SL-004",
                                          parseSpaceList("", options),
                                          "RS_SPACE_LIST_EMPTY",
                                          0)) {
        return code;
    }

    if (const int code = expectDiagnostic("SL-005",
                                          parseSpaceList("20*0", options),
                                          "RS_SPACE_LIST_REPEAT_ZERO",
                                          3)) {
        return code;
    }

    if (const int code = expectDiagnostic("SL-006",
                                          parseSpaceList("20*-1", options),
                                          "RS_SPACE_LIST_REPEAT_INVALID",
                                          3)) {
        return code;
    }

    if (const int code = expectDiagnostic("SL-007",
                                          parseSpaceList("20*,25", options),
                                          "RS_SPACE_LIST_TOKEN_INVALID",
                                          3)) {
        return code;
    }

    if (const int code = expectDiagnostic("SL-008",
                                          parseSpaceList("20*12" "\xEF" "\xBC" "\x8C" "25*8", options),
                                          "RS_SPACE_LIST_SEPARATOR_UNSUPPORTED",
                                          5)) {
        return code;
    }

    if (const int code = expectDiagnostic("SL-009",
                                          parseSpaceList("+20", options),
                                          "RS_SPACE_LIST_TOKEN_INVALID",
                                          0)) {
        return code;
    }

    if (const int code = expectDiagnostic("SL-010",
                                          parseSpaceList("0x10", options),
                                          "RS_SPACE_LIST_TOKEN_INVALID",
                                          0)) {
        return code;
    }

    if (const int code = expectDiagnostic("SL-011",
                                          parseSpaceList("20,", options),
                                          "RS_SPACE_LIST_TOKEN_INVALID",
                                          2)) {
        return code;
    }

    if (const int code = expectDiagnostic("SL-012",
                                          parseSpaceList("20*100001", options),
                                          "RS_SPACE_LIST_REPEAT_TOO_LARGE",
                                          3)) {
        return code;
    }

    if (const int code = expectDiagnostic("SL-013",
                                          parseSpaceList("20*60000,25*60000", options),
                                          "RS_SPACE_LIST_REPEAT_TOO_LARGE",
                                          12)) {
        return code;
    }

    const SpaceListParseOptions chineseCommaOptions{0.01, true};
    if (const int code = expectOkValues("SL-014",
                                        parseSpaceList("20" "\xEF" "\xBC" "\x8C" "25",
                                                       chineseCommaOptions),
                                        {0.20, 0.25})) {
        return code;
    }

    const std::string embeddedNull{"20\0,25", 6};
    if (const int code = expectDiagnostic("SL-015",
                                          parseSpaceList(embeddedNull, options),
                                          "RS_SPACE_LIST_TOKEN_INVALID",
                                          0)) {
        return code;
    }

    return 0;
}
