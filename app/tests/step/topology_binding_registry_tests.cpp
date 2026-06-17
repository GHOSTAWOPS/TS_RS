#include "step/ShapeStore.h"
#include "step/StepImportService.h"
#include "step/TopologyBindingRegistry.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <STEPControl_Writer.hxx>
#include <TopoDS_Shape.hxx>

#include <filesystem>
#include <iostream>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

int fail(const std::string& message)
{
    std::cerr << message << '\n';
    return 1;
}

std::filesystem::path writeBoxStepFixture()
{
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "tsrs_topology_binding_box_fixture.step";
    std::filesystem::remove(path);

    const TopoDS_Shape box = BRepPrimAPI_MakeBox(1.0, 2.0, 3.0).Shape();
    STEPControl_Writer writer;
    writer.Transfer(box, STEPControl_AsIs);
    writer.Write(path.string().c_str());
    return path;
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

int expectMissingAndKindMismatchDiagnostics()
{
    const std::filesystem::path fixture = writeBoxStepFixture();
    const tsrs::step::TopologyBindingRegistry registry = importRegistry(fixture);

    tsrs::step::TopologyBindingReference missing =
        tsrs::step::makeBindingReference("selectedGuideEdge", registry.edges().front());
    missing.stableId = "tsrs-topology-v1:edge:missing";
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
        if (const int code = expectSerializeAndRestoreBinding()) {
            return code;
        }
        if (const int code = expectMissingAndKindMismatchDiagnostics()) {
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
