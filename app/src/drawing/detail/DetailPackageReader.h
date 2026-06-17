#pragma once

#include <string>
#include <vector>

namespace tsrs::detail {

inline constexpr const char* kDetailDiagnosticPackageMissing = "DETAIL_PACKAGE_MISSING";
inline constexpr const char* kDetailDiagnosticStyleMissing = "DETAIL_STYLE_MISSING";
inline constexpr const char* kDetailDiagnosticSheetMissing = "DETAIL_SHEET_MISSING";
inline constexpr const char* kDetailDiagnosticXmlParseFailed = "DETAIL_XML_PARSE_FAILED";
inline constexpr const char* kDetailDiagnosticRootUnexpected = "DETAIL_ROOT_UNEXPECTED";
inline constexpr const char* kDetailDiagnosticSheetIndexGap = "DETAIL_SHEET_INDEX_GAP";
inline constexpr const char* kDetailDiagnosticWriteFailed = "DETAIL_WRITE_FAILED";
inline constexpr const char* kDetailDiagnosticWriteValidateFailed = "DETAIL_WRITE_VALIDATE_FAILED";

struct DetailDiagnostic {
    std::string code;
    std::string severity;
    std::string fileName;
    int sheetIndex{-1};
    int line{-1};
    int column{-1};
    std::string nodePath;
    std::string message;
};

struct DetailRawAttribute {
    std::string nodePath;
    std::string name;
    std::string value;
    std::string namespaceUri;
    std::string qualifiedName;
};

struct DetailUnknownChild {
    std::string nodePath;
    std::string name;
    std::string namespaceUri;
};

struct DetailKnownSummary {
    int xmlNodeCount{0};
    int viewPortCount{0};
    int partDetailDrawingCount{0};
    int stbTableCount{0};
    int stbRowCount{0};
    int materialTableCount{0};
    int matRowCount{0};
    int sectionLineCount{0};
    int stbGroupElementCount{0};
    int stbGroupsContainerCount{0};
    int stbGroupEntryCount{0};
    int stdElementCount{0};
    int stbGeoElementCount{0};
    int faceEdgeCount{0};
};

struct DetailFileSnapshot {
    std::string fileName;
    int sheetIndex{-1};
    std::string rootName;
    std::string rawXml;
    DetailKnownSummary knownSummary;
    std::vector<DetailRawAttribute> rawAttributes;
    std::vector<DetailUnknownChild> unknownChildren;
    std::vector<DetailDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const;
};

struct DetailPackageSnapshot {
    std::string sourceDirectory;
    std::vector<DetailFileSnapshot> files;
    std::vector<DetailDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const;
};

class DetailPackageReader final {
public:
    [[nodiscard]] DetailPackageSnapshot readDirectory(
        const std::string& directory) const;
};

} // namespace tsrs::detail
