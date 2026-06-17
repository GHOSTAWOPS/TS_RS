#include "drawing/detail/DetailPackageReader.h"
#include "drawing/detail/DetailPackageWriter.h"

#include <QFile>
#include <QString>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>

namespace {

int fail(const std::string& message)
{
    std::cerr << message << '\n';
    return 1;
}

void writeTextFile(const std::filesystem::path& path, const std::string& content)
{
    std::ofstream stream(path, std::ios::binary);
    stream << content;
}

std::string readTextFile(const std::filesystem::path& path)
{
    std::ifstream stream(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
}

std::filesystem::path makeTempDirectory(const std::string& name)
{
    const std::filesystem::path path = std::filesystem::temp_directory_path() / name;
    std::filesystem::remove_all(path);
    std::filesystem::create_directories(path);
    return path;
}

std::filesystem::path locateTodo66Fixture()
{
    if (const char* env = std::getenv("TSRS_LOCAL_DETAIL_TODO66")) {
        if (*env != '\0') {
            return std::filesystem::path(env);
        }
    }

    std::filesystem::path cursor = std::filesystem::current_path();
    for (int depth = 0; depth < 8; ++depth) {
        const std::filesystem::path candidate = cursor / "docs" / "phase1" / "todo66";
        if (std::filesystem::exists(candidate / "Detail01.stl")) {
            return candidate;
        }
        if (!cursor.has_parent_path() || cursor == cursor.parent_path()) {
            break;
        }
        cursor = cursor.parent_path();
    }
    return {};
}

std::optional<tsrs::detail::DetailFileSnapshot> findFile(
    const tsrs::detail::DetailPackageSnapshot& package,
    const std::string& fileName)
{
    for (const tsrs::detail::DetailFileSnapshot& file : package.files) {
        if (file.fileName == fileName) {
            return file;
        }
    }
    return std::nullopt;
}

bool sameSummary(
    const tsrs::detail::DetailKnownSummary& lhs,
    const tsrs::detail::DetailKnownSummary& rhs)
{
    return lhs.xmlNodeCount == rhs.xmlNodeCount
        && lhs.viewPortCount == rhs.viewPortCount
        && lhs.partDetailDrawingCount == rhs.partDetailDrawingCount
        && lhs.stbTableCount == rhs.stbTableCount
        && lhs.stbRowCount == rhs.stbRowCount
        && lhs.materialTableCount == rhs.materialTableCount
        && lhs.matRowCount == rhs.matRowCount
        && lhs.sectionLineCount == rhs.sectionLineCount
        && lhs.stbGroupElementCount == rhs.stbGroupElementCount
        && lhs.stbGroupsContainerCount == rhs.stbGroupsContainerCount
        && lhs.stbGroupEntryCount == rhs.stbGroupEntryCount
        && lhs.stdElementCount == rhs.stdElementCount
        && lhs.stbGeoElementCount == rhs.stbGeoElementCount
        && lhs.faceEdgeCount == rhs.faceEdgeCount;
}

int expectSyntheticRawPassthroughRoundTrip()
{
    const std::filesystem::path sourceDir =
        makeTempDirectory("tsrs_detail_package_writer_synthetic_source");
    const std::filesystem::path outputDir =
        makeTempDirectory("tsrs_detail_package_writer_synthetic_output");

    const std::string styleXml = "<StyleRoot customStyle=\"yes\"/>\n";
    const std::string sheetXml =
        "<DrawingRoot customRoot=\"keep\">\n"
        "  <HViewPorts><ViewPort><PartDetailDrawing num=\"1\">\n"
        "    <section-line><lines><Line1 start_x=\"0\" start_y=\"0\" end_x=\"1\" end_y=\"1\"/></lines></section-line>\n"
        "    <UnknownFutureNode customAttr=\"preserve\">raw text</UnknownFutureNode>\n"
        "  </PartDetailDrawing></ViewPort></HViewPorts>\n"
        "</DrawingRoot>\n";

    writeTextFile(sourceDir / "Detail.xml", styleXml);
    writeTextFile(sourceDir / "Detail01.stl", sheetXml);

    const tsrs::detail::DetailPackageReader reader;
    const tsrs::detail::DetailPackageSnapshot sourcePackage =
        reader.readDirectory(sourceDir.string());
    if (!sourcePackage.ok()) {
        return fail("expected synthetic source package to read before write");
    }

    const tsrs::detail::DetailPackageWriter writer;
    const tsrs::detail::DetailPackageWriteResult writeResult =
        writer.writePreserveMode(sourcePackage, outputDir.string());
    if (!writeResult.ok()) {
        return fail("expected preserve-mode writer to finish without error diagnostics");
    }
    if (writeResult.filesWritten != 2) {
        return fail("expected writer to report two written files");
    }
    if (readTextFile(outputDir / "Detail.xml") != styleXml
        || readTextFile(outputDir / "Detail01.stl") != sheetXml) {
        return fail("expected writer to preserve rawXml bytes exactly");
    }

    const tsrs::detail::DetailPackageSnapshot roundTripPackage =
        reader.readDirectory(outputDir.string());
    if (!roundTripPackage.ok() || roundTripPackage.files.size() != sourcePackage.files.size()) {
        return fail("expected round-trip output package to read with same file count");
    }

    const std::optional<tsrs::detail::DetailFileSnapshot> before =
        findFile(sourcePackage, "Detail01.stl");
    const std::optional<tsrs::detail::DetailFileSnapshot> after =
        findFile(roundTripPackage, "Detail01.stl");
    if (!before || !after || !sameSummary(before->knownSummary, after->knownSummary)) {
        return fail("expected known summary to match after round-trip");
    }
    if (after->rawAttributes.size() < before->rawAttributes.size()
        || after->unknownChildren.size() < before->unknownChildren.size()) {
        return fail("expected unknown attributes and children not to decrease");
    }
    return 0;
}

int expectWriterValidatesOutput()
{
    const std::filesystem::path sourceDir =
        makeTempDirectory("tsrs_detail_package_writer_validate_source");
    const std::filesystem::path outputDir =
        makeTempDirectory("tsrs_detail_package_writer_validate_output");

    writeTextFile(sourceDir / "Detail.xml", "<StyleRoot/>");
    writeTextFile(sourceDir / "Detail01.stl", "<DrawingRoot><Broken></DrawingRoot>");

    const tsrs::detail::DetailPackageReader reader;
    const tsrs::detail::DetailPackageSnapshot sourcePackage =
        reader.readDirectory(sourceDir.string());
    const tsrs::detail::DetailPackageWriter writer;
    const tsrs::detail::DetailPackageWriteResult writeResult =
        writer.writePreserveMode(sourcePackage, outputDir.string());

    if (writeResult.ok()) {
        return fail("expected writer validation to reject malformed rawXml output");
    }
    bool sawValidateFailed = false;
    for (const tsrs::detail::DetailDiagnostic& diagnostic : writeResult.diagnostics) {
        if (diagnostic.code == tsrs::detail::kDetailDiagnosticWriteValidateFailed) {
            sawValidateFailed = true;
        }
    }
    if (!sawValidateFailed) {
        return fail("expected DETAIL_WRITE_VALIDATE_FAILED diagnostic");
    }
    return 0;
}

int expectWriterRejectsUnsafeFileName()
{
    const std::filesystem::path outputDir =
        makeTempDirectory("tsrs_detail_package_writer_unsafe_output");

    tsrs::detail::DetailPackageSnapshot package;
    package.files.push_back({});
    package.files.back().fileName = "../tsrs_detail_writer_escape_probe.stl";
    package.files.back().sheetIndex = 1;
    package.files.back().rawXml = "<DrawingRoot/>";
    const std::filesystem::path escapedPath =
        outputDir.parent_path() / "tsrs_detail_writer_escape_probe.stl";
    std::filesystem::remove(escapedPath);

    const tsrs::detail::DetailPackageWriter writer;
    const tsrs::detail::DetailPackageWriteResult writeResult =
        writer.writePreserveMode(package, outputDir.string());

    if (writeResult.ok()) {
        return fail("expected writer to reject unsafe fileName");
    }
    if (std::filesystem::exists(escapedPath)) {
        return fail("expected unsafe fileName not to escape target directory");
    }
    return 0;
}

int expectTodo66PreserveModeRoundTrip()
{
    const std::filesystem::path fixture = locateTodo66Fixture();
    if (fixture.empty() || !std::filesystem::exists(fixture / "Detail01.stl")) {
        std::cout << "todo66 Detail fixture not found; writer round-trip probe skipped.\n";
        return 77;
    }

    const std::filesystem::path outputDir =
        makeTempDirectory("tsrs_detail_package_writer_todo66_output");
    const tsrs::detail::DetailPackageReader reader;
    const tsrs::detail::DetailPackageSnapshot sourcePackage =
        reader.readDirectory(fixture.string());
    if (!sourcePackage.ok()) {
        return fail("expected todo66 fixture to read before writer round-trip");
    }

    const tsrs::detail::DetailPackageWriter writer;
    const tsrs::detail::DetailPackageWriteResult writeResult =
        writer.writePreserveMode(sourcePackage, outputDir.string());
    if (!writeResult.ok() || writeResult.filesWritten != sourcePackage.files.size()) {
        return fail("expected todo66 writer round-trip to write all files");
    }

    const tsrs::detail::DetailPackageSnapshot roundTripPackage =
        reader.readDirectory(outputDir.string());
    if (!roundTripPackage.ok() || roundTripPackage.files.size() != sourcePackage.files.size()) {
        return fail("expected todo66 round-trip output to read with same file count");
    }

    for (const tsrs::detail::DetailFileSnapshot& before : sourcePackage.files) {
        const std::optional<tsrs::detail::DetailFileSnapshot> after =
            findFile(roundTripPackage, before.fileName);
        if (!after || !sameSummary(before.knownSummary, after->knownSummary)) {
            return fail("expected todo66 summary to match for " + before.fileName);
        }
        if (readTextFile(outputDir / before.fileName) != before.rawXml) {
            return fail("expected todo66 rawXml passthrough for " + before.fileName);
        }
    }
    return 0;
}

} // namespace

int main(int argc, char** argv)
{
    try {
        if (argc > 1 && std::string{argv[1]} == "--todo66-fixture") {
            return expectTodo66PreserveModeRoundTrip();
        }
        if (const int code = expectSyntheticRawPassthroughRoundTrip()) {
            return code;
        }
        if (const int code = expectWriterValidatesOutput()) {
            return code;
        }
        if (const int code = expectWriterRejectsUnsafeFileName()) {
            return code;
        }
    } catch (const std::exception& exception) {
        return fail(std::string{"unexpected exception: "} + exception.what());
    }
    return 0;
}
