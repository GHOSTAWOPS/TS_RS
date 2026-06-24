#pragma once

#include <string>
#include <vector>

namespace tsrs::domain::rebar {

inline constexpr const char* kRebarModelDiagnosticOk = "REBAR_MODEL_OK";
inline constexpr const char* kRebarModelDiagnosticInvalidDraft =
    "REBAR_MODEL_INVALID_DRAFT";
inline constexpr const char* kRebarModelDiagnosticNoPreview =
    "REBAR_MODEL_NO_PREVIEW";

struct RebarGroupDraft {
    std::string groupId;
    std::string commandId;
    std::string styleName;
    std::string grade;
    double diameterM{0.0};
    int bundleCount{1};
    std::vector<std::string> centerlineStableIds;
};

struct RebarModelTransactionResult {
    bool ok{false};
    std::string diagnosticCode{kRebarModelDiagnosticInvalidDraft};
    std::string diagnostic;
};

class RebarModel final {
public:
    [[nodiscard]] RebarModelTransactionResult replacePreview(
        std::string commandId,
        RebarGroupDraft draft);

    [[nodiscard]] RebarModelTransactionResult commitPreview();
    [[nodiscard]] RebarModelTransactionResult cancelPreview();

    [[nodiscard]] bool hasPreview() const;
    [[nodiscard]] bool isDirty() const;
    [[nodiscard]] std::size_t committedGroupCount() const;
    [[nodiscard]] const std::vector<RebarGroupDraft>& committedGroups() const;

private:
    [[nodiscard]] static bool isValidDraft(const RebarGroupDraft& draft);

    bool hasPreview_{false};
    bool dirty_{false};
    std::string previewCommandId_;
    RebarGroupDraft preview_;
    std::vector<RebarGroupDraft> committedGroups_;
};

} // namespace tsrs::domain::rebar
