#include "presentation/style/StepModelDisplayStyle.h"

#include <cmath>
#include <iostream>
#include <string>

namespace {

bool nearlyEqual(double lhs, double rhs)
{
    return std::fabs(lhs - rhs) < 1.0e-12;
}

int fail(const std::string& message)
{
    std::cerr << message << '\n';
    return 1;
}

} // namespace

int main()
{
    const tsrs::presentation::StepModelDisplayStyle style =
        tsrs::presentation::defaultStepModelDisplayStyle();

    if (style.semanticName != "cement-gray") {
        return fail("STEP model display style must use cement-gray semantic");
    }
    if (!nearlyEqual(style.red, 0.56) || !nearlyEqual(style.green, 0.58)
        || !nearlyEqual(style.blue, 0.59)) {
        return fail("STEP model display color must be fixed cement gray");
    }
    if (style.useSourceStepColor) {
        return fail("STEP source color must be ignored in P0");
    }
    return 0;
}
