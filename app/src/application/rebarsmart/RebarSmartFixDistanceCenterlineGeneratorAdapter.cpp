#include "application/rebarsmart/RebarSmartFixDistanceCenterlineGeneratorAdapter.h"

#include "rebarsmart/generators/FixDistanceGenerator.h"

namespace tsrs::application {

namespace {

tsrs::rebarsmart::FixDistancePriorityMode toGeneratorPriorityMode(
    RebarCreationFixDistancePriorityMode priorityMode)
{
    switch (priorityMode) {
    case RebarCreationFixDistancePriorityMode::Spacing:
        return tsrs::rebarsmart::FixDistancePriorityMode::Spacing;
    case RebarCreationFixDistancePriorityMode::Count:
        return tsrs::rebarsmart::FixDistancePriorityMode::Count;
    case RebarCreationFixDistancePriorityMode::SpacingList:
        return tsrs::rebarsmart::FixDistancePriorityMode::SpacingList;
    }
    return tsrs::rebarsmart::FixDistancePriorityMode::Spacing;
}

tsrs::rebarsmart::FixDistanceGeneratorInput toGeneratorInput(
    const RebarCreationFixDistanceInput& input)
{
    tsrs::rebarsmart::FixDistanceGeneratorInput generatorInput;
    generatorInput.mainGuideCurve = input.mainGuideCurve;
    generatorInput.auxiliaryGuideCurve = input.auxiliaryGuideCurve;
    generatorInput.parameters.styleName = input.styleName;
    generatorInput.parameters.grade = input.grade;
    generatorInput.parameters.diameterM = input.diameterM;
    generatorInput.parameters.spacingM = input.spacingM;
    generatorInput.parameters.count = input.count;
    generatorInput.parameters.bundleCount = input.bundleCount;
    generatorInput.parameters.priorityMode =
        toGeneratorPriorityMode(input.priorityMode);
    generatorInput.parameters.spacingRatio = input.spacingRatio;
    generatorInput.parameters.zone = tsrs::rebarsmart::ZoneLengths{
        input.headZoneLengthM,
        input.tailZoneLengthM,
    };
    generatorInput.parameters.spacingListText = input.spacingListText;
    return generatorInput;
}

RebarCreationGenerationResult toApplicationResult(
    const tsrs::rebarsmart::FixDistanceGenerationResult& result)
{
    RebarCreationGenerationResult converted;
    converted.ok = result.ok;
    converted.diagnosticCode = result.diagnosticCode;
    converted.centerlines.reserve(result.centerlines.size());
    for (const auto& centerline : result.centerlines) {
        converted.centerlines.push_back({centerline.curveRef});
    }
    return converted;
}

} // namespace

RebarCreationGenerationResult RebarSmartFixDistanceCenterlineGeneratorAdapter::
    generate(const RebarCreationFixDistanceInput& input,
             tsrs::geometry::IGeometryEngine& geometry) const
{
    return toApplicationResult(
        tsrs::rebarsmart::generateFixDistanceCenterlines(
            toGeneratorInput(input),
            geometry));
}

} // namespace tsrs::application
