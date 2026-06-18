#include "drawing/detail/DetailPackageWriter.h"

#include <QFile>
#include <QString>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <locale>
#include <sstream>
#include <string>

namespace {

using tsrs::detail::DetailDiagnostic;
using tsrs::detail::DetailFileSnapshot;
using tsrs::detail::DetailPackageSnapshot;
using tsrs::detail::DetailPackageWriteResult;
using tsrs::detail::kDetailDiagnosticWriteFailed;
using tsrs::detail::kDetailDiagnosticWriteValidateFailed;

QString toQString(const std::filesystem::path& path)
{
#ifdef _WIN32
    return QString::fromStdWString(path.wstring());
#else
    return QString::fromStdString(path.string());
#endif
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
    std::string message)
{
    tsrs::detail::DetailDiagnostic diagnostic;
    diagnostic.code = std::move(code);
    diagnostic.severity = std::move(severity);
    diagnostic.fileName = std::move(fileName);
    diagnostic.sheetIndex = sheetIndex;
    diagnostic.line = -1;
    diagnostic.column = -1;
    diagnostic.message = std::move(message);
    return diagnostic;
}

bool writeRawXmlFile(const std::filesystem::path& path, const std::string& rawXml)
{
    QFile file(toQString(path));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    return file.write(rawXml.data(), static_cast<qint64>(rawXml.size()))
        == static_cast<qint64>(rawXml.size());
}

std::string readRawFile(const std::filesystem::path& path)
{
    QFile file(toQString(path));
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    return file.readAll().toStdString();
}

void appendDiagnostics(
    tsrs::detail::DetailPackageWriteResult& result,
    const tsrs::detail::DetailPackageSnapshot& package)
{
    for (const tsrs::detail::DetailDiagnostic& diagnostic : package.diagnostics) {
        result.diagnostics.push_back(diagnostic);
    }
    for (const tsrs::detail::DetailFileSnapshot& file : package.files) {
        result.diagnostics.insert(
            result.diagnostics.end(),
            file.diagnostics.begin(),
            file.diagnostics.end());
    }
}

bool recreateDirectory(
    const std::filesystem::path& directory,
    tsrs::detail::DetailPackageWriteResult& result,
    const std::string& label)
{
    std::error_code error;
    std::filesystem::remove_all(directory, error);
    if (error) {
        result.diagnostics.push_back(makeDiagnostic(
            kDetailDiagnosticWriteFailed,
            "error",
            {},
            -1,
            "Failed to remove " + label + " Detail directory: " + error.message()));
        return false;
    }

    std::filesystem::create_directories(directory, error);
    if (error) {
        result.diagnostics.push_back(makeDiagnostic(
            kDetailDiagnosticWriteFailed,
            "error",
            {},
            -1,
            "Failed to create " + label + " Detail directory: " + error.message()));
        return false;
    }
    return true;
}

std::filesystem::path normalizeTargetDirectoryPath(const std::filesystem::path& target)
{
    std::filesystem::path normalized = target.lexically_normal();
    while (!normalized.empty()
        && normalized.filename().empty()
        && normalized.has_parent_path()) {
        const std::filesystem::path parent = normalized.parent_path();
        if (parent == normalized) {
            break;
        }
        normalized = parent;
    }
    return normalized;
}

std::filesystem::path makeSiblingTempDirectoryPath(const std::filesystem::path& target)
{
    const std::filesystem::path normalized = normalizeTargetDirectoryPath(target);
    std::filesystem::path tempName = normalized.filename();
    tempName += ".tmp";
    return normalized.parent_path() / tempName;
}

std::filesystem::path makeSiblingBackupDirectoryPath(const std::filesystem::path& target)
{
    const std::filesystem::path normalized = normalizeTargetDirectoryPath(target);
    std::filesystem::path backupName = normalized.filename();
    backupName += ".backup";
    return normalized.parent_path() / backupName;
}

bool commitTempDirectory(
    const std::filesystem::path& temp,
    const std::filesystem::path& target,
    tsrs::detail::DetailPackageWriteResult& result)
{
    std::error_code error;
    const std::filesystem::path backup = makeSiblingBackupDirectoryPath(target);
    const bool hadTarget = std::filesystem::exists(target, error);
    if (error) {
        result.diagnostics.push_back(makeDiagnostic(
            kDetailDiagnosticWriteFailed,
            "error",
            {},
            -1,
            "Failed to inspect previous Detail output directory: " + error.message()));
        return false;
    }

    if (hadTarget) {
        if (std::filesystem::exists(backup, error)) {
            result.diagnostics.push_back(makeDiagnostic(
                kDetailDiagnosticWriteFailed,
                "error",
                {},
                -1,
                "Refusing to commit Detail output because backup directory already exists."));
            return false;
        }
        if (error) {
            result.diagnostics.push_back(makeDiagnostic(
                kDetailDiagnosticWriteFailed,
                "error",
                {},
                -1,
                "Failed to inspect Detail backup directory: " + error.message()));
            return false;
        }

        std::filesystem::rename(target, backup, error);
        if (error) {
            result.diagnostics.push_back(makeDiagnostic(
                kDetailDiagnosticWriteFailed,
                "error",
                {},
                -1,
                "Failed to backup previous Detail output directory: " + error.message()));
            return false;
        }
    }

    std::filesystem::rename(temp, target, error);
    if (error) {
        if (hadTarget) {
            std::error_code restoreError;
            std::filesystem::rename(backup, target, restoreError);
            if (restoreError) {
                result.diagnostics.push_back(makeDiagnostic(
                    kDetailDiagnosticWriteFailed,
                    "error",
                    {},
                    -1,
                    "Failed to restore previous Detail output directory after commit failure: "
                        + restoreError.message()));
            }
        }
        result.diagnostics.push_back(makeDiagnostic(
            kDetailDiagnosticWriteFailed,
            "error",
            {},
            -1,
            "Failed to commit Detail output directory: " + error.message()));
        return false;
    }

    if (hadTarget) {
        std::filesystem::remove_all(backup, error);
        if (error) {
            result.diagnostics.push_back(makeDiagnostic(
                kDetailDiagnosticWriteFailed,
                "error",
                {},
                -1,
                "Failed to remove Detail backup directory after commit: " + error.message()));
            return false;
        }
    }
    return true;
}

std::string xmlEscaped(const std::string& value)
{
    std::string escaped;
    escaped.reserve(value.size());
    for (const char ch : value) {
        switch (ch) {
        case '&':
            escaped += "&amp;";
            break;
        case '<':
            escaped += "&lt;";
            break;
        case '>':
            escaped += "&gt;";
            break;
        case '"':
            escaped += "&quot;";
            break;
        case '\'':
            escaped += "&apos;";
            break;
        default:
            escaped += ch;
            break;
        }
    }
    return escaped;
}

std::string formatDetailNumber(double value)
{
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream.precision(15);
    stream << value;
    return stream.str();
}

bool isFiniteLine(const tsrs::detail::DetailSectionLine2d& line)
{
    return std::isfinite(line.startX)
        && std::isfinite(line.startY)
        && std::isfinite(line.endX)
        && std::isfinite(line.endY);
}

std::string minimalStyleXml()
{
    return "<StyleRoot/>\n";
}

std::string minimalSheetXml(const tsrs::detail::DetailMinimalSectionLinePackage& package)
{
    const tsrs::detail::DetailSectionLine2d& line = package.sectionLine;
    std::ostringstream stream;
    stream
        << "<DrawingRoot>\n"
        << "  <HViewPorts>\n"
        << "    <ViewPort>\n"
        << "      <PartDetailDrawing num=\"1\">\n"
        << "        <General-Info DrawingName=\"" << xmlEscaped(package.drawingName)
        << "\" DrawingUnit=\"mm\" DrawingScale=\"1\" GeneralScale=\"1\"/>\n"
        << "        <continue-line><lines/></continue-line>\n"
        << "        <hidden-line><lines/></hidden-line>\n"
        << "        <central-line><lines/></central-line>\n"
        << "        <section-line>\n"
        << "          <lines>\n"
        << "            <Line1 start_x=\"" << formatDetailNumber(line.startX)
        << "\" start_y=\"" << formatDetailNumber(line.startY)
        << "\" end_x=\"" << formatDetailNumber(line.endX)
        << "\" end_y=\"" << formatDetailNumber(line.endY)
        << "\" ZValue=\"0 0 0\"/>\n"
        << "          </lines>\n"
        << "        </section-line>\n"
        << "        <hatch-line><lines/></hatch-line>\n"
        << "        <Others/>\n"
        << "        <steeljoint-line><lines/></steeljoint-line>\n"
        << "      </PartDetailDrawing>\n"
        << "      <StbDetailDrawing>\n"
        << "        <StbGroups stbGroupCount=\"0\"/>\n"
        << "      </StbDetailDrawing>\n"
        << "    </ViewPort>\n"
        << "  </HViewPorts>\n"
        << "</DrawingRoot>\n";
    return stream.str();
}

bool isSafePackageFileName(const std::string& fileName)
{
    const std::filesystem::path path(fileName);
    return !fileName.empty()
        && !path.is_absolute()
        && path.filename().string() == fileName;
}

bool sameKnownSummary(
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

std::string knownSummaryDigest(const tsrs::detail::DetailKnownSummary& summary)
{
    std::ostringstream stream;
    stream
        << "nodes=" << summary.xmlNodeCount
        << ",viewPorts=" << summary.viewPortCount
        << ",partDrawings=" << summary.partDetailDrawingCount
        << ",stbTables=" << summary.stbTableCount
        << ",stbRows=" << summary.stbRowCount
        << ",materialTables=" << summary.materialTableCount
        << ",matRows=" << summary.matRowCount
        << ",sectionLines=" << summary.sectionLineCount
        << ",stbGroups=" << summary.stbGroupElementCount
        << ",stbGroupsContainers=" << summary.stbGroupsContainerCount
        << ",stbGroupEntries=" << summary.stbGroupEntryCount
        << ",stds=" << summary.stdElementCount
        << ",stbGeos=" << summary.stbGeoElementCount
        << ",faceEdges=" << summary.faceEdgeCount;
    return stream.str();
}

const tsrs::detail::DetailFileSnapshot* findFile(
    const tsrs::detail::DetailPackageSnapshot& package,
    const std::string& fileName)
{
    const auto it = std::find_if(
        package.files.begin(),
        package.files.end(),
        [&fileName](const tsrs::detail::DetailFileSnapshot& file) {
            return file.fileName == fileName;
        });
    return it == package.files.end() ? nullptr : &(*it);
}

} // namespace

namespace tsrs::detail {

bool DetailPackageWriteResult::ok() const
{
    return std::none_of(diagnostics.begin(), diagnostics.end(), isErrorDiagnostic);
}

DetailPackageWriteResult DetailPackageWriter::writePreserveMode(
    const DetailPackageSnapshot& package,
    const std::string& targetDirectory) const
{
    DetailPackageWriteResult result;
    result.targetDirectory = targetDirectory;

    if (!package.ok()) {
        result.diagnostics.push_back(makeDiagnostic(
            kDetailDiagnosticWriteFailed,
            "error",
            {},
            -1,
            "Refusing to write a Detail package snapshot with error diagnostics."));
        appendDiagnostics(result, package);
        return result;
    }

    const std::filesystem::path target = normalizeTargetDirectoryPath(targetDirectory);
    const std::filesystem::path temp = makeSiblingTempDirectoryPath(target);
    if (!recreateDirectory(temp, result, "temporary")) {
        return result;
    }

    int filesWritten = 0;
    for (const DetailFileSnapshot& file : package.files) {
        if (!isSafePackageFileName(file.fileName)) {
            result.diagnostics.push_back(makeDiagnostic(
                kDetailDiagnosticWriteFailed,
                "error",
                file.fileName,
                file.sheetIndex,
                "Detail snapshot fileName must be a basename inside target directory."));
            continue;
        }
        if (file.rawXml.empty()) {
            result.diagnostics.push_back(makeDiagnostic(
                kDetailDiagnosticWriteFailed,
                "error",
                file.fileName,
                file.sheetIndex,
                "Detail snapshot has empty rawXml."));
            continue;
        }

        if (!writeRawXmlFile(temp / file.fileName, file.rawXml)) {
            result.diagnostics.push_back(makeDiagnostic(
                kDetailDiagnosticWriteFailed,
                "error",
                file.fileName,
                file.sheetIndex,
                "Failed to write Detail file: " + file.fileName));
            continue;
        }
        ++filesWritten;
    }

    if (!result.ok()) {
        std::error_code ignored;
        std::filesystem::remove_all(temp, ignored);
        return result;
    }

    const DetailPackageReader reader;
    const DetailPackageSnapshot writtenPackage = reader.readDirectory(temp.string());
    if (!writtenPackage.ok() || writtenPackage.files.size() != package.files.size()) {
        result.diagnostics.push_back(makeDiagnostic(
            kDetailDiagnosticWriteValidateFailed,
            "error",
            {},
            -1,
                "Written Detail package failed reader validation or file count changed."));
        appendDiagnostics(result, writtenPackage);
        std::error_code ignored;
        std::filesystem::remove_all(temp, ignored);
        return result;
    }

    for (const DetailFileSnapshot& before : package.files) {
        const DetailFileSnapshot* after = findFile(writtenPackage, before.fileName);
        if (!after
            || !sameKnownSummary(before.knownSummary, after->knownSummary)
            || readRawFile(temp / before.fileName) != before.rawXml
            || after->rawAttributes.size() < before.rawAttributes.size()
            || after->unknownChildren.size() < before.unknownChildren.size()) {
            result.diagnostics.push_back(makeDiagnostic(
                kDetailDiagnosticWriteValidateFailed,
                "error",
                before.fileName,
                before.sheetIndex,
                "Written Detail file did not preserve rawXml bytes, P0 summary, or unknown counters. "
                    + std::string{"before{"}
                    + knownSummaryDigest(before.knownSummary)
                    + ",rawAttrs=" + std::to_string(before.rawAttributes.size())
                    + ",unknownChildren=" + std::to_string(before.unknownChildren.size())
                    + "} after{"
                    + (after ? knownSummaryDigest(after->knownSummary) : "missing")
                    + (after ? ",rawAttrs=" + std::to_string(after->rawAttributes.size()) : "")
                    + (after ? ",unknownChildren=" + std::to_string(after->unknownChildren.size()) : "")
                    + "}"));
        }
    }

    if (!result.ok()) {
        std::error_code ignored;
        std::filesystem::remove_all(temp, ignored);
        return result;
    }

    if (!commitTempDirectory(temp, target, result)) {
        std::error_code ignored;
        std::filesystem::remove_all(temp, ignored);
        return result;
    }

    result.filesWritten = filesWritten;
    return result;
}

DetailPackageWriteResult DetailPackageWriter::writeMinimalSectionLinePackage(
    const DetailMinimalSectionLinePackage& package,
    const std::string& targetDirectory) const
{
    DetailPackageWriteResult result;
    result.targetDirectory = targetDirectory;

    if (!isFiniteLine(package.sectionLine)) {
        result.diagnostics.push_back(makeDiagnostic(
            kDetailDiagnosticWriteFailed,
            "error",
            "Detail01.stl",
            1,
            "Minimal section-line coordinates must be finite numbers."));
        return result;
    }

    const std::filesystem::path target = normalizeTargetDirectoryPath(targetDirectory);
    const std::filesystem::path temp = makeSiblingTempDirectoryPath(target);
    if (!recreateDirectory(temp, result, "temporary")) {
        return result;
    }

    const std::string styleXml = minimalStyleXml();
    const std::string sheetXml = minimalSheetXml(package);
    int filesWritten = 0;
    if (!writeRawXmlFile(temp / "Detail.xml", styleXml)) {
        result.diagnostics.push_back(makeDiagnostic(
            kDetailDiagnosticWriteFailed,
            "error",
            "Detail.xml",
            -1,
            "Failed to write minimal Detail.xml."));
        std::error_code ignored;
        std::filesystem::remove_all(temp, ignored);
        return result;
    }
    ++filesWritten;

    if (!writeRawXmlFile(temp / "Detail01.stl", sheetXml)) {
        result.diagnostics.push_back(makeDiagnostic(
            kDetailDiagnosticWriteFailed,
            "error",
            "Detail01.stl",
            1,
            "Failed to write minimal Detail01.stl."));
        std::error_code ignored;
        std::filesystem::remove_all(temp, ignored);
        return result;
    }
    ++filesWritten;

    const DetailPackageReader reader;
    const DetailPackageSnapshot writtenPackage = reader.readDirectory(temp.string());
    const DetailFileSnapshot* sheet = findFile(writtenPackage, "Detail01.stl");
    if (!writtenPackage.ok()
        || writtenPackage.files.size() != 2
        || !sheet
        || sheet->knownSummary.viewPortCount != 1
        || sheet->knownSummary.partDetailDrawingCount != 1
        || sheet->knownSummary.sectionLineCount != 1) {
        result.diagnostics.push_back(makeDiagnostic(
            kDetailDiagnosticWriteValidateFailed,
            "error",
            "Detail01.stl",
            1,
            "Generated minimal Detail package failed reader validation."));
        appendDiagnostics(result, writtenPackage);
    }

    if (!result.ok()) {
        std::error_code ignored;
        std::filesystem::remove_all(temp, ignored);
        return result;
    }

    if (!commitTempDirectory(temp, target, result)) {
        std::error_code ignored;
        std::filesystem::remove_all(temp, ignored);
        return result;
    }

    result.filesWritten = filesWritten;
    return result;
}

} // namespace tsrs::detail
