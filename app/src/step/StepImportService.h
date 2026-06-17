#pragma once

#include <TopoDS_Shape.hxx>

#include <string>

namespace tsrs::step {

inline constexpr const char* kStepImportDiagnosticOk = "STEP_IMPORT_OK";
inline constexpr const char* kStepImportDiagnosticMissingFile = "STEP_IMPORT_MISSING_FILE";
inline constexpr const char* kStepImportDiagnosticUnsupportedExtension =
    "STEP_IMPORT_UNSUPPORTED_EXTENSION";
inline constexpr const char* kStepImportDiagnosticReadFailed = "STEP_IMPORT_READ_FAILED";
inline constexpr const char* kStepImportDiagnosticTransferFailed = "STEP_IMPORT_TRANSFER_FAILED";
inline constexpr const char* kStepImportDiagnosticEmptyShape = "STEP_IMPORT_EMPTY_SHAPE";
inline constexpr const char* kStepImportDiagnosticException = "STEP_IMPORT_EXCEPTION";

struct StepImportResult {
    bool ok{false};
    bool readOk{false};
    bool transferOk{false};
    std::string diagnosticCode{kStepImportDiagnosticReadFailed};
    std::string diagnostic;
    std::string sourcePath;
    int rootCount{0};
    int solidCount{0};
    int faceCount{0};
    int edgeCount{0};
    int vertexCount{0};
    TopoDS_Shape rootShape;
};

class StepImportService final {
public:
    [[nodiscard]] StepImportResult importFile(const std::string& stepPath) const;
};

} // namespace tsrs::step
