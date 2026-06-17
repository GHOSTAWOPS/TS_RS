#include "drawing/detail/DetailPackageReader.h"

#include <QCryptographicHash>
#include <QFile>
#include <QString>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

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

QString toQString(const std::filesystem::path& path)
{
#ifdef _WIN32
    return QString::fromStdWString(path.wstring());
#else
    return QString::fromStdString(path.string());
#endif
}

std::string sha256(const std::filesystem::path& path)
{
    QFile file(toQString(path));
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    return QCryptographicHash::hash(file.readAll(), QCryptographicHash::Sha256)
        .toHex()
        .toStdString();
}

bool hasDiagnostic(
    const std::vector<tsrs::detail::DetailDiagnostic>& diagnostics,
    const std::string& code)
{
    for (const tsrs::detail::DetailDiagnostic& diagnostic : diagnostics) {
        if (diagnostic.code == code) {
            return true;
        }
    }
    return false;
}

int expectSyntheticPackageRead()
{
    const std::filesystem::path dir =
        makeTempDirectory("tsrs_detail_package_reader_synthetic");
    writeTextFile(dir / "Detail.xml", "<StyleRoot/>");
    writeTextFile(
        dir / "Detail01.stl",
        "<DrawingRoot customRoot=\"keep\">"
        "<HViewPorts><ViewPort><PartDetailDrawing num=\"1\">"
        "<section-line><lines>"
        "<Line1 start_x=\"0\" start_y=\"0\" end_x=\"1\" end_y=\"1\"/>"
        "</lines></section-line>"
        "<CustomNode customAttr=\"preserve\"/>"
        "</PartDetailDrawing></ViewPort></HViewPorts>"
        "<StbDetailDrawing><StbGroups stbGroupCount=\"1\">"
        "<StbGroup1 groupID=\"1\"><Std1 segCount=\"1\">"
        "<StbGeo1 shapeType=\"L\"/>"
        "</Std1><FaceEdge shapeType=\"L\"/></StbGroup1>"
        "</StbGroups></StbDetailDrawing>"
        "</DrawingRoot>");

    const tsrs::detail::DetailPackageReader reader;
    const tsrs::detail::DetailPackageSnapshot package = reader.readDirectory(dir.string());
    if (!package.ok()) {
        return fail("expected synthetic package to read without error diagnostics");
    }
    if (package.sourceDirectory != dir.string()) {
        return fail("expected source directory to be preserved");
    }
    if (package.files.size() != 2) {
        return fail("expected Detail.xml plus one Detail01.stl snapshot");
    }

    const std::optional<tsrs::detail::DetailFileSnapshot> style =
        findFile(package, "Detail.xml");
    if (!style || style->rootName != "StyleRoot" || style->sheetIndex != -1) {
        return fail("expected StyleRoot snapshot with non-sheet index");
    }
    if (style->rawXml != "<StyleRoot/>") {
        return fail("expected raw Detail.xml bytes to be preserved");
    }

    const std::optional<tsrs::detail::DetailFileSnapshot> sheet =
        findFile(package, "Detail01.stl");
    if (!sheet) {
        return fail("expected Detail01.stl snapshot");
    }
    if (sheet->sheetIndex != 1 || sheet->rootName != "DrawingRoot") {
        return fail("expected DrawingRoot sheet index 1");
    }
    if (sheet->knownSummary.xmlNodeCount != 14) {
        return fail("expected synthetic XML node count to include all start elements");
    }
    if (sheet->knownSummary.viewPortCount != 1
        || sheet->knownSummary.partDetailDrawingCount != 1
        || sheet->knownSummary.sectionLineCount != 1
        || sheet->knownSummary.stbGroupElementCount != 2
        || sheet->knownSummary.stdElementCount != 1
        || sheet->knownSummary.stbGeoElementCount != 1
        || sheet->knownSummary.faceEdgeCount != 1) {
        return fail("expected synthetic known summary counts");
    }
    if (sheet->rawXml.find("CustomNode") == std::string::npos
        || sheet->unknownChildren.empty()
        || sheet->rawAttributes.empty()) {
        return fail("expected raw XML, unknown child names, and raw attributes to be preserved");
    }
    return 0;
}

int expectMissingPackageDiagnostic()
{
    const tsrs::detail::DetailPackageReader reader;
    const tsrs::detail::DetailPackageSnapshot package =
        reader.readDirectory(
            (std::filesystem::temp_directory_path() / "tsrs_detail_reader_missing_dir")
                .string());

    if (package.ok()) {
        return fail("expected missing package to produce an error diagnostic");
    }
    if (!hasDiagnostic(package.diagnostics, tsrs::detail::kDetailDiagnosticPackageMissing)) {
        return fail("expected DETAIL_PACKAGE_MISSING diagnostic");
    }
    return 0;
}

