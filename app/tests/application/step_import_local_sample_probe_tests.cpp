#include "application/commands/StepImportCommandService.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

namespace {

int fail(const std::string& message)
{
    std::cerr << message << '\n';
    return 1;
}

} // namespace

int main()
{
    const char* samplePath = std::getenv("TSRS_LOCAL_STEP_SAMPLE");
    if (samplePath == nullptr || std::string(samplePath).empty()) {
        std::cout << "TSRS_LOCAL_STEP_SAMPLE not set; local STEP probe skipped.\n";
        return 0;
    }

    const std::filesystem::path path(samplePath);
    const tsrs::application::StepImportCommandService service;
    const tsrs::application::StepImportCommandResult result =
        service.importStep({path.string()});

    if (!result.ok) {
        return fail("local STEP import failed: " + result.diagnosticCode + " " + result.diagnostic);
    }

    std::cout << "source=" << result.displayModel.sourcePath << '\n';
    std::cout << "rootCount=" << result.displayModel.rootCount << '\n';
    std::cout << "solidCount=" << result.displayModel.solidCount << '\n';
    std::cout << "faceCount=" << result.displayModel.faceCount << '\n';
    std::cout << "edgeCount=" << result.displayModel.edgeCount << '\n';
    std::cout << "vertexCount=" << result.displayModel.vertexCount << '\n';
    return 0;
}
