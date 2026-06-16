#pragma once

#include "rebarsmart/distribute/DistributionTypes.h"

namespace tsrs::rebarsmart {

DistributionResult distributeBySpacing(double curveLengthM,
                                      ZoneLengths zone,
                                      double spacingM,
                                      double spacingRatio);

} // namespace tsrs::rebarsmart
