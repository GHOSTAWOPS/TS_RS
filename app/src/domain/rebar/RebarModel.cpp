#include "domain/rebar/RebarModel.h"

#include <utility>

namespace tsrs::domain::rebar {

namespace {

RebarModelTransactionResult okResult()
{
    return {true, kRebarModelDiagnosticOk, {}};
}

RebarModelTransactionResult failResult(const char* code, std::string diagnostic)
{
    return {false, code, std::move(diagnostic)};
}

} // namespace

RebarModelTransactionResult RebarModel::replacePreview(
    std::string commandId,
    RebarGroupDraft draft)
{
    if (commandId.empty() || !isValidDraft(draft)) {
        return failResult(kRebarModelDiagnosticInvalidDraft,
                          "Preview draft requires command id, group id, and centerlines.");
    }

    draft.commandId = commandId;
    previewCommandId_ = std::move(commandId);
    preview_ = std::move(draft);
    hasPreview_ = true;
    return okResult();
}

RebarModelTransactionResult RebarModel::commitPreview()
{
    if (!hasPreview_) {
        return failResult(kRebarModelDiagnosticNoPreview,
                          "Cannot commit because no preview is active.");
    }

    committedGroups_.push_back(preview_);
    preview_ = {};
    previewCommandId_.clear();
    hasPreview_ = false;
    dirty_ = true;
    return okResult();
}

RebarModelTransactionResult RebarModel::cancelPreview()
{
    if (!hasPreview_) {
        return failResult(kRebarModelDiagnosticNoPreview,
                          "Cannot cancel because no preview is active.");
    }

    preview_ = {};
    previewCommandId_.clear();
    hasPreview_ = false;
    return okResult();
}

bool RebarModel::hasPreview() const
{
    return hasPreview_;
}

bool RebarModel::isDirty() const
{
    return dirty_;
}

std::size_t RebarModel::committedGroupCount() const
{
    return committedGroups_.size();
}

const std::vector<RebarGroupDraft>& RebarModel::committedGroups() const
{
    return committedGroups_;
}

bool RebarModel::isValidDraft(const RebarGroupDraft& draft)
{
    return !draft.groupId.empty() && !draft.centerlineStableIds.empty();
}

} // namespace tsrs::domain::rebar
