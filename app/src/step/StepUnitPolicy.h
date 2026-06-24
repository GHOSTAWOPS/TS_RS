#pragma once

#include <string>

namespace tsrs::step {

struct StepLengthUnitPolicy {
    std::string rawSourceLengthUnit;
    std::string normalizedSourceLengthUnit{"mm"};
    std::string internalLengthUnit{"m"};
    double sourceToMeterScale{0.001};
    bool sourceLengthUnitDetected{false};
    bool sourceLengthUnitKnown{false};
    bool sourceToMeterScaleAssumed{true};
    bool shapeCoordinatesScaledToInternalMeters{false};
};

[[nodiscard]] StepLengthUnitPolicy makeStepLengthUnitPolicy(
    const std::string& rawSourceLengthUnit);

} // namespace tsrs::step
