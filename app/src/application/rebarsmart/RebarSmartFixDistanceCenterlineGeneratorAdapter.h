#pragma once

#include "application/commands/RebarCreationPorts.h"

namespace tsrs::application {

class RebarSmartFixDistanceCenterlineGeneratorAdapter final
    : public IFixDistanceCenterlineGenerator {
public:
    [[nodiscard]] RebarCreationGenerationResult generate(
        const RebarCreationFixDistanceInput& input,
        tsrs::geometry::IGeometryEngine& geometry) const override;
};

} // namespace tsrs::application
