#include "step/ShapeStore.h"

#include <TopAbs_ShapeEnum.hxx>
#include <TopExp.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>

namespace {

std::vector<tsrs::step::IndexedShape> collectUniqueShapes(
    const TopoDS_Shape& root,
    TopAbs_ShapeEnum kind)
{
    TopTools_IndexedMapOfShape mapped;
    TopExp::MapShapes(root, kind, mapped);

    std::vector<tsrs::step::IndexedShape> shapes;
    shapes.reserve(static_cast<std::size_t>(mapped.Extent()));
    for (int index = 1; index <= mapped.Extent(); ++index) {
        shapes.push_back({index - 1, mapped.FindKey(index)});
    }
    return shapes;
}

} // namespace

namespace tsrs::step {

ShapeStore ShapeStore::fromImportedStep(const StepImportResult& imported)
{
    ShapeStore store;
    store.sourcePath_ = imported.sourcePath;
    store.rootShape_ = imported.rootShape;
    if (store.rootShape_.IsNull()) {
        return store;
    }

    store.faces_ = collectUniqueShapes(store.rootShape_, TopAbs_FACE);
    store.edges_ = collectUniqueShapes(store.rootShape_, TopAbs_EDGE);
    store.vertices_ = collectUniqueShapes(store.rootShape_, TopAbs_VERTEX);
    return store;
}

} // namespace tsrs::step
