#include "rebarsmart/zone/GuideSurfaceOffsetCalculator.h"

#include "rebarsmart/distribute/DistributionTypes.h"

#include <cmath>

namespace tsrs::rebarsmart {
namespace {

GuideSurfaceOffsetResult makeFailure(const char* diagnosticCode, double offsetM = 0.0)
{
    return GuideSurfaceOffsetResult{false, offsetM, diagnosticCode};
}

GuideSurfaceOffsetResult makeSuccess(double offsetM)
{
    return GuideSurfaceOffsetResult{true, offsetM, std::string{kDiagnosticOk}};
}

bool isFiniteNonNegative(double value)
{
    return std::isfinite(value) && value >= 0.0;
}

bool isValidInput(GuideSurfaceOffsetInput input)
{
    return input.activeLayerIndex >= 0
        && isFiniteNonNegative(input.diameterM)
        && isFiniteNonNegative(input.coverThicknessM)
        && isFiniteNonNegative(input.layerSpacingM)
        && isFiniteNonNegative(input.firstLayerProtectThicknessNetM);
}

} // namespace

GuideSurfaceOffsetResult calculateGuideSurfaceOffset(GuideSurfaceOffsetInput input)
{
    if (!isValidInput(input)) {
        return makeFailure(kDiagnosticGuideSurfaceOffsetInvalid);
    }

    const double halfDiameterM = 0.5 * input.diameterM;
    double baseOffsetM = 0.0;

    switch (input.offsetMode) {
    case 0:
    case 2:
        baseOffsetM = input.coverThicknessM + halfDiameterM;
        break;
    case 1:
        baseOffsetM = input.coverThicknessM - halfDiameterM;
        break;
    case 3:
    case 4:
        baseOffsetM = input.firstLayerProtectThicknessNetM + halfDiameterM;
        break;
    case 5:
        baseOffsetM = input.coverThicknessM;
        break;
    default:
        return makeFailure(kDiagnosticGuideSurfaceOffsetModeUnknown, 0.0);
    }

    const double offsetM =
        baseOffsetM + static_cast<double>(input.activeLayerIndex) * input.layerSpacingM;
    if (!std::isfinite(offsetM)) {
        return makeFailure(kDiagnosticGuideSurfaceOffsetInvalid);
    }

    return makeSuccess(offsetM);
}

} // namespace tsrs::rebarsmart
