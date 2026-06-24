#include "step/StepImportService.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <STEPControl_Writer.hxx>
#include <TopoDS_Shape.hxx>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <cmath>
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
        std::filesystem::temp_directory_path() / "tsrs_step_import_box_fixture.step";
    std::filesystem::remove(path);

    const TopoDS_Shape box = BRepPrimAPI_MakeBox(1.0, 2.0, 3.0).Shape();
    STEPControl_Writer writer;
    writer.Transfer(box, STEPControl_AsIs);
    writer.Write(path.string().c_str());
    return path;
}

int expectSuccessImport()
{
    const std::filesystem::path fixture = writeBoxStepFixture();
    const tsrs::step::StepImportService service;
    const tsrs::step::StepImportResult result = service.importFile(fixture.string());

    if (!result.ok) {
        return fail("expected STEP import ok, got " + result.diagnostic);
    }
    if (result.rootShape.IsNull()) {
        return fail("expected non-null root shape");
    }
    if (result.rootCount < 1) {
        return fail("expected at least one STEP root");
    }
    if (result.faceCount < 6) {
        return fail("expected imported box to expose faces");
    }
    if (result.edgeCount < 12) {
        return fail("expected imported box to expose edges");
    }
    if (result.vertexCount < 8) {
        return fail("expected imported box to expose vertices");
    }
    if (result.sourcePath != fixture.string()) {
        return fail("expected source path to be preserved");
    }
    if (result.unitPolicy.internalLengthUnit != "m") {
        return fail("expected TS_RS internal length unit to be meter");
    }
    if (std::abs(result.unitPolicy.sourceToMeterScale - 0.001) > 1.0e-12) {
        return fail("expected OCCT-written STEP fixture to resolve to millimeter scale");
    }
    if (result.unitPolicy.shapeCoordinatesScaledToInternalMeters) {
        return fail("STEP import P0 must not silently scale OCCT display/topology shape");
    }
    return 0;
}

int expectKnownLengthUnitPolicy()
{
    const tsrs::step::StepLengthUnitPolicy policy =
        tsrs::step::makeStepLengthUnitPolicy("MM");

    if (!policy.sourceLengthUnitDetected) {
        return fail("known MM unit must be marked as detected");
    }
    if (!policy.sourceLengthUnitKnown) {
        return fail("known MM unit must be marked as known");
    }
    if (policy.normalizedSourceLengthUnit != "mm") {
        return fail("MM must normalize to mm");
    }
    if (std::abs(policy.sourceToMeterScale - 0.001) > 1.0e-12) {
        return fail("MM must convert to meter scale 0.001");
    }
    if (policy.sourceToMeterScaleAssumed) {
        return fail("known MM unit must not be marked as assumed");
    }
    return 0;
}

int expectAssumedLengthUnitPolicy()
{
    const tsrs::step::StepLengthUnitPolicy policy =
        tsrs::step::makeStepLengthUnitPolicy("");

    if (policy.sourceLengthUnitDetected) {
        return fail("empty STEP length unit must not be marked as detected");
    }
    if (policy.sourceLengthUnitKnown) {
        return fail("empty STEP length unit must not be marked as known");
    }
    if (policy.normalizedSourceLengthUnit != "mm") {
        return fail("empty STEP length unit must assume mm");
    }
    if (std::abs(policy.sourceToMeterScale - 0.001) > 1.0e-12) {
        return fail("assumed mm unit must convert to meter scale 0.001");
    }
    if (!policy.sourceToMeterScaleAssumed) {
        return fail("empty STEP length unit must be marked as an assumption");
    }
    return 0;
}

int expectMissingFileDiagnostic()
{
    const tsrs::step::StepImportService service;
    const tsrs::step::StepImportResult result =
        service.importFile("C:/definitely/missing/tsrs_missing.step");

    if (result.ok) {
        return fail("missing STEP file must fail");
    }
    if (result.diagnosticCode != tsrs::step::kStepImportDiagnosticMissingFile) {
        return fail("missing STEP file diagnostic mismatch: " + result.diagnosticCode);
    }
    return 0;
}

int expectUnsupportedExtensionDiagnostic()
{
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "tsrs_step_import_wrong_extension.txt";
    std::ofstream file(path);
    file << "not a step file";
    file.close();

    const tsrs::step::StepImportService service;
    const tsrs::step::StepImportResult result = service.importFile(path.string());
    std::filesystem::remove(path);

    if (result.ok) {
        return fail("non-step extension must fail");
    }
    if (result.diagnosticCode != tsrs::step::kStepImportDiagnosticUnsupportedExtension) {
        return fail("unsupported extension diagnostic mismatch: " + result.diagnosticCode);
    }
    return 0;
}

} // namespace

int main()
{
    if (const int code = expectKnownLengthUnitPolicy()) {
        return code;
    }
    if (const int code = expectAssumedLengthUnitPolicy()) {
        return code;
    }
    if (const int code = expectSuccessImport()) {
        return code;
    }
    if (const int code = expectMissingFileDiagnostic()) {
        return code;
    }
    if (const int code = expectUnsupportedExtensionDiagnostic()) {
        return code;
    }
    return 0;
}
