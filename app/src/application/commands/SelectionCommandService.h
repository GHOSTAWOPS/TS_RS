#pragma once

#include "step/TopologyBindingRegistry.h"

#include <string>

namespace tsrs::application {

inline constexpr const char* kSelectionCommandDiagnosticOk =
    "SELECTION_COMMAND_OK";
inline constexpr const char* kSelectionCommandDiagnosticRegistryMissing =
    "SELECTION_REGISTRY_MISSING";

struct SelectionCommandRequest {
    std::string role;
    tsrs::step::TopologyBindingReference reference;
};

struct SelectionCommandResult {
    bool ok{false};
    std::string diagnosticCode{kSelectionCommandDiagnosticRegistryMissing};
    std::string diagnostic;
    std::string role;
    std::string kind;
    std::string bindingStableId;
    bool usedFallback{false};
};

class SelectionCommandService final {
public:
    explicit SelectionCommandService(
        const tsrs::step::TopologyBindingRegistry* registry);

    [[nodiscard]] SelectionCommandResult resolveSelection(
        const SelectionCommandRequest& request) const;

private:
    const tsrs::step::TopologyBindingRegistry* registry_{nullptr};
};

} // namespace tsrs::application
