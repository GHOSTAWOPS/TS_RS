#pragma once

#include "domain/rebar/RebarModel.h"

#include <filesystem>
#include <string>
#include <vector>

namespace tsrs::project::tsrebar {

inline constexpr int kTsrebarFormatVersion = 1;
inline constexpr const char* kTsrebarDiagnosticOk = "TSREBAR_OK";
inline constexpr const char* kTsrebarDiagnosticIoError = "TSREBAR_IO_ERROR";
inline constexpr const char* kTsrebarDiagnosticInvalidFormat =
    "TSREBAR_INVALID_FORMAT";
inline constexpr const char* kTsrebarDiagnosticUnsupportedVersion =
    "TSREBAR_UNSUPPORTED_VERSION";

struct TsrebarProjectDocument {
    std::string projectId;
    std::string sourceStepPath;
    std::string sourceStepSha256;
    std::string sourceLengthUnit{"mm"};
    double sourceToMeterScale{0.001};
    std::vector<tsrs::domain::rebar::RebarGroupDraft> rebarGroups;
};

struct TsrebarProjectWriteResult {
    bool ok{false};
    std::string diagnosticCode{kTsrebarDiagnosticIoError};
    std::string diagnostic;
};

struct TsrebarProjectReadResult {
    bool ok{false};
    std::string diagnosticCode{kTsrebarDiagnosticInvalidFormat};
    std::string diagnostic;
    TsrebarProjectDocument document;
};

[[nodiscard]] TsrebarProjectWriteResult writeTsrebarProjectFile(
    const std::filesystem::path& path,
    const TsrebarProjectDocument& document);

[[nodiscard]] TsrebarProjectReadResult readTsrebarProjectFile(
    const std::filesystem::path& path);

} // namespace tsrs::project::tsrebar
