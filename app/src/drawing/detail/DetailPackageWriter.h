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

class DetailPackageWriter final {
public:
    [[nodiscard]] DetailPackageWriteResult writePreserveMode(
        const DetailPackageSnapshot& package,
        const std::string& targetDirectory) const;
};

} // namespace tsrs::detail
