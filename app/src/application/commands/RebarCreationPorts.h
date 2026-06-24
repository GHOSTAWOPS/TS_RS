#pragma once

#include "geometry/kernel/IGeometryEngine.h"

#include <string>
#include <vector>

namespace tsrs::application {

enum class RebarCreationFixDistancePriorityMode {
    Spacing = 0,
    Count = 1,
    SpacingList = 2,
};

struct RebarCreationFixDistanceInput {
    std::string styleName;
    std::string grade;
    double diameterM{0.0};
    double spacingM{0.0};
    int count{0};
    int bundleCount{1};
    RebarCreationFixDistancePriorityMode priorityMode{
        RebarCreationFixDistancePriorityMode::Spacing};
    double spacingRatio{1.0};
    double headZoneLengthM{0.0};
    double tailZoneLengthM{0.0};
    std::string spacingListText;
    tsrs::geometry::GeometryRef mainGuideCurve;
    tsrs::geometry::GeometryRef auxiliaryGuideCurve;
};

struct RebarCreationGeneratedCenterline {
    tsrs::geometry::GeometryRef curveRef;
};

struct RebarCreationGenerationResult {
    bool ok{false};
    std::vector<RebarCreationGeneratedCenterline> centerlines;
    std::string diagnosticCode;
};

class IFixDistanceCenterlineGenerator {
public:
    virtual ~IFixDistanceCenterlineGenerator() = default;

    [[nodiscard]] virtual RebarCreationGenerationResult generate(
        const RebarCreationFixDistanceInput& input,
        tsrs::geometry::IGeometryEngine& geometry) const = 0;
};

} // namespace tsrs::application
