#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace tsrs::rebarsmart {

struct SpaceListParseOptions {
    double unitScaleToM{1.0};
    bool allowChineseComma{false};
};

struct SpaceListParseResult {
    bool ok{false};
    std::vector<double> valuesM;
    std::string diagnosticCode;
    int errorOffset{-1};
};

SpaceListParseResult parseSpaceList(std::string_view text, SpaceListParseOptions options);

} // namespace tsrs::rebarsmart
