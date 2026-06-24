#include "application/commands/RebarCreationCommandService.h"

namespace tsrs::application {

RebarCreationCommandResult RebarCreationCommandService::previewFixDistance(
    const RebarCreationPreviewFixDistanceRequest&) const
{
    RebarCreationCommandResult result;
    result.diagnosticCode = kRebarCreationCommandDiagnosticNotImplemented;
    result.diagnostic =
        "Rebar creation command service is a guardrail skeleton.";
    return result;
}

} // namespace tsrs::application
