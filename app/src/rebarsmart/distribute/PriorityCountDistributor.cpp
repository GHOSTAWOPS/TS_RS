#include "rebarsmart/distribute/PriorityCountDistributor.h"

#include <algorithm>
#include <cmath>

namespace tsrs::rebarsmart {
namespace {

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

DistributionResult distributeByCount(double curveLengthM, ZoneLengths zone, int count)
{
    if (!std::isfinite(curveLengthM) || curveLengthM <= 0.0) {
        return makeFailure(kDiagnosticCurveLengthNonPositive);
    }

    if (count <= 0) {
        return makeFailure(kDiagnosticCountNonPositive);
    }

    if (!isValidZone(zone)) {
        return makeFailure(kDiagnosticZoneLengthInvalid);
    }

    const double usableLengthM = std::max(0.0, curveLengthM - zone.headM - zone.tailM);

    if (count > 1) {
        return makeSuccess(count, usableLengthM / static_cast<double>(count - 1), 0.0);
    }

    return makeSuccess(count, 2.0 * usableLengthM, 0.0);
}

} // namespace tsrs::rebarsmart
