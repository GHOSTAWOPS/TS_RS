#pragma once

#include <string>

namespace tsrs::presentation {

struct StepModelDisplayStyle {
    std::string semanticName;
    double red{0.0};
    double green{0.0};
    double blue{0.0};
    bool useSourceStepColor{false};
};

[[nodiscard]] StepModelDisplayStyle defaultStepModelDisplayStyle();

} // namespace tsrs::presentation
