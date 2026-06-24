#pragma once

#include "domain/rebar/RebarModel.h"

#include <cstddef>
#include <string>
#include <vector>

namespace tsrs::drawing::schedule {

struct ScheduleRow {
    std::string groupId;
    std::string commandId;
    std::string styleName;
    std::string grade;
    double diameterM{0.0};
    int bundleCount{1};
    std::size_t centerlineCount{0};
    std::size_t barCount{0};
    bool lengthKnown{false};
    double totalLengthM{0.0};
};

struct BasicSchedule {
    std::vector<ScheduleRow> rows;
    std::size_t totalGroupCount{0};
    std::size_t totalBarCount{0};
    bool hasKnownLengths{false};
    double totalKnownLengthM{0.0};
};

[[nodiscard]] BasicSchedule buildBasicSchedule(
    const tsrs::domain::rebar::RebarModel& model);

} // namespace tsrs::drawing::schedule
