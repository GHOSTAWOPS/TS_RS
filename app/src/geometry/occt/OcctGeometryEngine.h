#pragma once

#include "geometry/kernel/IGeometryEngine.h"

#include <TopoDS_Edge.hxx>

#include <map>
#include <string>
#include <vector>

namespace tsrs::geometry::occt {

class OcctGeometryEngine final : public IGeometryEngine {
public:
    GeometryRef registerEdge(std::string stableId, const TopoDS_Edge& edge);

    GeometryLengthResult curveLength(const GeometryRef& curveRef) const override;
    GeometryPointResult pointAtLength(const GeometryRef& curveRef, double lengthM) const override;
    GeometryVectorResult tangentAtLength(const GeometryRef& curveRef,
                                         double lengthM) const override;
    GeometryRefResult makePolylineCurve(std::vector<GeometryPoint3d> points) override;

private:
    struct Segment {
        TopoDS_Edge edge;
        double lengthM{0.0};
    };

    struct CurveRecord {
        std::vector<Segment> segments;
        double totalLengthM{0.0};
    };

    GeometryRef makeCurveRef(const std::string& stableId) const;
    std::string nextGeneratedStableId();

    std::map<std::string, CurveRecord> curves_;
    int generatedCurveOrdinal_{0};
};

} // namespace tsrs::geometry::occt
