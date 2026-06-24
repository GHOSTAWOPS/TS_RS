#include "application/commands/DetailPackageCommandService.h"
#include "application/commands/RebarCreationCommandService.h"
#include "application/commands/SelectionCommandService.h"
#include "step/TopologyBindingRegistry.h"

#include <iostream>
#include <string>

namespace {

int fail(const std::string& message)
{
    std::cerr << message << '\n';
    return 1;
}

tsrs::step::TopologyBinding makeEdgeBinding()
{
    tsrs::step::TopologyBinding binding;
    binding.kind = tsrs::step::kTopologyKindEdge;
    binding.localIndex = 7;
    binding.stableId = "edge:7:demo";
    binding.geometryFingerprint = "edge-fingerprint-demo";
    binding.measure = 12.5;
    binding.bbox = {0.0, 0.0, 0.0, 10.0, 0.0, 0.0};
    return binding;
}

int expectSelectionServiceRestoresStableBinding()
{
    const tsrs::step::TopologyBinding binding = makeEdgeBinding();
    const tsrs::step::TopologyBindingRegistry registry =
        tsrs::step::TopologyBindingRegistry::fromBindings({binding});

    const tsrs::application::SelectionCommandService service(&registry);
    const tsrs::application::SelectionCommandResult result =
        service.resolveSelection(
            {"mainGuideEdge",
             tsrs::step::makeBindingReference("mainGuideEdge", binding)});

    if (!result.ok) {
        return fail("selection command must resolve a registry binding: "
                    + result.diagnosticCode);
    }
    if (result.diagnosticCode
        != tsrs::application::kSelectionCommandDiagnosticOk) {
        return fail("selection command success diagnostic mismatch: "
                    + result.diagnosticCode);
    }
    if (result.bindingStableId != binding.stableId) {
        return fail("selection command must expose stable binding id");
    }
    if (result.kind != tsrs::step::kTopologyKindEdge) {
        return fail("selection command must preserve binding kind");
    }
    return 0;
}

int expectSelectionServiceRejectsMissingRegistry()
{
    const tsrs::application::SelectionCommandService service(nullptr);
    const tsrs::application::SelectionCommandResult result =
        service.resolveSelection({"mainGuideEdge", {}});

    if (result.ok) {
        return fail("selection command without registry must fail");
    }
    if (result.diagnosticCode
        != tsrs::application::kSelectionCommandDiagnosticRegistryMissing) {
        return fail("selection command missing registry diagnostic mismatch: "
                    + result.diagnosticCode);
    }
    return 0;
}

int expectDetailCommandServiceIsGuardrailOnly()
{
    const tsrs::application::DetailPackageCommandService service;
    const tsrs::application::DetailPackageCommandResult result =
        service.exportCurrentProject({});

    if (result.ok) {
        return fail("detail command skeleton must not export without project state");
    }
    if (result.diagnosticCode
        != tsrs::application::kDetailPackageCommandDiagnosticNotImplemented) {
        return fail("detail command skeleton diagnostic mismatch: "
                    + result.diagnosticCode);
    }
    return 0;
}

int expectRebarCommandServiceRequiresModelBeforeGeneration()
{
    const tsrs::application::RebarCreationCommandService service;
    const tsrs::application::RebarCreationCommandResult result =
        service.previewFixDistance({});

    if (result.ok) {
        return fail("rebar creation command without model must not generate rebar");
    }
    if (result.diagnosticCode
        != tsrs::application::kRebarCreationCommandDiagnosticModelMissing) {
        return fail("rebar command missing model diagnostic mismatch: "
                    + result.diagnosticCode);
    }
    return 0;
}

} // namespace

int main()
{
    if (const int code = expectSelectionServiceRestoresStableBinding()) {
        return code;
    }
    if (const int code = expectSelectionServiceRejectsMissingRegistry()) {
        return code;
    }
    if (const int code = expectDetailCommandServiceIsGuardrailOnly()) {
        return code;
    }
    if (const int code = expectRebarCommandServiceRequiresModelBeforeGeneration()) {
        return code;
    }
    return 0;
}
