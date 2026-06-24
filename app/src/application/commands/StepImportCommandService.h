#pragma once

#include "presentation/occ/StepDisplayModel.h"

#include <string>

namespace tsrs::application {

class ImportedModelStore;

inline constexpr const char* kStepImportCommandDiagnosticOk =
    "STEP_IMPORT_OK";
inline constexpr const char* kStepImportCommandDiagnosticMissingFile =
    "STEP_IMPORT_MISSING_FILE";

struct StepImportCommandRequest {
    std::string path;
};

struct StepImportCommandResult {
    bool ok{false};
    std::string diagnosticCode;
    std::string diagnostic;
    std::string sourcePath;
    std::string sessionId;
    tsrs::presentation::StepDisplayModel displayModel;
};

class StepImportCommandService final {
public:
    explicit StepImportCommandService(ImportedModelStore* importedModelStore = nullptr);

    [[nodiscard]] StepImportCommandResult importStep(
        const StepImportCommandRequest& request) const;

private:
    ImportedModelStore* importedModelStore_{nullptr};
};

} // namespace tsrs::application
