#pragma once

#include "application/commands/RebarCreationCommandService.h"
#include "application/rebarsmart/RebarSmartFixDistanceCenterlineGeneratorAdapter.h"

namespace tsrs::application {

class RebarCreationCommandServiceFactory final {
public:
    [[nodiscard]] static RebarCreationCommandService create(
        tsrs::domain::rebar::RebarModel* model);

private:
    static const RebarSmartFixDistanceCenterlineGeneratorAdapter&
        fixDistanceGenerator();
};

} // namespace tsrs::application
