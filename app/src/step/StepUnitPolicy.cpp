#include "step/StepUnitPolicy.h"

#include <algorithm>
#include <cctype>
#include <map>

namespace {

std::string trimAscii(std::string value)
{
    const auto isSpace = [](unsigned char ch) {
        return std::isspace(ch) != 0;
    };
    value.erase(value.begin(), std::find_if_not(value.begin(), value.end(), isSpace));
    value.erase(std::find_if_not(value.rbegin(), value.rend(), isSpace).base(), value.end());
    return value;
}

std::string lowerAscii(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

} // namespace

namespace tsrs::step {

StepLengthUnitPolicy makeStepLengthUnitPolicy(const std::string& rawSourceLengthUnit)
{
    StepLengthUnitPolicy policy;
    policy.rawSourceLengthUnit = rawSourceLengthUnit;

    const std::string normalized = lowerAscii(trimAscii(rawSourceLengthUnit));
    policy.sourceLengthUnitDetected = !normalized.empty();

    static const std::map<std::string, std::pair<std::string, double>> kKnownUnits = {
        {"millimeter", {"mm", 0.001}},
        {"millimetre", {"mm", 0.001}},
        {"millimeters", {"mm", 0.001}},
        {"millimetres", {"mm", 0.001}},
        {"mm", {"mm", 0.001}},
        {"centimeter", {"cm", 0.01}},
        {"centimetre", {"cm", 0.01}},
        {"centimeters", {"cm", 0.01}},
        {"centimetres", {"cm", 0.01}},
        {"cm", {"cm", 0.01}},
        {"meter", {"m", 1.0}},
        {"metre", {"m", 1.0}},
        {"meters", {"m", 1.0}},
        {"metres", {"m", 1.0}},
        {"m", {"m", 1.0}},
        {"inch", {"in", 0.0254}},
        {"inches", {"in", 0.0254}},
        {"in", {"in", 0.0254}},
        {"foot", {"ft", 0.3048}},
        {"feet", {"ft", 0.3048}},
        {"ft", {"ft", 0.3048}},
    };

    const auto found = kKnownUnits.find(normalized);
    if (found != kKnownUnits.end()) {
        policy.normalizedSourceLengthUnit = found->second.first;
        policy.sourceToMeterScale = found->second.second;
        policy.sourceLengthUnitKnown = true;
        policy.sourceToMeterScaleAssumed = false;
        return policy;
    }

    policy.normalizedSourceLengthUnit = policy.sourceLengthUnitDetected ? normalized : "mm";
    policy.sourceToMeterScale = 0.001;
    policy.sourceLengthUnitKnown = false;
    policy.sourceToMeterScaleAssumed = true;
    return policy;
}

} // namespace tsrs::step
