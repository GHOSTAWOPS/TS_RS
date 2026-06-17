#include "drawing/detail/DetailPackageWriter.h"

#include <QFile>
#include <QString>

#include <algorithm>
#include <filesystem>
#include <sstream>
#include <string>

namespace {

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

    const std::filesystem::path target(targetDirectory);
    std::error_code error;
    std::filesystem::create_directories(target, error);
    if (error) {
        result.diagnostics.push_back(makeDiagnostic(
            kDetailDiagnosticWriteFailed,
            "error",
            {},
            -1,
            "Failed to create Detail output directory: " + error.message()));
        return result;
    }

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

        if (!writeRawXmlFile(target / file.fileName, file.rawXml)) {
            result.diagnostics.push_back(makeDiagnostic(
                kDetailDiagnosticWriteFailed,
                "error",
                file.fileName,
                file.sheetIndex,
                "Failed to write Detail file: " + file.fileName));
            continue;
        }
        ++result.filesWritten;
    }

    if (!result.ok()) {
        return result;
    }

    const DetailPackageReader reader;
    const DetailPackageSnapshot writtenPackage = reader.readDirectory(targetDirectory);
    if (!writtenPackage.ok() || writtenPackage.files.size() != package.files.size()) {
        result.diagnostics.push_back(makeDiagnostic(
            kDetailDiagnosticWriteValidateFailed,
            "error",
            {},
            -1,
            "Written Detail package failed reader validation or file count changed."));
        for (const DetailDiagnostic& diagnostic : writtenPackage.diagnostics) {
            result.diagnostics.push_back(diagnostic);
        }
        for (const DetailFileSnapshot& file : writtenPackage.files) {
            result.diagnostics.insert(
                result.diagnostics.end(),
                file.diagnostics.begin(),
                file.diagnostics.end());
        }
        return result;
    }

    for (const DetailFileSnapshot& before : package.files) {
        const DetailFileSnapshot* after = findFile(writtenPackage, before.fileName);
        if (!after
            || !sameKnownSummary(before.knownSummary, after->knownSummary)
            || after->rawAttributes.size() < before.rawAttributes.size()
            || after->unknownChildren.size() < before.unknownChildren.size()) {
            result.diagnostics.push_back(makeDiagnostic(
                kDetailDiagnosticWriteValidateFailed,
                "error",
                before.fileName,
                before.sheetIndex,
                "Written Detail file did not preserve P0 summary or unknown counters. "
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

    return result;
}

} // namespace tsrs::detail
