#include "rebarsmart/generators/FixNumberGenerator.h"

#include <cmath>
#include <utility>

namespace tsrs::rebarsmart {
namespace {

FixNumberGenerationResult makeFailure(std::string_view diagnosticCode)
{
    return FixNumberGenerationResult{false, {}, std::string{diagnosticCode}};
}

bool isFiniteNonNegative(double value)
{
    return std::isfinite(value) && value >= 0.0;
}

bool isValidParameters(const FixNumberParameters& parameters,
                       std::string_view& diagnosticCode)
{
    if (parameters.priorityMode != FixNumberPriorityMode::Count) {
        diagnosticCode = kDiagnosticFixNumberPriorityModeUnknown;
        return false;
    }
    if (parameters.count < 2) {
        diagnosticCode = kDiagnosticCountNonPositive;
        return false;
    }
    if (!std::isfinite(parameters.diameterM) || parameters.diameterM <= 0.0) {
        diagnosticCode = kDiagnosticFixNumberDiameterInvalid;
        return false;
    }
    if (parameters.bundleCount < 1) {
        diagnosticCode = kDiagnosticFixNumberBundleCountInvalid;
        return false;
    }
    if (!isFiniteNonNegative(parameters.coverThicknessM)
        || !isFiniteNonNegative(parameters.headDistanceM)
        || !isFiniteNonNegative(parameters.tailDistanceM)) {
        diagnosticCode = kDiagnosticZoneLengthInvalid;
        return false;
    }

    return true;
}

bool intervalPointLength(double curveLengthM,
                         ZoneLengths zone,
                         int count,
                         int index,
                         double& pointLengthM)
{
    if (!std::isfinite(curveLengthM) || curveLengthM <= 0.0) {
        return false;
    }
    if (zone.headM < 0.0 || zone.tailM < 0.0 || zone.headM + zone.tailM > curveLengthM) {
        return false;
    }
    if (count < 2 || index < 0 || index >= count) {
        return false;
    }

    const double usableLengthM = curveLengthM - zone.headM - zone.tailM;
    const double spacingM = usableLengthM / static_cast<double>(count - 1);
    pointLengthM = zone.headM + spacingM * static_cast<double>(index);
    return std::isfinite(pointLengthM);
}

} // namespace

FixNumberGenerationResult generateFixNumberCenterlines(
    const FixNumberGeneratorInput& input,
    geometry::IGeometryEngine& geometry)
{
    std::string_view diagnosticCode;
    if (!isValidParameters(input.parameters, diagnosticCode)) {
        return makeFailure(diagnosticCode);
    }

    if (input.selection.mainGuideCurves.size() < 2) {
        return makeFailure(kDiagnosticFixNumberGuideCurveCountInvalid);
    }

    std::vector<double> guideLengthsM;
    guideLengthsM.reserve(input.selection.mainGuideCurves.size());
    for (const auto& guideCurve : input.selection.mainGuideCurves) {
        const auto length = geometry.curveLength(guideCurve);
        if (!length.ok) {
            return makeFailure(kDiagnosticFixNumberGuideCurveInvalid);
        }
        guideLengthsM.push_back(length.lengthM);
    }

    const ZoneLengths zone{input.parameters.headDistanceM, input.parameters.tailDistanceM};
    std::vector<FixNumberGeneratedCenterline> centerlines;
    centerlines.reserve(
        (input.selection.mainGuideCurves.size() - 1) * static_cast<std::size_t>(input.parameters.count));

    for (std::size_t guideIndex = 0; guideIndex + 1 < input.selection.mainGuideCurves.size();
         ++guideIndex) {
        for (int pointIndex = 0; pointIndex < input.parameters.count; ++pointIndex) {
            double firstPointLengthM = 0.0;
            double secondPointLengthM = 0.0;
            if (!intervalPointLength(guideLengthsM[guideIndex],
                                     zone,
                                     input.parameters.count,
                                     pointIndex,
                                     firstPointLengthM)
                || !intervalPointLength(guideLengthsM[guideIndex + 1],
                                        zone,
                                        input.parameters.count,
                                        pointIndex,
                                        secondPointLengthM)) {
                return makeFailure(kDiagnosticZoneLengthInvalid);
            }

            const auto firstPoint =
                geometry.pointAtLength(input.selection.mainGuideCurves[guideIndex],
                                       firstPointLengthM);
            const auto secondPoint =
                geometry.pointAtLength(input.selection.mainGuideCurves[guideIndex + 1],
                                       secondPointLengthM);
            if (!firstPoint.ok || !secondPoint.ok) {
                return makeFailure(kDiagnosticFixNumberGuideCurveInvalid);
            }

            const auto centerline =
                geometry.makePolylineCurve({firstPoint.point, secondPoint.point});
            if (!centerline.ok) {
                return makeFailure(kDiagnosticFixNumberCenterlineInvalid);
            }

            centerlines.push_back(FixNumberGeneratedCenterline{
                centerline.ref,
                guideIndex,
                firstPointLengthM,
                secondPointLengthM,
            });
        }
    }

    return FixNumberGenerationResult{true, std::move(centerlines), std::string{kDiagnosticOk}};
}

} // namespace tsrs::rebarsmart
