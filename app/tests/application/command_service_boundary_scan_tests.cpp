#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

int fail(const std::string& message)
{
    std::cerr << message << '\n';
    return 1;
}

std::filesystem::path findAppRoot()
{
    std::filesystem::path current = std::filesystem::current_path();
    for (int depth = 0; depth < 8; ++depth) {
        const std::filesystem::path candidate = current / "app" / "src";
        if (std::filesystem::exists(candidate)) {
            return current / "app";
        }
        if (!current.has_parent_path()) {
            break;
        }
        current = current.parent_path();
    }
    return {};
}

std::string readText(const std::filesystem::path& path)
{
    std::ifstream input(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(input),
            std::istreambuf_iterator<char>()};
}

int expectNoForbiddenPatterns(
    const std::filesystem::path& root,
    const std::vector<std::filesystem::path>& directories,
    const std::vector<std::string>& forbidden)
{
    for (const std::filesystem::path& directory : directories) {
        const std::filesystem::path scanRoot = root / directory;
        if (!std::filesystem::exists(scanRoot)) {
            continue;
        }
        for (const std::filesystem::directory_entry& entry :
             std::filesystem::recursive_directory_iterator(scanRoot)) {
            if (!entry.is_regular_file()) {
                continue;
            }
            const std::filesystem::path path = entry.path();
            if (path.extension() != ".h" && path.extension() != ".cpp") {
                continue;
            }
            const std::string text = readText(path);
            for (const std::string& pattern : forbidden) {
                if (text.find(pattern) != std::string::npos) {
                    return fail("forbidden boundary pattern '" + pattern
                                + "' found in " + path.string());
                }
            }
        }
    }
    return 0;
}

} // namespace

int main()
{
    const std::filesystem::path appRoot = findAppRoot();
    if (appRoot.empty()) {
        return fail("unable to locate app root for boundary scan");
    }

    if (const int code = expectNoForbiddenPatterns(
            appRoot,
            {"src/ui", "src/presentation/occ"},
            {"rebarsmart/generators",
             "FixDistanceGenerator",
             "FixNumberGenerator",
             "drawing/detail",
             "DetailPackageWriter",
             "DetailPackageReader"})) {
        return code;
    }

    if (const int code = expectNoForbiddenPatterns(
            appRoot,
            {"src/application/commands"},
            {"rebarsmart/generators/FixDistanceGenerator",
             "rebarsmart/generators/FixNumberGenerator",
             "drawing/detail/DetailPackageWriter",
             "drawing/detail/DetailPackageReader"})) {
        return code;
    }

    if (const int code = expectNoForbiddenPatterns(
            appRoot,
            {"src/rebarsmart", "src/drawing"},
            {"TopoDS_", "AIS_", "BRep", "TopAbs", "gp_", "Geom_"})) {
        return code;
    }

    return 0;
}
