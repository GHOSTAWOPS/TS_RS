#pragma once

#include <TopoDS_Shape.hxx>

#include <string>

namespace tsrs::presentation {

struct StepDisplayModel {
    std::string sourcePath;
    int rootCount{0};
    int solidCount{0};
    int faceCount{0};
    int edgeCount{0};
    int vertexCount{0};
    TopoDS_Shape shape;
};

} // namespace tsrs::presentation
