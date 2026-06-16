#pragma once

#include "rebarsmart/distribute/DistributionTypes.h"

namespace tsrs::rebarsmart {

DistributionResult distributeByCount(double curveLengthM, ZoneLengths zone, int count);

} // namespace tsrs::rebarsmart
