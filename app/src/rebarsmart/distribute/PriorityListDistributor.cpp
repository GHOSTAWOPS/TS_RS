#include "rebarsmart/distribute/PriorityListDistributor.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>

namespace tsrs::rebarsmart {
namespace {

constexpr double kPointEpsilon = 1.0e-9;

DistributionResult makeFailure(std::string_view diagnosticCode)
{
    return DistributionResult{false, 0, 0.0, 0.0, {}, std::string{diagnosticCode}, -1};
}

DistributionResult makeSuccess(std::vector<double> pointLengthsM, double marginM)
{
    const int count = static_cast<int>(pointLengthsM.size());
    return DistributionResult{
        true,
        count,
        0.0,
        marginM,
        std::move(pointLengthsM),
        std::string{kDiagnosticOk},
        -1,
    };
}

bool isValidZone(ZoneLengths zone)
{
    return std::isfinite(zone.headM) && zone.headM >= 0.0
        && std::isfinite(zone.tailM) && zone.tailM >= 0.0;
}

bool isValidSpacingList(const std::vector<double>& spacingListM)
{
    return std::all_of(spacingListM.begin(), spacingListM.end(), [](double spacingM) {
        return std::isfinite(spacingM) && spacingM > 0.0;
    });
}

} // namespace

DistributionResult distributeBySpacingList(double curveLengthM,
                                           ZoneLengths zone,
                                           std::vector<double> spacingListM)
{
    if (!std::isfinite(curveLengthM) || curveLengthM <= 0.0) {
        return makeFailure(kDiagnosticCurveLengthNonPositive);
    }

    if (!isValidZone(zone)) {
        return makeFailure(kDiagnosticZoneLengthInvalid);
    }

    if (spacingListM.empty()) {
        return makeFailure(kDiagnosticSpaceListEmpty);
    }

    if (!isValidSpacingList(spacingListM)) {
        return makeFailure(kDiagnosticSpacingNonPositive);
    }

    const double tailLimitM = curveLengthM - zone.tailM;
    std::vector<double> pointLengthsM;
    pointLengthsM.push_back(zone.headM);

    if (tailLimitM <= zone.headM + kPointEpsilon) {
        return makeFailure(kDiagnosticPointCountTooSmall);
    }

    double currentPointM = zone.headM;
    for (double spacingM : spacingListM) {
        const double nextPointM = currentPointM + spacingM;
        if (nextPointM > tailLimitM + kPointEpsilon) {
            break;
        }

        double candidatePointM = nextPointM;
        if (std::fabs(candidatePointM - tailLimitM) <= kPointEpsilon) {
            candidatePointM = tailLimitM;
        }

        if (candidatePointM <= currentPointM + kPointEpsilon) {
            break;
        }

        currentPointM = candidatePointM;
        if (std::fabs(currentPointM - tailLimitM) <= kPointEpsilon) {
            currentPointM = tailLimitM;
        }
        pointLengthsM.push_back(currentPointM);
    }

    if (pointLengthsM.size() <= 1) {
        return makeFailure(kDiagnosticPointCountTooSmall);
    }

    double marginM = tailLimitM - pointLengthsM.back();
    if (std::fabs(marginM) <= kPointEpsilon) {
        marginM = 0.0;
    }

    return makeSuccess(std::move(pointLengthsM), marginM);
}

} // namespace tsrs::rebarsmart