int expectTodo66ManifestFixture()
{
    const std::filesystem::path fixture = locateTodo66Fixture();
    if (fixture.empty() || !std::filesystem::exists(fixture / "Detail01.stl")) {
        std::cout << "todo66 Detail fixture not found; local manifest probe skipped.\n";
        return 0;
    }

    const std::unordered_map<std::string, std::string> expectedHashes = {
        {"Detail.xml", "ccbd220d75d7f9c7e26e2540d639fa5956a369a31d7902f75ed36461f778f271"},
        {"Detail01.stl", "444be32ed907c0393104f415639ced2fb698f21c93df7f27f9ed830a70e40be6"},
        {"Detail02.stl", "478d166a5dce69a3bd8042e0118443780c8c7fbbdb93e7ac910eeb2ccb4e32f6"},
        {"Detail03.stl", "2531d11a15184909cc9fb3aa8cc96866f765e8254e443422854c176453a8fb43"},
        {"Detail04.stl", "03617911fbfcc2d53bcc310f515f8c375b3c6b78640c638cb2591da1494d4d0e"},
    };

    for (const auto& [fileName, expectedHash] : expectedHashes) {
        const std::string actualHash = sha256(fixture / fileName);
        if (actualHash != expectedHash) {
            return fail("todo66 manifest hash mismatch for " + fileName);
        }
    }

    const tsrs::detail::DetailPackageReader reader;
    const tsrs::detail::DetailPackageSnapshot package = reader.readDirectory(fixture.string());
    if (!package.ok()) {
        return fail("expected todo66 Detail fixture to read without error diagnostics");
    }
    if (package.files.size() != 5) {
        return fail("expected todo66 Detail.xml plus four DetailNN.stl snapshots");
    }

    struct ExpectedSheet {
        std::string fileName;
        int nodes;
        int viewPorts;
        int partDetailDrawings;
        int stbTables;
        int stbRows;
        int materialTables;
        int matRows;
        int sectionLines;
        int stbGroups;
        int stds;
        int stbGeos;
        int faceEdges;
    };

    const std::vector<ExpectedSheet> expectedSheets = {
        {"Detail01.stl", 138, 1, 1, 1, 7, 1, 2, 8, 13, 19, 23, 7},
        {"Detail02.stl", 67, 1, 1, 0, 0, 0, 0, 4, 7, 9, 9, 3},
        {"Detail03.stl", 73, 1, 1, 0, 0, 0, 0, 10, 6, 10, 12, 0},
        {"Detail04.stl", 81, 1, 1, 0, 0, 0, 0, 10, 8, 13, 15, 0},
    };

    const std::optional<tsrs::detail::DetailFileSnapshot> style =
        findFile(package, "Detail.xml");
    if (!style || style->rootName != "StyleRoot"
        || style->rawXml.find("<StyleRoot/>") == std::string::npos) {
        return fail("expected todo66 Detail.xml StyleRoot snapshot");
    }

    int expectedIndex = 1;
    for (const ExpectedSheet& expected : expectedSheets) {
        const std::optional<tsrs::detail::DetailFileSnapshot> sheet =
            findFile(package, expected.fileName);
        if (!sheet) {
            return fail("missing todo66 sheet snapshot: " + expected.fileName);
        }
        const tsrs::detail::DetailKnownSummary& summary = sheet->knownSummary;
        if (sheet->sheetIndex != expectedIndex || sheet->rootName != "DrawingRoot") {
            return fail("unexpected sheet index/root for " + expected.fileName);
        }
        if (summary.xmlNodeCount != expected.nodes
            || summary.viewPortCount != expected.viewPorts
            || summary.partDetailDrawingCount != expected.partDetailDrawings
            || summary.stbTableCount != expected.stbTables
            || summary.stbRowCount != expected.stbRows
            || summary.materialTableCount != expected.materialTables
            || summary.matRowCount != expected.matRows
            || summary.sectionLineCount != expected.sectionLines
            || summary.stbGroupElementCount != expected.stbGroups
            || summary.stdElementCount != expected.stds
            || summary.stbGeoElementCount != expected.stbGeos
            || summary.faceEdgeCount != expected.faceEdges) {
            return fail("unexpected known summary for " + expected.fileName);
        }
        if (sheet->rawXml.empty() || sheet->rawAttributes.empty()) {
            return fail("expected raw XML and raw attributes for " + expected.fileName);
        }
        ++expectedIndex;
    }

    return 0;
}

} // namespace

int main()
{
    try {
        if (const int code = expectSyntheticPackageRead()) {
            return code;
        }
        if (const int code = expectMissingPackageDiagnostic()) {
            return code;
        }
        if (const int code = expectTodo66ManifestFixture()) {
            return code;
        }
    } catch (const std::exception& exception) {
        return fail(std::string{"unexpected exception: "} + exception.what());
    }
    return 0;
}
