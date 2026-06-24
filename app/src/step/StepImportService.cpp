#include "step/StepImportService.h"

#include <IFSelect_ReturnStatus.hxx>
#include <STEPControl_Reader.hxx>
#include <Standard_Failure.hxx>
#include <TColStd_SequenceOfAsciiString.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Shape.hxx>

#include <algorithm>
#include <cctype>
#include <exception>
#include <filesystem>
#include <sstream>
#include <string>

namespace {

bool hasStepExtension(const std::filesystem::path& path)
{
    std::string extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return extension == ".stp" || extension == ".step";
}

int countSubShapes(const TopoDS_Shape& shape, TopAbs_ShapeEnum kind)
{
    int count = 0;
    for (TopExp_Explorer explorer(shape, kind); explorer.More(); explorer.Next()) {
        ++count;
    }
    return count;
}

std::string readStatusMessage(IFSelect_ReturnStatus status)
{
    std::ostringstream stream;
    stream << "STEP ReadFile failed with status " << static_cast<int>(status);
    return stream.str();
}

std::string firstUnit(const TColStd_SequenceOfAsciiString& units)
{
    if (units.IsEmpty()) {
        return {};
    }
    return units.First().ToCString();
}

} // namespace

namespace tsrs::step {

StepImportResult StepImportService::importFile(const std::string& stepPath) const
{
    StepImportResult result;
    result.sourcePath = stepPath;

    const std::filesystem::path path(stepPath);
    if (!std::filesystem::exists(path)) {
        result.diagnosticCode = kStepImportDiagnosticMissingFile;
        result.diagnostic = "STEP file does not exist: " + stepPath;
        return result;
    }

    if (!hasStepExtension(path)) {
        result.diagnosticCode = kStepImportDiagnosticUnsupportedExtension;
        result.diagnostic = "Only .stp/.step files are supported: " + stepPath;
        return result;
    }

    try {
        STEPControl_Reader reader;
        const IFSelect_ReturnStatus status = reader.ReadFile(stepPath.c_str());
        result.readOk = status == IFSelect_RetDone;
        if (!result.readOk) {
            result.diagnosticCode = kStepImportDiagnosticReadFailed;
            result.diagnostic = readStatusMessage(status);
            return result;
        }

        result.rootCount = reader.NbRootsForTransfer();
        TColStd_SequenceOfAsciiString lengthUnits;
        TColStd_SequenceOfAsciiString angleUnits;
        TColStd_SequenceOfAsciiString solidAngleUnits;
        reader.FileUnits(lengthUnits, angleUnits, solidAngleUnits);
        result.unitPolicy = makeStepLengthUnitPolicy(firstUnit(lengthUnits));

        const Standard_Integer transferred = reader.TransferRoots();
        result.transferOk = transferred > 0;
        if (!result.transferOk) {
            result.diagnosticCode = kStepImportDiagnosticTransferFailed;
            result.diagnostic = "STEP TransferRoots produced no transferred roots.";
            return result;
        }

        result.rootShape = reader.OneShape();
        if (result.rootShape.IsNull()) {
            result.diagnosticCode = kStepImportDiagnosticEmptyShape;
            result.diagnostic = "STEP import produced an empty root shape.";
            return result;
        }

        result.solidCount = countSubShapes(result.rootShape, TopAbs_SOLID);
        result.faceCount = countSubShapes(result.rootShape, TopAbs_FACE);
        result.edgeCount = countSubShapes(result.rootShape, TopAbs_EDGE);
        result.vertexCount = countSubShapes(result.rootShape, TopAbs_VERTEX);
        result.ok = true;
        result.diagnosticCode = kStepImportDiagnosticOk;
        return result;
    } catch (const Standard_Failure& failure) {
        result.diagnosticCode = kStepImportDiagnosticException;
        result.diagnostic = std::string("OCCT exception: ") + failure.GetMessageString();
    } catch (const std::exception& exception) {
        result.diagnosticCode = kStepImportDiagnosticException;
        result.diagnostic = std::string("std::exception: ") + exception.what();
    } catch (...) {
        result.diagnosticCode = kStepImportDiagnosticException;
        result.diagnostic = "Unknown exception while importing STEP.";
    }
    return result;
}

} // namespace tsrs::step
