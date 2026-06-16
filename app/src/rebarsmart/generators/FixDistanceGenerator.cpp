#include "rebarsmart/generators/FixDistanceGenerator.h"

#include "rebarsmart/distribute/PriorityCountDistributor.h"
#include "rebarsmart/distribute/PriorityListDistributor.h"
#include "rebarsmart/distribute/PrioritySpaceDistributor.h"
#include "rebarsmart/space_list/SpaceListParser.h"

#include <cmath>
#include <utility>

namespace tsrs::rebarsmart {
namespace {

FixDistanceGenerationResult makeFailure(std::string_view diagnosticCode,
                                        DistributionResult distribution = {})
{
    if (distribution.diagnosticCode.empty()) {
        distribution.diagnosticCode = std::string{diagnosticCode};
    }
    return FixDistanceGenerationResult{false, {}, std::move(distribution),
                                       std::string{diagnosticCode}};
}

FixDistanceGenerationResult makeDistributionFailure(DistributionResult distribution)
{
    const std::string diagnosticCode = distribution.diagnosticCode;
    return FixDistanceGenerationResult{false, {}, std::move(distribution), diagnosticCode};
}

DistributionResult dispatchDistribution(const FixDistanceParameters& parameters,
                                        double mainCurveLengthM)
{
    switch (parameters.priorityMode) {
    case FixDistancePriorityMode::Spacing:
        return distributeBySpacing(mainCurveLengthM,
                                   parameters.zone,
                                   parameters.spacingM,
                                   parameters.spacingRatio);
    case FixDistancePriorityMode::Count:
        return distributeByCount(mainCurveLengthM, parameters.zone, parameters.count);
    case FixDistancePriorityMode::SpacingList: {
        const auto parseResult =
            parseSpaceList(parameters.spacingListText, SpaceListParseOptions{0.01, false});
        if (!parseResult.ok) {
            return DistributionResult{
                false,
                0,
                0.0,
                0.0,
                {},
                parseResult.diagnosticCode,
                parseResult.errorOffset,
            };
        }
        return distributeBySpacingList(mainCurveLengthM, parameters.zone, parseResult.valuesM);
    }
    }

    return DistributionResult{false, 0, 0.0, 0.0, {},
                              std::string{kDiagnosticFixDistancePriorityModeUnknown}, -1};
}

std::vector<double> pointLengthsFromDistribution(const DistributionResult& distribution,
                                                ZoneLengths zone)
{
    if (!distribution.pointLengthsM.empty()) {
        return distribution.pointLengthsM;
    }

    std::vector<double> pointLengthsM;
    pointLengthsM.reserve(static_cast<std::size_t>(distribution.count));
    double currentM = zone.headM;
    for (int index = 0; index < distribution.count; ++index) {
        pointLengthsM.push_back(currentM);
        currentM += distribution.spacingM;
    }
    return pointLengthsM;
}

geometry::GeometryPoint3d translatedPoint(geometry::GeometryPoint3d start,
                                          geometry::GeometryVector3d vector)
{
    return geometry::GeometryPoint3d{
        start.x + vector.x,
        start.y + vector.y,
        start.z + vector.z,
    };
}

geometry::GeometryVector3d vectorBetween(geometry::GeometryPoint3d start,
                                         geometry::GeometryPoint3d end)
{
    return geometry::GeometryVector3d{end.x - start.x, end.y - start.y, end.z - start.z};
}

bool isNonZeroVector(geometry::GeometryVector3d vector)
{
    return std::isfinite(vector.x) && std::isfinite(vector.y) && std::isfinite(vector.z)
        && (vector.x != 0.0 || vector.y != 0.0 || vector.z != 0.0);
}

} // namespace

FixDistanceGenerationResult generateFixDistanceCenterlines(
    const FixDistanceGeneratorInput& input,
    geometry::IGeometryEngine& geometry)
{
    const auto mainLength = geometry.curveLength(input.mainGuideCurve);
    if (!mainLength.ok) {
        return makeFailure(kDiagnosticFixDistanceMainCurveInvalid);
    }

    const auto auxiliaryLength = geometry.curveLength(input.auxiliaryGuideCurve);
    if (!auxiliaryLength.ok) {
        return makeFailure(kDiagnosticFixDistanceAuxiliaryCurveInvalid);
    }

    const auto auxiliaryStart = geometry.pointAtLength(input.auxiliaryGuideCurve, 0.0);
    const auto auxiliaryEnd = geometry.pointAtLength(input.auxiliaryGuideCurve,
                                                    auxiliaryLength.lengthM);
    if (!auxiliaryStart.ok || !auxiliaryEnd.ok) {
        return makeFailure(kDiagnosticFixDistanceAuxiliaryCurveInvalid);
    }

    const auto auxiliaryVector = vectorBetween(auxiliaryStart.point, auxiliaryEnd.point);
    if (!isNonZeroVector(auxiliaryVector)) {
        return makeFailure(kDiagnosticFixDistanceAuxiliaryCurveInvalid);
    }

    auto distribution = dispatchDistribution(input.parameters, mainLength.lengthM);
    if (!distribution.ok) {
        return makeDistributionFailure(std::move(distribution));
    }

    std::vector<GeneratedCenterline> centerlines;
    const auto pointLengthsM = pointLengthsFromDistribution(distribution, input.parameters.zone);
    centerlines.reserve(pointLengthsM.size());

    for (double pointLengthM : pointLengthsM) {
        const auto mainPoint = geometry.pointAtLength(input.mainGuideCurve, pointLengthM);
        if (!mainPoint.ok) {
            return makeFailure(kDiagnosticFixDistanceMainCurveInvalid, std::move(distribution));
        }

        const auto centerline = geometry.makePolylineCurve({
            mainPoint.point,
            translatedPoint(mainPoint.point, auxiliaryVector),
        });
        if (!centerline.ok) {
            return makeFailure(kDiagnosticFixDistanceCenterlineInvalid, std::move(distribution));
        }

        centerlines.push_back(GeneratedCenterline{centerline.ref, pointLengthM});
    }

    return FixDistanceGenerationResult{
        true,
        std::move(centerlines),
        std::move(distribution),
        std::string{kDiagnosticOk},
    };
}

} // namespace tsrs::rebarsmart
