#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace tsrs::rebarsmart {

inline constexpr std::string_view kDiagnosticOk = "RS_OK";
inline constexpr std::string_view kDiagnosticCurveLengthNonPositive = "RS_CURVE_LENGTH_NON_POSITIVE";
inline constexpr std::string_view kDiagnosticSpacingNonPositive = "RS_SPACING_NON_POSITIVE";
inline constexpr std::string_view kDiagnosticCountNonPositive = "RS_COUNT_NON_POSITIVE";
inline constexpr std::string_view kDiagnosticZoneLengthInvalid = "RS_ZONE_LENGTH_INVALID";
inline constexpr std::string_view kDiagnosticSectionCountTooLarge = "RS_SECTION_COUNT_TOO_LARGE";

struct ZoneLengths {
    double headM{0.0};
    double tailM{0.0};
};

struct DistributionResult {
    bool ok{false};
    int count{0};
    double spacingM{0.0};
    double marginM{0.0};
    std::vector<double> pointLengthsM;
    std::string diagnosticCode;
    int errorOffset{-1};
};

} // namespace tsrs::rebarsmart
