#pragma once

#include "domain/rebar/RebarModel.h"

#include <string>

namespace tsrs::application {

inline constexpr const char* kRebarCreationCommandDiagnosticNotImplemented =
    "REBAR_CREATION_COMMAND_NOT_IMPLEMENTED";
inline constexpr const char* kRebarCreationCommandDiagnosticOk =
    "REBAR_CREATION_COMMAND_OK";
inline constexpr const char* kRebarCreationCommandDiagnosticModelMissing =
    "REBAR_CREATION_MODEL_MISSING";

struct RebarCreationPreviewFixDistanceRequest {
    std::string commandId;
};

struct RebarCreationPreviewDraftRequest {
    std::string commandId;
    tsrs::domain::rebar::RebarGroupDraft draft;
};

struct RebarCreationCommandResult {
    bool ok{false};
    std::string diagnosticCode{kRebarCreationCommandDiagnosticNotImplemented};
    std::string diagnostic;
};

class RebarCreationCommandService final {
public:
    explicit RebarCreationCommandService(
        tsrs::domain::rebar::RebarModel* model = nullptr);

    [[nodiscard]] RebarCreationCommandResult previewFixDistance(
        const RebarCreationPreviewFixDistanceRequest& request) const;

    [[nodiscard]] RebarCreationCommandResult previewDraft(
        const RebarCreationPreviewDraftRequest& request) const;

    [[nodiscard]] RebarCreationCommandResult commitPreview() const;
    [[nodiscard]] RebarCreationCommandResult cancelPreview() const;

private:
    tsrs::domain::rebar::RebarModel* model_{nullptr};
};

} // namespace tsrs::application
