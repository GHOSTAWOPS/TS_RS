#include "application/commands/RebarCreationCommandService.h"
#include "domain/rebar/RebarModel.h"

#include <iostream>
#include <string>

namespace {

int fail(const std::string& message)
{
    std::cerr << message << '\n';
    return 1;
}

tsrs::domain::rebar::RebarGroupDraft makeDraft(std::string groupId)
{
    tsrs::domain::rebar::RebarGroupDraft draft;
    draft.groupId = std::move(groupId);
    draft.commandId = "fix-distance";
    draft.styleName = "D25";
    draft.grade = "HRB400";
    draft.diameterM = 0.025;
    draft.bundleCount = 1;
    draft.centerlineStableIds = {"centerline-1", "centerline-2"};
    return draft;
}

int expectServiceRequiresModel()
{
    const tsrs::application::RebarCreationCommandService service;
    const auto result =
        service.previewDraft({"cmd-missing-model", makeDraft("group-a")});

    if (result.ok) {
        return fail("previewDraft without model must fail");
    }
    if (result.diagnosticCode
        != tsrs::application::kRebarCreationCommandDiagnosticModelMissing) {
        return fail("missing model diagnostic mismatch: "
                    + result.diagnosticCode);
    }

    const auto commit = service.commitPreview();
    if (commit.ok || commit.diagnosticCode
                         != tsrs::application::kRebarCreationCommandDiagnosticModelMissing) {
        return fail("commitPreview without model must return MODEL_MISSING");
    }

    const auto cancel = service.cancelPreview();
    if (cancel.ok || cancel.diagnosticCode
                         != tsrs::application::kRebarCreationCommandDiagnosticModelMissing) {
        return fail("cancelPreview without model must return MODEL_MISSING");
    }

    return 0;
}

int expectPreviewCommitCancelUseModelTransaction()
{
    tsrs::domain::rebar::RebarModel model;
    tsrs::application::RebarCreationCommandService service(&model);

    const auto preview =
        service.previewDraft({"cmd-preview", makeDraft("group-preview")});
    if (!preview.ok) {
        return fail("previewDraft must accept valid draft: "
                    + preview.diagnosticCode);
    }
    if (!model.hasPreview()) {
        return fail("previewDraft must create RebarModel preview");
    }
    if (model.committedGroupCount() != 0 || model.isDirty()) {
        return fail("previewDraft must not commit or dirty model");
    }

    const auto cancel = service.cancelPreview();
    if (!cancel.ok) {
        return fail("cancelPreview must clear active preview: "
                    + cancel.diagnosticCode);
    }
    if (model.hasPreview() || model.committedGroupCount() != 0
        || model.isDirty()) {
        return fail("cancelPreview must leave model clean with no groups");
    }

    const auto secondPreview =
        service.previewDraft({"cmd-commit", makeDraft("group-commit")});
    if (!secondPreview.ok) {
        return fail("second previewDraft must accept valid draft");
    }

    const auto commit = service.commitPreview();
    if (!commit.ok) {
        return fail("commitPreview must commit active preview: "
                    + commit.diagnosticCode);
    }
    if (model.hasPreview()) {
        return fail("commitPreview must clear model preview");
    }
    if (model.committedGroupCount() != 1) {
        return fail("commitPreview must create committed rebar group");
    }
    if (!model.isDirty()) {
        return fail("commitPreview must mark RebarModel dirty");
    }

    return 0;
}

} // namespace

int main()
{
    if (const int code = expectServiceRequiresModel()) {
        return code;
    }
    if (const int code = expectPreviewCommitCancelUseModelTransaction()) {
        return code;
    }
    return 0;
}
