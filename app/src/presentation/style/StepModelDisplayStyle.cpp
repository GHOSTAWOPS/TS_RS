#include "presentation/style/StepModelDisplayStyle.h"

namespace tsrs::presentation {

StepModelDisplayStyle defaultStepModelDisplayStyle()
{
    return StepModelDisplayStyle{
        "cement-gray",
        0.56,
        0.58,
        0.59,
        false,
    };
}

} // namespace tsrs::presentation
