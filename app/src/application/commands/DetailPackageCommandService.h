#pragma once

#include <string>

namespace tsrs::application {

inline constexpr const char* kDetailPackageCommandDiagnosticNotImplemented =
    "DETAIL_PACKAGE_COMMAND_NOT_IMPLEMENTED";

struct DetailPackageExportRequest {
    std::string targetDirectory;
};

struct DetailPackageCommandResult {
    bool ok{false};
    std::string diagnosticCode{kDetailPackageCommandDiagnosticNotImplemented};
    std::string diagnostic;
};

class DetailPackageCommandService final {
public:
    [[nodiscard]] DetailPackageCommandResult exportCurrentProject(
        const DetailPackageExportRequest& request) const;
};

} // namespace tsrs::application
