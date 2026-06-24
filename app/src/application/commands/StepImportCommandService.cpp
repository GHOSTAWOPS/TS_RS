#include "application/commands/StepImportCommandService.h"

#include "application/import/ImportedModelStore.h"
#include "step/ShapeStore.h"
#include "step/StepImportService.h"
#include "step/TopologyBindingRegistry.h"

namespace tsrs::application {

StepImportCommandService::StepImportCommandService(
    ImportedModelStore* importedModelStore)
    : importedModelStore_(importedModelStore)
{
}

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

    if (importedModelStore_ == nullptr) {
        return result;
    }

    const tsrs::step::ShapeStore shapeStore =
        tsrs::step::ShapeStore::fromImportedStep(imported);
    const tsrs::step::TopologyBindingRegistry topologyBindings =
        tsrs::step::TopologyBindingRegistry::build(shapeStore);

    if (!topologyBindings.ok()) {
        result.ok = false;
        result.diagnosticCode = "STEP_TOPOLOGY_BINDING_FAILED";
        result.diagnostic = topologyBindings.diagnostic();
        return result;
    }

    StepSession session;
    session.sourcePath = imported.sourcePath;
    session.displayModel = result.displayModel;
    session.shapeStore = shapeStore;
    session.topologyBindings = topologyBindings;
    const StepSession& stored =
        importedModelStore_->addSession(std::move(session));
    result.sessionId = stored.sessionId;
    return result;
}

} // namespace tsrs::application
