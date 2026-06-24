#include "domain/rebar/RebarModel.h"
#include "drawing/schedule/ScheduleModel.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

namespace {

int fail(const std::string& message)
{
    std::cerr << message << '\n';
    return 1;
}

tsrs::domain::rebar::RebarGroupDraft makeGroup(
    std::string groupId,
    std::string commandId,
    std::string styleName,
    std::string grade,
    double diameterM,
    int bundleCount,
    std::vector<std::string> centerlineStableIds)
{
    tsrs::domain::rebar::RebarGroupDraft group;
    group.groupId = std::move(groupId);
    group.commandId = std::move(commandId);
    group.styleName = std::move(styleName);
    group.grade = std::move(grade);
    group.diameterM = diameterM;
    group.bundleCount = bundleCount;
    group.centerlineStableIds = std::move(centerlineStableIds);
    return group;
}

void commit(tsrs::domain::rebar::RebarModel& model,
            tsrs::domain::rebar::RebarGroupDraft group)
{
    const auto preview = model.replacePreview(group.commandId, group);
    if (!preview.ok) {
        std::cerr << "fixture preview failed: " << preview.diagnosticCode
                  << '\n';
        std::exit(2);
    }
    const auto commit = model.commitPreview();
    if (!commit.ok) {
        std::cerr << "fixture commit failed: " << commit.diagnosticCode
                  << '\n';
        std::exit(2);
    }
}

bool nearlyEqual(double lhs, double rhs)
{
    return std::fabs(lhs - rhs) < 1e-12;
}

int expectScheduleSummarizesCommittedGroups()
{
    tsrs::domain::rebar::RebarModel model;
    commit(model,
           makeGroup("group-a",
                     "fix-distance",
                     "D28",
                     "HRB400",
                     0.028,
                     2,
                     {"centerline-a", "centerline-b"}));
    commit(model,
           makeGroup("group-b",
                     "fix-number",
                     "D16",
                     "HRB500",
                     0.016,
                     1,
                     {"centerline-c"}));

    const auto schedule =
        tsrs::drawing::schedule::buildBasicSchedule(model);

    if (schedule.rows.size() != 2) {
        return fail("schedule must create one row per committed group");
    }
    if (schedule.totalGroupCount != 2) {
        return fail("schedule totalGroupCount must count committed groups");
    }
    if (schedule.totalBarCount != 5) {
        return fail("schedule totalBarCount must include bundle count");
    }

    const auto& first = schedule.rows[0];
    if (first.groupId != "group-a" || first.commandId != "fix-distance"
        || first.styleName != "D28" || first.grade != "HRB400") {
        return fail("first schedule row must preserve group identity fields");
    }
    if (!nearlyEqual(first.diameterM, 0.028)) {
        return fail("first schedule row must preserve diameterM");
    }
    if (first.bundleCount != 2 || first.centerlineCount != 2
        || first.barCount != 4) {
        return fail("first schedule row must count centerlines and bundles");
    }
    if (first.lengthKnown || !nearlyEqual(first.totalLengthM, 0.0)) {
        return fail("P0 schedule must not invent unknown centerline lengths");
    }

    const auto& second = schedule.rows[1];
    if (second.groupId != "group-b" || second.bundleCount != 1
        || second.centerlineCount != 1 || second.barCount != 1) {
        return fail("second schedule row did not summarize group-b");
    }
    if (schedule.hasKnownLengths) {
        return fail("P0 schedule must mark aggregate length as unknown");
    }
    if (!nearlyEqual(schedule.totalKnownLengthM, 0.0)) {
        return fail("unknown aggregate length must stay zero");
    }
    return 0;
}

int expectPreviewIsExcludedFromSchedule()
{
    tsrs::domain::rebar::RebarModel model;
    const auto preview =
        model.replacePreview("fix-distance",
                             makeGroup("preview",
                                       "fix-distance",
                                       "D20",
                                       "HRB400",
                                       0.02,
                                       1,
                                       {"preview-centerline"}));
    if (!preview.ok) {
        return fail("fixture preview must be accepted");
    }

    const auto schedule =
        tsrs::drawing::schedule::buildBasicSchedule(model);
    if (!schedule.rows.empty() || schedule.totalGroupCount != 0
        || schedule.totalBarCount != 0) {
        return fail("schedule must exclude uncommitted preview groups");
    }
    return 0;
}

} // namespace

int main()
{
    if (const int code = expectScheduleSummarizesCommittedGroups()) {
        return code;
    }
    if (const int code = expectPreviewIsExcludedFromSchedule()) {
        return code;
    }
    return 0;
}
