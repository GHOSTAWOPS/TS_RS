#include "drawing/schedule/ScheduleModel.h"

#include <utility>

namespace tsrs::drawing::schedule {

namespace {

std::size_t safeBundleCount(int bundleCount)
{
    return bundleCount > 0 ? static_cast<std::size_t>(bundleCount) : 0U;
}

ScheduleRow toScheduleRow(const tsrs::domain::rebar::RebarGroupDraft& group)
{
    ScheduleRow row;
    row.groupId = group.groupId;
    row.commandId = group.commandId;
    row.styleName = group.styleName;
    row.grade = group.grade;
    row.diameterM = group.diameterM;
    row.bundleCount = group.bundleCount;
    row.centerlineCount = group.centerlineStableIds.size();
    row.barCount = row.centerlineCount * safeBundleCount(group.bundleCount);
    row.lengthKnown = false;
    row.totalLengthM = 0.0;
    return row;
}

} // namespace

BasicSchedule buildBasicSchedule(
    const tsrs::domain::rebar::RebarModel& model)
{
    BasicSchedule schedule;
    const auto& groups = model.committedGroups();
    schedule.rows.reserve(groups.size());

    for (const auto& group : groups) {
        ScheduleRow row = toScheduleRow(group);
        schedule.totalBarCount += row.barCount;
        schedule.rows.push_back(std::move(row));
    }

    schedule.totalGroupCount = schedule.rows.size();
    schedule.hasKnownLengths = false;
    schedule.totalKnownLengthM = 0.0;
    return schedule;
}

} // namespace tsrs::drawing::schedule
