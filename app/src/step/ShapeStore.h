#pragma once

#include "step/StepImportService.h"

#include <TopoDS_Shape.hxx>

#include <string>
#include <vector>

namespace tsrs::step {

struct IndexedShape {
    int localIndex{0};
    TopoDS_Shape shape;
};

class ShapeStore final {
public:
    [[nodiscard]] static ShapeStore fromImportedStep(const StepImportResult& imported);

    [[nodiscard]] const std::string& sourcePath() const { return sourcePath_; }
    [[nodiscard]] const TopoDS_Shape& rootShape() const { return rootShape_; }
    [[nodiscard]] const std::vector<IndexedShape>& faces() const { return faces_; }
    [[nodiscard]] const std::vector<IndexedShape>& edges() const { return edges_; }
    [[nodiscard]] const std::vector<IndexedShape>& vertices() const { return vertices_; }

private:
    std::string sourcePath_;
    TopoDS_Shape rootShape_;
    std::vector<IndexedShape> faces_;
    std::vector<IndexedShape> edges_;
    std::vector<IndexedShape> vertices_;
};

} // namespace tsrs::step
