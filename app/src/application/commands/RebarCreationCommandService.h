#pragma once

#include <string>

namespace tsrs::application {

inline constexpr const char* kRebarCreationCommandDiagnosticNotImplemented =
    "REBAR_CREATION_COMMAND_NOT_IMPLEMENTED";

struct RebarCreationPreviewFixDistanceRequest {
    std::string commandId;
};

struct RebarCreationCommandResult {
    bool ok{false};
    std::string diagnosticCode{kRebarCreationCommandDiagnosticNotImplemented};
    std::string diagnostic;
};

class RebarCreationCommandService final {
public:
    [[nodiscard]] RebarCreationCommandResult previewFixDistance(
        const RebarCreationPreviewFixDistanceRequest& request) const;
};

} // namespace tsrs::application
