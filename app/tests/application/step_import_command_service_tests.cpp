#include "application/commands/StepImportCommandService.h"
#include "application/import/ImportedModelStore.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <STEPControl_Writer.hxx>
#include <TopoDS_Shape.hxx>

#include <filesystem>
#include <iostream>
#include <string>

namespace {

int fail(const std::string& message)
{
    std::cerr << message << '\n';
    return 1;
}

std::filesystem::path writeBoxStepFixture()
{
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "tsrs_step_import_command_box_fixture.step";
    std::filesystem::remove(path);

    const TopoDS_Shape box = BRepPrimAPI_MakeBox(1.0, 2.0, 3.0).Shape();
    STEPControl_Writer writer;
    writer.Transfer(box, STEPControl_AsIs);
    writer.Write(path.string().c_str());
    return path;
}

int expectMissingFileDiagnostic()
{
    const tsrs::application::StepImportCommandService service;
    const tsrs::application::StepImportCommandResult result =
        service.importStep({std::string{"C:/definitely/missing/tsrs_command_missing.step"}});

    if (result.ok) {
        return fail("missing STEP file command must fail");
    }
    if (result.diagnosticCode != tsrs::application::kStepImportCommandDiagnosticMissingFile) {
        return fail("missing STEP command diagnostic mismatch: " + result.diagnosticCode);
    }
    if (!result.displayModel.shape.IsNull()) {
        return fail("missing STEP command must not expose a display shape");
    }
    return 0;
}

int expectSuccessfulDisplayModel()
{
    const std::filesystem::path fixture = writeBoxStepFixture();
    const tsrs::application::StepImportCommandService service;
    const tsrs::application::StepImportCommandResult result =
        service.importStep({fixture.string()});

    if (!result.ok) {
        return fail("expected STEP import command ok, got " + result.diagnostic);
    }
    if (result.diagnosticCode != tsrs::application::kStepImportCommandDiagnosticOk) {
        return fail("successful STEP command diagnostic mismatch: " + result.diagnosticCode);
    }
    if (result.displayModel.shape.IsNull()) {
        return fail("successful STEP command must expose display shape");
    }
    if (result.displayModel.faceCount < 6 || result.displayModel.edgeCount < 12
        || result.displayModel.vertexCount < 8) {
        return fail("successful STEP command must expose topology counts");
    }
    if (result.displayModel.sourcePath != fixture.string()) {
        return fail("successful STEP command must preserve source path");
    }
    if (!result.sessionId.empty()) {
        return fail("successful STEP command without store must not expose a session id");
    }
    return 0;
}

int expectImportCreatesStoredSessionWithTopologyBindings()
{
    const std::filesystem::path fixture = writeBoxStepFixture();
    tsrs::application::ImportedModelStore store;
    const tsrs::application::StepImportCommandService service(&store);
    const tsrs::application::StepImportCommandResult result =
        service.importStep({fixture.string()});

    if (!result.ok) {
        return fail("expected STEP import command ok, got " + result.diagnostic);
    }
    if (result.sessionId.empty()) {
        return fail("successful STEP command must expose a session id");
    }
    if (store.size() != 1) {
        return fail("imported model store must contain one session");
    }

    const tsrs::application::StepSession* session =
        store.findSession(result.sessionId);
    if (session == nullptr) {
        return fail("imported model store must find the imported session");
    }
    if (session->sourcePath != fixture.string()) {
        return fail("stored session must preserve source path");
    }
    if (session->displayModel.sourcePath != result.displayModel.sourcePath) {
        return fail("stored session must preserve display model");
    }
    if (session->shapeStore.edges().empty()) {
        return fail("stored session must expose ShapeStore edges");
    }
    if (!session->topologyBindings.ok()) {
        return fail("stored topology registry must be valid: "
                    + session->topologyBindings.diagnostic());
    }
    if (session->topologyBindings.edges().empty()) {
        return fail("stored topology registry must expose edge bindings");
    }

    const tsrs::step::TopologyBindingReference reference =
        tsrs::step::makeBindingReference(
            "selectedGuideEdge",
            session->topologyBindings.edges().front());
    const tsrs::step::TopologyBindingLookupResult restored =
        session->topologyBindings.restore(reference);
    if (!restored.ok) {
        return fail("stored topology registry must restore its own binding: "
                    + restored.diagnosticCode + " " + restored.diagnostic);
    }
    return 0;
}

} // namespace

int main()
{
    if (const int code = expectMissingFileDiagnostic()) {
        return code;
    }
    if (const int code = expectSuccessfulDisplayModel()) {
        return code;
    }
    if (const int code = expectImportCreatesStoredSessionWithTopologyBindings()) {
        return code;
    }
    return 0;
}
