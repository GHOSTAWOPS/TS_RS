#include "rebarsmart/distribute/PrioritySpaceDistributor.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace tsrs::rebarsmart {
namespace {

constexpr double kSectionEpsilon = 1.0e-9;
constexpr int kMaxSectionCount = 100000000;

DistributionResult makeFailure(std::string_view diagnosticCode)
{
    return DistributionResult{false, 0, 0.0, 0.0, {}, std::string{diagnosticCode}, -1};
}

DistributionResult makeSuccess(int count, double spacingM, double marginM)
{
    return DistributionResult{true, count, spacingM, marginM, {}, std::string{kDiagnosticOk}, -1};
}

bool isValidZone(ZoneLengths zone)
{
    return std::isfinite(zone.headM) && zone.headM >= 0.0
        && std::isfinite(zone.tailM) && zone.tailM >= 0.0;
}

} // namespace

DistributionResult distributeBySpacing(double curveLengthM,
                                       ZoneLengths zone,
                                       double spacingM,
                                       double /*spacingRatio*/)
{
    if (!std::isfinite(curveLengthM) || curveLengthM <= 0.0) {
        return makeFailure(kDiagnosticCurveLengthNonPositive);
    }

    if (!std::isfinite(spacingM) || spacingM <= 0.0) {
        return makeFailure(kDiagnosticSpacingNonPositive);
    }

    if (!isValidZone(zone)) {
        return makeFailure(kDiagnosticZoneLengthInvalid);
    }

    const double usableLengthM = std::max(0.0, curveLengthM - zone.headM - zone.tailM);
    const double rawSectionCount = usableLengthM / spacingM;
    if (!std::isfinite(rawSectionCount)
        || rawSectionCount > static_cast<double>(kMaxSectionCount)) {
        return makeFailure(kDiagnosticSectionCountTooLarge);
    }

    const double nearestSectionCount = std::round(rawSectionCount);
    const double sectionCountDouble =
        std::fabs(rawSectionCount - nearestSectionCount) <= kSectionEpsilon
            ? nearestSectionCount
            : std::floor(rawSectionCount);

    if (sectionCountDouble > static_cast<double>(std::numeric_limits<int>::max() - 1)) {
        return makeFailure(kDiagnosticSectionCountTooLarge);
    }

    const int sectionCount = static_cast<int>(sectionCountDouble);
    const int pointCount = sectionCount + 1;
    double marginM = usableLengthM - static_cast<double>(sectionCount) * spacingM;
    if (std::fabs(marginM) <= kSectionEpsilon) {
        marginM = 0.0;
    }

    return makeSuccess(pointCount, spacingM, marginM);
}

} // namespace tsrs::rebarsmart
