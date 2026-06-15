#include "core/AppBuildInfo.h"

#include <iostream>
#include <string_view>

int main()
{
    if (tsrs::core::appName() != std::string_view{"TS_RS"}) {
        std::cerr << "unexpected app name\n";
        return 1;
    }

    return 0;
}
