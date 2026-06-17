#pragma once

#include "presentation/occ/StepDisplayModel.h"

#include <string>

namespace tsrs::application {

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
    tsrs::presentation::StepDisplayModel displayModel;
};

class StepImportCommandService final {
public:
    [[nodiscard]] StepImportCommandResult importStep(
        const StepImportCommandRequest& request) const;
};

} // namespace tsrs::application
