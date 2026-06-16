#pragma once

#include "rebarsmart/distribute/DistributionTypes.h"

#include <vector>

namespace tsrs::rebarsmart {

DistributionResult distributeBySpacingList(double curveLengthM,
                                           ZoneLengths zone,
                                           std::vector<double> spacingListM);

} // namespace tsrs::rebarsmart
