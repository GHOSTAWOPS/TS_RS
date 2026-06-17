#pragma once

#include "drawing/detail/DetailPackageReader.h"

#include <string>
#include <vector>

namespace tsrs::detail {

struct DetailPackageWriteResult {
    std::string targetDirectory;
    int filesWritten{0};
    std::vector<DetailDiagnostic> diagnostics;

    [[nodiscard]] bool ok() const;
};

struct DetailSectionLine2d {
    double startX{0.0};
    double startY{0.0};
    double endX{100.0};
    double endY{0.0};
};

struct DetailMinimalSectionLinePackage {
    std::string drawingName{"TS_RS minimal Detail"};
    DetailSectionLine2d sectionLine;
};

class DetailPackageWriter final {
public:
    [[nodiscard]] DetailPackageWriteResult writePreserveMode(
        const DetailPackageSnapshot& package,
        const std::string& targetDirectory) const;

    [[nodiscard]] DetailPackageWriteResult writeMinimalSectionLinePackage(
        const DetailMinimalSectionLinePackage& package,
        const std::string& targetDirectory) const;
};

} // namespace tsrs::detail
