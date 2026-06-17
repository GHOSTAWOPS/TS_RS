#include "drawing/detail/DetailPackageReader.h"

#include <QByteArray>
#include <QFile>
#include <QString>
#include <QXmlStreamReader>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace {

QString toQString(const std::filesystem::path& path)
{
#ifdef _WIN32
    return QString::fromStdWString(path.wstring());
#else
    return QString::fromStdString(path.string());
#endif
}

std::string toUtf8(const QString& value)
{
    return value.toUtf8().toStdString();
}

std::string fileNameOf(const std::filesystem::path& path)
{
    return path.filename().u8string();
}

std::string extensionLower(const std::filesystem::path& path)
{
    std::string extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return extension;
}

bool startsWith(const std::string& value, const std::string& prefix)
{
    return value.rfind(prefix, 0) == 0;
}

bool isKnownDetailElement(const std::string& name)
{
    static const std::set<std::string> knownExact = {
        "DrawingRoot",
        "StyleRoot",
        "StbTables",
        "StbTable",
        "MaterialTable",
        "HViewPorts",
        "ViewPort",
        "PartDetailDrawing",
        "General-Info",
        "continue-line",
        "hidden-line",
        "central-line",
        "section-line",
        "hatch-line",
        "Others",
        "steeljoint-line",
        "StbDetailDrawing",
        "StbGroups",
        "FaceEdge",
        "Line",
        "lines",
        "circles",
        "Arcs",
        "Ellipses",
        "EllipseArcs",
        "Splines",
        "joints",
    };

    return knownExact.find(name) != knownExact.end()
        || startsWith(name, "StbRow")
        || startsWith(name, "StbSeg")
        || startsWith(name, "MatRow")
        || startsWith(name, "StbGroup")
        || startsWith(name, "Std")
        || startsWith(name, "StbGeo")
        || startsWith(name, "Line");
}

bool isErrorDiagnostic(const tsrs::detail::DetailDiagnostic& diagnostic)
{
    return diagnostic.severity == "error";
}

tsrs::detail::DetailDiagnostic makeDiagnostic(
    std::string code,
    std::string severity,
    std::string fileName,
    int sheetIndex,
    std::string nodePath,
    std::string message)
{
    tsrs::detail::DetailDiagnostic diagnostic;
    diagnostic.code = std::move(code);
    diagnostic.severity = std::move(severity);
    diagnostic.fileName = std::move(fileName);
    diagnostic.sheetIndex = sheetIndex;
    diagnostic.nodePath = std::move(nodePath);
    diagnostic.message = std::move(message);
    return diagnostic;
}

std::string readRawFile(const std::filesystem::path& path)
{
    QFile file(toQString(path));
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    return file.readAll().toStdString();
}

int sheetIndexFromName(const std::string& fileName)
{
    if (!startsWith(fileName, "Detail") || fileName.size() < 12) {
        return -1;
    }
    const std::string digits = fileName.substr(6, 2);
    if (!std::isdigit(static_cast<unsigned char>(digits[0]))
        || !std::isdigit(static_cast<unsigned char>(digits[1]))) {
        return -1;
    }
    return std::stoi(digits);
}

void countKnownElement(
    const std::string& name,
    const std::vector<std::string>& pathStack,
    tsrs::detail::DetailKnownSummary& summary)
{
    ++summary.xmlNodeCount;

    if (name == "ViewPort") {
        ++summary.viewPortCount;
    } else if (name == "PartDetailDrawing") {
        ++summary.partDetailDrawingCount;
    } else if (name == "StbTable") {
        ++summary.stbTableCount;
    } else if (startsWith(name, "StbRow")) {
        ++summary.stbRowCount;
    } else if (name == "MaterialTable") {
        ++summary.materialTableCount;
    } else if (startsWith(name, "MatRow")) {
        ++summary.matRowCount;
    } else if (startsWith(name, "StbGroup")) {
        ++summary.stbGroupElementCount;
    } else if (startsWith(name, "Std")) {
        ++summary.stdElementCount;
    } else if (startsWith(name, "StbGeo")) {
        ++summary.stbGeoElementCount;
    } else if (name == "FaceEdge") {
        ++summary.faceEdgeCount;
    } else if (startsWith(name, "Line")) {
        const bool underSectionLines = pathStack.size() >= 3
            && pathStack[pathStack.size() - 3] == "section-line"
            && pathStack[pathStack.size() - 2] == "lines";
        if (underSectionLines) {
            ++summary.sectionLineCount;
        }
    }
}

std::string nodePathOf(const std::vector<std::string>& pathStack)
{
    std::ostringstream stream;
    for (const std::string& name : pathStack) {
        stream << '/' << name;
    }
    return stream.str();
}

