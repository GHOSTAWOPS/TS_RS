#include "application/commands/StepImportCommandService.h"

#include "step/StepImportService.h"

namespace tsrs::application {

StepImportCommandResult StepImportCommandService::importStep(
    const StepImportCommandRequest& request) const
{
    const tsrs::step::StepImportService stepImportService;
    const tsrs::step::StepImportResult imported = stepImportService.importFile(request.path);

    StepImportCommandResult result;
    result.ok = imported.ok;
    result.diagnosticCode = imported.diagnosticCode;
    result.diagnostic = imported.diagnostic;
    result.sourcePath = imported.sourcePath;

    if (!imported.ok) {
        return result;
    }

    result.displayModel.sourcePath = imported.sourcePath;
    result.displayModel.rootCount = imported.rootCount;
    result.displayModel.solidCount = imported.solidCount;
    result.displayModel.faceCount = imported.faceCount;
    result.displayModel.edgeCount = imported.edgeCount;
    result.displayModel.vertexCount = imported.vertexCount;
    result.displayModel.shape = imported.rootShape;
    return result;
}

} // namespace tsrs::application
