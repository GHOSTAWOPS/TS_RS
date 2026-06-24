#include "domain/rebar/RebarModel.h"

#include <iostream>
#include <string>

namespace {

int fail(const std::string& message)
{
    std::cerr << message << '\n';
    return 1;
}

tsrs::domain::rebar::RebarGroupDraft makeDraft(std::string id)
{
    tsrs::domain::rebar::RebarGroupDraft draft;
    draft.groupId = std::move(id);
    draft.commandId = "fix-distance-preview";
    draft.styleName = "D28";
    draft.grade = "HRB400";
    draft.diameterM = 0.028;
    draft.bundleCount = 1;
    draft.centerlineStableIds = {"curve-1", "curve-2"};
    return draft;
}

int expectPreviewCommitMarksDirty()
{
    tsrs::domain::rebar::RebarModel model;

    const auto preview =
        model.replacePreview("cmd-001", makeDraft("group-preview"));
    if (!preview.ok) {
        return fail("replacePreview must accept a valid draft: "
                    + preview.diagnosticCode);
    }
    if (!model.hasPreview()) {
        return fail("model must expose active preview after replacePreview");
    }
    if (model.committedGroupCount() != 0) {
        return fail("preview must not create committed groups");
    }
    if (model.isDirty()) {
        return fail("preview alone must not mark project dirty");
    }

    const auto commit = model.commitPreview();
    if (!commit.ok) {
        return fail("commitPreview must accept active preview: "
                    + commit.diagnosticCode);
    }
    if (model.hasPreview()) {
        return fail("commitPreview must clear active preview");
    }
    if (model.committedGroupCount() != 1) {
        return fail("commitPreview must create one committed group");
    }
    if (!model.isDirty()) {
        return fail("commitPreview must mark model dirty");
    }
    if (model.committedGroups().front().groupId != "group-preview") {
        return fail("commitPreview must preserve draft group id");
    }

    return 0;
}

int expectCancelClearsPreviewWithoutDirty()
{
    tsrs::domain::rebar::RebarModel model;
    const auto preview =
        model.replacePreview("cmd-002", makeDraft("group-cancel"));
    if (!preview.ok) {
        return fail("replacePreview must accept cancel draft");
    }

    const auto cancel = model.cancelPreview();
    if (!cancel.ok) {
        return fail("cancelPreview must accept active preview: "
                    + cancel.diagnosticCode);
    }
    if (model.hasPreview()) {
        return fail("cancelPreview must clear active preview");
    }
    if (model.committedGroupCount() != 0) {
        return fail("cancelPreview must not commit groups");
    }
    if (model.isDirty()) {
        return fail("cancelPreview must not mark model dirty");
    }

    return 0;
}

int expectInvalidTransitionsReturnDiagnostics()
{
    tsrs::domain::rebar::RebarModel model;

    const auto commit = model.commitPreview();
    if (commit.ok || commit.diagnosticCode
                         != tsrs::domain::rebar::kRebarModelDiagnosticNoPreview) {
        return fail("commitPreview without preview must return NO_PREVIEW");
    }

    const auto cancel = model.cancelPreview();
    if (cancel.ok || cancel.diagnosticCode
                         != tsrs::domain::rebar::kRebarModelDiagnosticNoPreview) {
        return fail("cancelPreview without preview must return NO_PREVIEW");
    }

    auto invalidDraft = makeDraft("");
    const auto preview = model.replacePreview("cmd-003", invalidDraft);
    if (preview.ok
        || preview.diagnosticCode
               != tsrs::domain::rebar::kRebarModelDiagnosticInvalidDraft) {
        return fail("replacePreview must reject empty group id");
    }
    if (model.hasPreview()) {
        return fail("invalid draft must not create preview state");
    }

    return 0;
}

} // namespace

int main()
{
    if (const int code = expectPreviewCommitMarksDirty()) {
        return code;
    }
    if (const int code = expectCancelClearsPreviewWithoutDirty()) {
        return code;
    }
    if (const int code = expectInvalidTransitionsReturnDiagnostics()) {
        return code;
    }
    return 0;
}
