#include "application/commands/RebarCreationCommandService.h"

#include <utility>

namespace tsrs::application {

namespace {

RebarCreationCommandResult okResult()
{
    return {true, kRebarCreationCommandDiagnosticOk, {}};
}

RebarCreationCommandResult failResult(std::string code, std::string diagnostic)
{
    return {false, std::move(code), std::move(diagnostic)};
}

RebarCreationCommandResult fromModelResult(
    const tsrs::domain::rebar::RebarModelTransactionResult& result)
{
    if (result.ok) {
        return okResult();
    }
    return failResult(result.diagnosticCode, result.diagnostic);
}

tsrs::domain::rebar::RebarGroupDraft toDraft(
    const RebarCreationPreviewFixDistanceRequest& request,
    const RebarCreationGenerationResult& generated)
{
    tsrs::domain::rebar::RebarGroupDraft draft;
    draft.groupId = request.groupId;
    draft.commandId = request.commandId;
    draft.styleName = request.input.styleName;
    draft.grade = request.input.grade;
    draft.diameterM = request.input.diameterM;
    draft.bundleCount = request.input.bundleCount;
    draft.centerlineStableIds.reserve(generated.centerlines.size());
    for (const auto& centerline : generated.centerlines) {
        draft.centerlineStableIds.push_back(centerline.curveRef.stableId);
    }
    return draft;
}

} // namespace

RebarCreationCommandService::RebarCreationCommandService(
    tsrs::domain::rebar::RebarModel* model,
    const IFixDistanceCenterlineGenerator* fixDistanceGenerator)
    : model_(model)
    , fixDistanceGenerator_(fixDistanceGenerator)
{
}

RebarCreationCommandResult RebarCreationCommandService::previewFixDistance(
    const RebarCreationPreviewFixDistanceRequest& request) const
{
    if (model_ == nullptr) {
        return failResult(kRebarCreationCommandDiagnosticModelMissing,
                          "Rebar creation command service requires RebarModel.");
    }
    if (request.geometry == nullptr) {
        return failResult(kRebarCreationCommandDiagnosticGeometryMissing,
                          "FixDistance preview requires geometry engine.");
    }
    if (fixDistanceGenerator_ == nullptr) {
        return failResult(kRebarCreationCommandDiagnosticGeneratorMissing,
                          "FixDistance preview requires generator port.");
    }

    const auto generated =
        fixDistanceGenerator_->generate(request.input, *request.geometry);
    if (!generated.ok) {
        return failResult(generated.diagnosticCode,
                          "FixDistance generator failed.");
    }

    return fromModelResult(
        model_->replacePreview(request.commandId, toDraft(request, generated)));
}

RebarCreationCommandResult RebarCreationCommandService::previewDraft(
    const RebarCreationPreviewDraftRequest& request) const
{
    if (model_ == nullptr) {
        return failResult(kRebarCreationCommandDiagnosticModelMissing,
                          "Rebar creation command service requires RebarModel.");
    }

    return fromModelResult(model_->replacePreview(request.commandId, request.draft));
}

RebarCreationCommandResult RebarCreationCommandService::commitPreview() const
{
    if (model_ == nullptr) {
        return failResult(kRebarCreationCommandDiagnosticModelMissing,
                          "Rebar creation command service requires RebarModel.");
    }

    return fromModelResult(model_->commitPreview());
}

RebarCreationCommandResult RebarCreationCommandService::cancelPreview() const
{
    if (model_ == nullptr) {
        return failResult(kRebarCreationCommandDiagnosticModelMissing,
                          "Rebar creation command service requires RebarModel.");
    }

    return fromModelResult(model_->cancelPreview());
}

} // namespace tsrs::application
