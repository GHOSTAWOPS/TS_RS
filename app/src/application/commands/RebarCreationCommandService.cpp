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

} // namespace

RebarCreationCommandService::RebarCreationCommandService(
    tsrs::domain::rebar::RebarModel* model)
    : model_(model)
{
}

RebarCreationCommandResult RebarCreationCommandService::previewFixDistance(
    const RebarCreationPreviewFixDistanceRequest&) const
{
    RebarCreationCommandResult result;
    result.diagnosticCode = kRebarCreationCommandDiagnosticNotImplemented;
    result.diagnostic =
        "Rebar creation command service is a guardrail skeleton.";
    return result;
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
