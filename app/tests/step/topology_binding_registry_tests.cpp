#include "step/ShapeStore.h"
#include "step/StepImportService.h"
#include "step/TopologyBindingRegistry.h"

#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRep_Builder.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <gp_Pnt.hxx>
#include <STEPControl_Writer.hxx>
#include <TopExp_Explorer.hxx>
#include <TopAbs.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Edge.hxx>

#include <filesystem>
#include <iostream>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

namespace {

int fail(const std::string& message)
{
    std::cerr << message << '\n';
    return 1;
}

std::filesystem::path writeBoxStepFixture()
{
    const auto processId =
#ifdef _WIN32
        _getpid();
#else
        getpid();
#endif
    const std::filesystem::path path = std::filesystem::temp_directory_path()
        / ("tsrs_topology_binding_box_fixture_" + std::to_string(processId) + ".step");
    std::filesystem::remove(path);

    const TopoDS_Shape box = BRepPrimAPI_MakeBox(1.0, 2.0, 3.0).Shape();
    STEPControl_Writer writer;
    writer.Transfer(box, STEPControl_AsIs);
    writer.Write(path.string().c_str());
    return path;
}

std::filesystem::path writeShapeStepFixture(
    const TopoDS_Shape& shape,
    const std::string& stem)
{
    const auto processId =
#ifdef _WIN32
        _getpid();
#else
        getpid();
#endif
    const std::filesystem::path path = std::filesystem::temp_directory_path()
        / ("tsrs_topology_binding_" + stem + "_" + std::to_string(processId) + ".step");
    std::filesystem::remove(path);

    STEPControl_Writer writer;
    writer.Transfer(shape, STEPControl_AsIs);
    writer.Write(path.string().c_str());
    return path;
}

TopoDS_Shape makeCompoundStressShape()
{
    TopoDS_Compound compound;
    BRep_Builder builder;
    builder.MakeCompound(compound);
    builder.Add(compound, BRepPrimAPI_MakeBox(1.0, 1.0, 1.0).Shape());
    builder.Add(
        compound,
        BRepPrimAPI_MakeBox(gp_Pnt(1.5, 0.25, 0.0), 2.5, 1.25, 1.0).Shape());
    return compound;
}

TopoDS_Shape makeFusedLShapeStressShape()
{
    const TopoDS_Shape left = BRepPrimAPI_MakeBox(1.0, 1.0, 1.0).Shape();
    const TopoDS_Shape right =
        BRepPrimAPI_MakeBox(gp_Pnt(0.5, 0.5, 0.0), 1.5, 2.0, 1.0).Shape();
    return BRepAlgoAPI_Fuse(left, right).Shape();
}

tsrs::step::TopologyBindingRegistry importRegistry(const std::filesystem::path& fixture)
{
    const tsrs::step::StepImportService importService;
    const tsrs::step::StepImportResult imported = importService.importFile(fixture.string());
    if (!imported.ok) {
        throw std::runtime_error("STEP import failed: " + imported.diagnostic);
    }

    const tsrs::step::ShapeStore store =
        tsrs::step::ShapeStore::fromImportedStep(imported);
    return tsrs::step::TopologyBindingRegistry::build(store);
}

tsrs::step::TopologyBindingRegistry registryFromShape(const TopoDS_Shape& shape)
{
    tsrs::step::StepImportResult imported;
    imported.ok = true;
    imported.sourcePath = "synthetic-shape";
    imported.rootShape = shape;

    const tsrs::step::ShapeStore store =
        tsrs::step::ShapeStore::fromImportedStep(imported);
    return tsrs::step::TopologyBindingRegistry::build(store);
}

std::vector<std::string> stableIdsOf(
    const std::vector<tsrs::step::TopologyBinding>& bindings)
{
    std::vector<std::string> stableIds;
    stableIds.reserve(bindings.size());
    for (const tsrs::step::TopologyBinding& binding : bindings) {
        stableIds.push_back(binding.stableId);
    }
    return stableIds;
}

bool nearlyEqual(double lhs, double rhs, double tolerance = 1.0e-9)
{
    return std::fabs(lhs - rhs) <= tolerance;
}

bool bboxNearlyEqual(
    const tsrs::step::TopologyBbox& lhs,
    const tsrs::step::TopologyBbox& rhs,
    double tolerance = 1.0e-9)
{
    for (std::size_t i = 0; i < lhs.size(); ++i) {
        if (!nearlyEqual(lhs[i], rhs[i], tolerance)) {
            return false;
        }
    }
    return true;
}

tsrs::step::TopologyBinding makeSyntheticBinding(
    int localIndex,
    std::string stableId,
    std::string geometryFingerprint,
    tsrs::step::TopologyBbox bbox)
{
    tsrs::step::TopologyBinding binding;
    binding.kind = tsrs::step::kTopologyKindEdge;
    binding.localIndex = localIndex;
    binding.stableId = std::move(stableId);
    binding.geometryFingerprint = std::move(geometryFingerprint);
    binding.bbox = bbox;
    binding.measure = 10.0;
    return binding;
}

int expectStableFingerprintsAcrossRepeatedImports()
{
    const std::filesystem::path fixture = writeBoxStepFixture();
    const tsrs::step::TopologyBindingRegistry baseline = importRegistry(fixture);

    if (baseline.faces().size() != 6) {
        return fail("expected imported box to expose exactly 6 unique face bindings");
    }
    if (baseline.edges().size() != 12) {
        return fail("expected imported box to expose exactly 12 unique edge bindings");
    }
    if (baseline.vertices().size() != 8) {
        return fail("expected imported box to expose exactly 8 unique vertex bindings");
    }
    if (!baseline.ok()) {
        return fail("expected baseline registry without duplicate stable ids: "
                    + baseline.diagnostic());
    }

    const std::vector<std::string> faceIds = stableIdsOf(baseline.faces());
    const std::vector<std::string> edgeIds = stableIdsOf(baseline.edges());
    const std::vector<std::string> vertexIds = stableIdsOf(baseline.vertices());

    for (int run = 0; run < 5; ++run) {
        const tsrs::step::TopologyBindingRegistry current = importRegistry(fixture);
        if (!current.ok()) {
            return fail("expected repeated registry without duplicate stable ids");
        }
        if (stableIdsOf(current.faces()) != faceIds) {
            return fail("face stable id list changed across repeated import");
        }
        if (stableIdsOf(current.edges()) != edgeIds) {
            return fail("edge stable id list changed across repeated import");
        }
        if (stableIdsOf(current.vertices()) != vertexIds) {
            return fail("vertex stable id list changed across repeated import");
        }
    }

    return 0;
}

int expectEdgeEndpointOrderIsCanonicalized()
{
    const TopoDS_Shape forwardEdge =
        BRepBuilderAPI_MakeEdge(gp_Pnt(0.0, 0.0, 0.0), gp_Pnt(3.0, 2.0, 1.0)).Shape();
    const TopoDS_Shape reversedEdge =
        BRepBuilderAPI_MakeEdge(gp_Pnt(3.0, 2.0, 1.0), gp_Pnt(0.0, 0.0, 0.0)).Shape();

    const tsrs::step::TopologyBindingRegistry forward = registryFromShape(forwardEdge);
    const tsrs::step::TopologyBindingRegistry reversed = registryFromShape(reversedEdge);
    if (forward.edges().size() != 1 || reversed.edges().size() != 1) {
        return fail("expected one synthetic edge binding in each endpoint-order probe");
    }

    if (forward.edges().front().geometryFingerprint
        != reversed.edges().front().geometryFingerprint) {
        return fail("expected reversed edge endpoint order to keep geometry fingerprint stable");
    }
    if (forward.edges().front().stableId != reversed.edges().front().stableId) {
        return fail("expected reversed edge endpoint order to keep stable id stable");
    }
    return 0;
}

int expectSerializeAndRestoreBinding()
{
    const std::filesystem::path fixture = writeBoxStepFixture();
    const tsrs::step::TopologyBindingRegistry baseline = importRegistry(fixture);
    const tsrs::step::TopologyBindingReference saved =
        tsrs::step::makeBindingReference("selectedGuideEdge", baseline.edges().at(3));
    const std::string json = tsrs::step::serializeBindingReference(saved);
    const tsrs::step::TopologyBindingReference restoredReference =
        tsrs::step::deserializeBindingReference(json);

    const tsrs::step::TopologyBindingRegistry reloaded = importRegistry(fixture);
    const tsrs::step::TopologyBindingLookupResult restored =
        reloaded.restore(restoredReference);

    if (!restored.ok) {
        return fail("expected restored binding, got " + restored.diagnosticCode + ": "
                    + restored.diagnostic);
    }
    if (restored.binding.stableId != saved.stableId) {
        return fail("restored binding stable id mismatch");
    }
    if (restored.binding.kind != tsrs::step::kTopologyKindEdge) {
        return fail("restored binding kind mismatch");
    }
    if (restored.binding.geometryFingerprint != saved.geometryFingerprint) {
        return fail("restored binding geometry fingerprint mismatch");
    }
    if (!bboxNearlyEqual(restored.binding.bbox, saved.fallbackBbox)) {
        return fail("restored binding bbox mismatch");
    }
    if (!nearlyEqual(restored.binding.measure, baseline.edges().at(3).measure)) {
        return fail("restored binding measure mismatch");
    }
    return 0;
}

int expectRestoreFallsBackWhenStableIdDrifts()
{
    const tsrs::step::TopologyBbox bbox{0.0, 0.0, 0.0, 10.0, 0.0, 0.0};
    const tsrs::step::TopologyBinding saved =
        makeSyntheticBinding(4, "tsrs-topology-v1:edge:old", "edge|same", bbox);
    const tsrs::step::TopologyBinding changed =
        makeSyntheticBinding(4, "tsrs-topology-v1:edge:new", "edge|same", bbox);

    const tsrs::step::TopologyBindingRegistry registry =
        tsrs::step::TopologyBindingRegistry::fromBindings({changed});
    const tsrs::step::TopologyBindingReference reference =
        tsrs::step::makeBindingReference("selectedGuideEdge", saved);
    const tsrs::step::TopologyBindingLookupResult result = registry.restore(reference);

    if (!result.ok || result.diagnosticCode != tsrs::step::kTopologyDiagnosticOk) {
        return fail("expected fallback restore after stable id drift, got "
                    + result.diagnosticCode + ": " + result.diagnostic);
    }
    if (!result.usedFallback) {
        return fail("expected restore result to report fallback usage");
    }
    if (result.binding.stableId != changed.stableId) {
        return fail("expected fallback restore to return the changed stable id binding");
    }
    return 0;
}

int expectRestoreRejectsGeometryFingerprintDriftWithOnlyLocalIndexBbox()
{
    const tsrs::step::TopologyBbox savedBbox{0.0, 0.0, 0.0, 10.0, 0.0, 0.0};
    const tsrs::step::TopologyBbox changedBbox{0.0, 0.0, 0.0, 10.0000005, 0.0, 0.0};
    const tsrs::step::TopologyBinding saved =
        makeSyntheticBinding(7, "tsrs-topology-v1:edge:old", "edge|old", savedBbox);
    const tsrs::step::TopologyBinding changed =
        makeSyntheticBinding(7, "tsrs-topology-v1:edge:new", "edge|new", changedBbox);

    const tsrs::step::TopologyBindingRegistry registry =
        tsrs::step::TopologyBindingRegistry::fromBindings({changed});
    const tsrs::step::TopologyBindingReference reference =
        tsrs::step::makeBindingReference("selectedGuideEdge", saved);
    const tsrs::step::TopologyBindingLookupResult result = registry.restore(reference);

    if (result.ok) {
        return fail("expected geometry fingerprint drift with only localIndex+bbox to fail");
    }
    if (result.diagnosticCode != tsrs::step::kTopologyDiagnosticBindingLowConfidence) {
        return fail("expected low confidence diagnostic for geometry fingerprint drift, got "
                    + result.diagnosticCode + ": " + result.diagnostic);
    }
    return 0;
}

int expectLocalIndexBboxOnlyFallbackIsLowConfidence()
{
    const tsrs::step::TopologyBbox savedBbox{0.0, 0.0, 0.0, 10.0, 0.0, 0.0};
    const tsrs::step::TopologyBbox changedBbox{0.0, 0.0, 0.0, 10.0000005, 0.0, 0.0};
    const tsrs::step::TopologyBinding saved =
        makeSyntheticBinding(7, "tsrs-topology-v1:edge:old", "edge|old", savedBbox);
    const tsrs::step::TopologyBinding changed =
        makeSyntheticBinding(7, "tsrs-topology-v1:edge:new", "edge|new", changedBbox);

    const tsrs::step::TopologyBindingRegistry registry =
        tsrs::step::TopologyBindingRegistry::fromBindings({changed});
    const tsrs::step::TopologyBindingReference reference =
        tsrs::step::makeBindingReference("selectedGuideEdge", saved);
    const tsrs::step::TopologyBindingLookupResult result = registry.restore(reference);

    if (result.ok) {
        return fail("expected localIndex+bbox-only fallback to be rejected as low confidence");
    }
    if (result.diagnosticCode != tsrs::step::kTopologyDiagnosticBindingLowConfidence) {
        return fail("expected low confidence diagnostic for localIndex+bbox-only fallback, got "
                    + result.diagnosticCode + ": " + result.diagnostic);
    }
    if (!result.usedFallback || result.candidateCount != 1) {
        return fail("expected low confidence result to report one fallback candidate");
    }
    return 0;
}

int expectLocalIndexBboxOnlyFallbackRejectsZeroBbox()
{
    const tsrs::step::TopologyBbox zeroBbox{0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    const tsrs::step::TopologyBinding saved =
        makeSyntheticBinding(0, "tsrs-topology-v1:edge:old", "edge|old", zeroBbox);
    const tsrs::step::TopologyBinding changed =
        makeSyntheticBinding(0, "tsrs-topology-v1:edge:new", "edge|new", zeroBbox);

    const tsrs::step::TopologyBindingRegistry registry =
        tsrs::step::TopologyBindingRegistry::fromBindings({changed});
    const tsrs::step::TopologyBindingReference reference =
        tsrs::step::makeBindingReference("selectedGuideEdge", saved);
    const tsrs::step::TopologyBindingLookupResult result = registry.restore(reference);

    if (result.ok) {
        return fail("expected localIndex+bbox-only zero bbox fallback to be rejected");
    }
    if (result.diagnosticCode != tsrs::step::kTopologyDiagnosticBindingLowConfidence) {
        return fail("expected low confidence diagnostic for zero bbox fallback, got "
                    + result.diagnosticCode + ": " + result.diagnostic);
    }
    return 0;
}

int expectRealisticStepTopologyStressRepeatedImportIsStable(
    const TopoDS_Shape& shape,
    const std::string& stem)
{
    const std::filesystem::path fixture = writeShapeStepFixture(shape, stem);
    const tsrs::step::TopologyBindingRegistry baseline = importRegistry(fixture);
    if (!baseline.ok()) {
        return fail("expected " + stem + " stress registry without duplicate stable ids: "
                    + baseline.diagnostic());
    }
    if (baseline.edges().empty() || baseline.faces().empty()) {
        return fail("expected " + stem + " stress STEP to expose face and edge bindings");
    }

    const std::vector<std::string> faceIds = stableIdsOf(baseline.faces());
    const std::vector<std::string> edgeIds = stableIdsOf(baseline.edges());
    const std::vector<std::string> vertexIds = stableIdsOf(baseline.vertices());
    const tsrs::step::TopologyBindingReference saved =
        tsrs::step::makeBindingReference("stressSelectedEdge", baseline.edges().front());

    for (int run = 0; run < 3; ++run) {
        const tsrs::step::TopologyBindingRegistry current = importRegistry(fixture);
        if (!current.ok()) {
            return fail("expected repeated " + stem
                        + " stress registry without duplicate stable ids");
        }
        if (stableIdsOf(current.faces()) != faceIds) {
            return fail(stem + " face stable id list changed across repeated import");
        }
        if (stableIdsOf(current.edges()) != edgeIds) {
            return fail(stem + " edge stable id list changed across repeated import");
        }
        if (stableIdsOf(current.vertices()) != vertexIds) {
            return fail(stem + " vertex stable id list changed across repeated import");
        }

        const tsrs::step::TopologyBindingLookupResult restored = current.restore(saved);
        if (!restored.ok || restored.binding.stableId != saved.stableId) {
            return fail(stem + " restore determinism failed");
        }
    }
    return 0;
}

int expectMissingAndKindMismatchDiagnostics()
{
    const std::filesystem::path fixture = writeBoxStepFixture();
    const tsrs::step::TopologyBindingRegistry registry = importRegistry(fixture);

    tsrs::step::TopologyBindingReference missing =
        tsrs::step::makeBindingReference("selectedGuideEdge", registry.edges().front());
    missing.stableId = "tsrs-topology-v1:edge:missing";
    missing.geometryFingerprint = "edge|missing";
    missing.fallbackLocalIndex = -1;
    missing.fallbackBbox = {999.0, 999.0, 999.0, 1000.0, 1000.0, 1000.0};
    const tsrs::step::TopologyBindingLookupResult missingResult = registry.restore(missing);
    if (missingResult.ok
        || missingResult.diagnosticCode != tsrs::step::kTopologyDiagnosticBindingMissing) {
        return fail("expected missing binding diagnostic");
    }

    tsrs::step::TopologyBindingReference wrongKind =
        tsrs::step::makeBindingReference("selectedFace", registry.edges().front());
    wrongKind.kind = tsrs::step::kTopologyKindFace;
    const tsrs::step::TopologyBindingLookupResult wrongKindResult =
        registry.restore(wrongKind);
    if (wrongKindResult.ok
        || wrongKindResult.diagnosticCode != tsrs::step::kTopologyDiagnosticKindMismatch) {
        return fail("expected kind mismatch diagnostic");
    }

    return 0;
}

int expectFallbackAmbiguousDiagnosticForDuplicateFallbackCandidates()
{
    const tsrs::step::TopologyBbox bbox{0.0, 0.0, 0.0, 10.0, 0.0, 0.0};
    const tsrs::step::TopologyBinding saved =
        makeSyntheticBinding(2, "tsrs-topology-v1:edge:old", "edge|same", bbox);
    const tsrs::step::TopologyBinding candidateA =
        makeSyntheticBinding(2, "tsrs-topology-v1:edge:new-a", "edge|same", bbox);
    const tsrs::step::TopologyBinding candidateB =
        makeSyntheticBinding(2, "tsrs-topology-v1:edge:new-b", "edge|same", bbox);

    const tsrs::step::TopologyBindingRegistry registry =
        tsrs::step::TopologyBindingRegistry::fromBindings({candidateA, candidateB});
    const tsrs::step::TopologyBindingReference reference =
        tsrs::step::makeBindingReference("selectedGuideEdge", saved);
    const tsrs::step::TopologyBindingLookupResult result = registry.restore(reference);

    if (result.ok
        || result.diagnosticCode != tsrs::step::kTopologyDiagnosticBindingAmbiguous
        || result.candidateCount != 2) {
        return fail("expected ambiguous diagnostic for duplicate fallback candidates");
    }
    return 0;
}

int expectGeometryFingerprintMultiCandidateStaysAmbiguous()
{
    const tsrs::step::TopologyBbox savedBbox{0.0, 0.0, 0.0, 10.0, 0.0, 0.0};
    const tsrs::step::TopologyBbox otherBbox{5.0, 0.0, 0.0, 15.0, 0.0, 0.0};
    const tsrs::step::TopologyBinding saved =
        makeSyntheticBinding(2, "tsrs-topology-v1:edge:old", "edge|same", savedBbox);
    const tsrs::step::TopologyBinding candidateA =
        makeSyntheticBinding(2, "tsrs-topology-v1:edge:new-a", "edge|same", savedBbox);
    const tsrs::step::TopologyBinding candidateB =
        makeSyntheticBinding(9, "tsrs-topology-v1:edge:new-b", "edge|same", otherBbox);

    const tsrs::step::TopologyBindingRegistry registry =
        tsrs::step::TopologyBindingRegistry::fromBindings({candidateA, candidateB});
    const tsrs::step::TopologyBindingReference reference =
        tsrs::step::makeBindingReference("selectedGuideEdge", saved);
    const tsrs::step::TopologyBindingLookupResult result = registry.restore(reference);

    if (result.ok
        || result.diagnosticCode != tsrs::step::kTopologyDiagnosticBindingAmbiguous
        || result.candidateCount != 2) {
        return fail("expected geometry fingerprint multi-candidate match to stay ambiguous");
    }
    return 0;
}

int expectAmbiguousDiagnosticForDuplicateStableIds()
{
    tsrs::step::TopologyBinding duplicateA;
    duplicateA.kind = tsrs::step::kTopologyKindEdge;
    duplicateA.localIndex = 1;
    duplicateA.stableId = "tsrs-topology-v1:edge:duplicate";
    duplicateA.geometryFingerprint = "duplicate";

    tsrs::step::TopologyBinding duplicateB = duplicateA;
    duplicateB.localIndex = 2;

    const tsrs::step::TopologyBindingRegistry registry =
        tsrs::step::TopologyBindingRegistry::fromBindings({duplicateA, duplicateB});
    const tsrs::step::TopologyBindingReference reference =
        tsrs::step::makeBindingReference("selectedGuideEdge", duplicateA);
    const tsrs::step::TopologyBindingLookupResult result = registry.restore(reference);

    if (result.ok
        || result.diagnosticCode != tsrs::step::kTopologyDiagnosticBindingAmbiguous) {
        return fail("expected ambiguous binding diagnostic");
    }

    return 0;
}

} // namespace

int main()
{
    try {
        if (const int code = expectStableFingerprintsAcrossRepeatedImports()) {
            return code;
        }
        if (const int code = expectEdgeEndpointOrderIsCanonicalized()) {
            return code;
        }
        if (const int code = expectSerializeAndRestoreBinding()) {
            return code;
        }
        if (const int code = expectRestoreFallsBackWhenStableIdDrifts()) {
            return code;
        }
        if (const int code = expectRestoreRejectsGeometryFingerprintDriftWithOnlyLocalIndexBbox()) {
            return code;
        }
        if (const int code = expectLocalIndexBboxOnlyFallbackIsLowConfidence()) {
            return code;
        }
        if (const int code = expectLocalIndexBboxOnlyFallbackRejectsZeroBbox()) {
            return code;
        }
        if (const int code = expectRealisticStepTopologyStressRepeatedImportIsStable(
                makeCompoundStressShape(), "compound")) {
            return code;
        }
        if (const int code = expectRealisticStepTopologyStressRepeatedImportIsStable(
                makeFusedLShapeStressShape(), "fused_lshape")) {
            return code;
        }
        if (const int code = expectMissingAndKindMismatchDiagnostics()) {
            return code;
        }
        if (const int code = expectFallbackAmbiguousDiagnosticForDuplicateFallbackCandidates()) {
            return code;
        }
        if (const int code = expectGeometryFingerprintMultiCandidateStaysAmbiguous()) {
            return code;
        }
        if (const int code = expectAmbiguousDiagnosticForDuplicateStableIds()) {
            return code;
        }
    } catch (const std::exception& exception) {
        return fail(std::string{"unexpected exception: "} + exception.what());
    }
    return 0;
}
