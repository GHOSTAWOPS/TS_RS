#include "application/commands/SelectionCommandService.h"

namespace tsrs::application {

SelectionCommandService::SelectionCommandService(
    const tsrs::step::TopologyBindingRegistry* registry)
    : registry_(registry)
{
}

SelectionCommandResult SelectionCommandService::resolveSelection(
    const SelectionCommandRequest& request) const
{
    SelectionCommandResult result;
    result.role = request.role;

    if (registry_ == nullptr) {
        result.diagnosticCode = kSelectionCommandDiagnosticRegistryMissing;
        result.diagnostic = "Selection registry is missing.";
        return result;
    }

    const tsrs::step::TopologyBindingLookupResult restored =
        registry_->restore(request.reference);
    result.ok = restored.ok;
    result.diagnosticCode = restored.diagnosticCode;
    result.diagnostic = restored.diagnostic;
    result.kind = restored.binding.kind;
    result.bindingStableId = restored.binding.stableId;
    result.usedFallback = restored.usedFallback;

    if (result.ok) {
        result.diagnosticCode = kSelectionCommandDiagnosticOk;
    }
    return result;
}

} // namespace tsrs::application
