#include "application/commands/DetailPackageCommandService.h"

namespace tsrs::application {

DetailPackageCommandResult DetailPackageCommandService::exportCurrentProject(
    const DetailPackageExportRequest&) const
{
    DetailPackageCommandResult result;
    result.diagnosticCode = kDetailPackageCommandDiagnosticNotImplemented;
    result.diagnostic =
        "Detail export command service is a guardrail skeleton.";
    return result;
}

} // namespace tsrs::application