tsrs::detail::DetailFileSnapshot readXmlFile(const std::filesystem::path& path, int sheetIndex)
{
    tsrs::detail::DetailFileSnapshot snapshot;
    snapshot.fileName = fileNameOf(path);
    snapshot.sheetIndex = sheetIndex;
    snapshot.rawXml = readRawFile(path);

    QXmlStreamReader xml(QString::fromStdString(snapshot.rawXml));
    std::vector<std::string> pathStack;

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            const std::string name = toUtf8(xml.name().toString());
            pathStack.push_back(name);
            if (snapshot.rootName.empty()) {
                snapshot.rootName = name;
            }

            countKnownElement(name, pathStack, snapshot.knownSummary);

            const QXmlStreamAttributes attributes = xml.attributes();
            for (const QXmlStreamAttribute& attribute : attributes) {
                snapshot.rawAttributes.push_back(
                    nodePathOf(pathStack) + "/@" + toUtf8(attribute.name().toString()));
            }

            if (!isKnownDetailElement(name)) {
                snapshot.unknownChildren.push_back(nodePathOf(pathStack));
            }
        } else if (xml.isEndElement() && !pathStack.empty()) {
            pathStack.pop_back();
        }
    }

    if (xml.hasError()) {
        snapshot.diagnostics.push_back(makeDiagnostic(
            tsrs::detail::kDetailDiagnosticXmlParseFailed,
            "error",
            snapshot.fileName,
            snapshot.sheetIndex,
            nodePathOf(pathStack),
            toUtf8(xml.errorString())));
    }

    const bool isStyleFile = snapshot.fileName == "Detail.xml";
    const std::string expectedRoot = isStyleFile ? "StyleRoot" : "DrawingRoot";
    if (!snapshot.rootName.empty() && snapshot.rootName != expectedRoot) {
        snapshot.diagnostics.push_back(makeDiagnostic(
            tsrs::detail::kDetailDiagnosticRootUnexpected,
            "warning",
            snapshot.fileName,
            snapshot.sheetIndex,
            "/" + snapshot.rootName,
            "Unexpected Detail XML root: " + snapshot.rootName));
    }

    return snapshot;
}

std::vector<std::filesystem::path> sortedSheetFiles(const std::filesystem::path& directory)
{
    std::vector<std::filesystem::path> files;
    for (const std::filesystem::directory_entry& entry :
         std::filesystem::directory_iterator(directory)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        const std::filesystem::path path = entry.path();
        const std::string fileName = fileNameOf(path);
        if (startsWith(fileName, "Detail")
            && extensionLower(path) == ".stl"
            && sheetIndexFromName(fileName) > 0) {
            files.push_back(path);
        }
    }

    std::sort(files.begin(), files.end(), [](const auto& lhs, const auto& rhs) {
        return fileNameOf(lhs) < fileNameOf(rhs);
    });
    return files;
}

} // namespace

namespace tsrs::detail {

bool DetailFileSnapshot::ok() const
{
    return std::none_of(diagnostics.begin(), diagnostics.end(), isErrorDiagnostic);
}

bool DetailPackageSnapshot::ok() const
{
    if (std::any_of(diagnostics.begin(), diagnostics.end(), isErrorDiagnostic)) {
        return false;
    }
    return std::all_of(files.begin(), files.end(), [](const DetailFileSnapshot& file) {
        return file.ok();
    });
}

DetailPackageSnapshot DetailPackageReader::readDirectory(const std::string& directoryPath) const
{
    DetailPackageSnapshot package;
    const std::filesystem::path directory(directoryPath);
    package.sourceDirectory = directory.string();

    if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
        package.diagnostics.push_back(makeDiagnostic(
            kDetailDiagnosticPackageMissing,
            "error",
            {},
            -1,
            {},
            "Detail package directory does not exist: " + directory.string()));
        return package;
    }

    const std::filesystem::path stylePath = directory / "Detail.xml";
    if (std::filesystem::exists(stylePath)) {
        package.files.push_back(readXmlFile(stylePath, -1));
    } else {
        package.diagnostics.push_back(makeDiagnostic(
            kDetailDiagnosticStyleMissing,
            "error",
            "Detail.xml",
            -1,
            {},
            "Detail.xml is missing."));
    }

    const std::vector<std::filesystem::path> sheets = sortedSheetFiles(directory);
    if (sheets.empty()) {
        package.diagnostics.push_back(makeDiagnostic(
            kDetailDiagnosticSheetMissing,
            "error",
            {},
            -1,
            {},
            "No DetailNN.stl sheet files found."));
        return package;
    }

    int expectedSheetIndex = 1;
    for (const std::filesystem::path& sheetPath : sheets) {
        const int sheetIndex = sheetIndexFromName(fileNameOf(sheetPath));
        if (sheetIndex != expectedSheetIndex) {
            package.diagnostics.push_back(makeDiagnostic(
                kDetailDiagnosticSheetIndexGap,
                "warning",
                fileNameOf(sheetPath),
                sheetIndex,
                {},
                "Detail sheet index is not contiguous."));
            expectedSheetIndex = sheetIndex;
        }
        package.files.push_back(readXmlFile(sheetPath, sheetIndex));
        ++expectedSheetIndex;
    }

    return package;
}

} // namespace tsrs::detail
