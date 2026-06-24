#include "project/tsrebar/TsrebarProjectFile.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <cmath>
#include <string>

namespace {

int fail(const std::string& message)
{
    std::cerr << message << '\n';
    return 1;
}

tsrs::domain::rebar::RebarGroupDraft makeGroup(
    std::string groupId,
    std::string commandId,
    std::string firstCenterline)
{
    tsrs::domain::rebar::RebarGroupDraft group;
    group.groupId = std::move(groupId);
    group.commandId = std::move(commandId);
    group.styleName = "D28";
    group.grade = "HRB400";
    group.diameterM = 0.028;
    group.bundleCount = 2;
    group.centerlineStableIds = {std::move(firstCenterline), "centerline-b"};
    return group;
}

tsrs::project::tsrebar::TsrebarProjectDocument makeDocument()
{
    tsrs::project::tsrebar::TsrebarProjectDocument document;
    document.projectId = "p0a-demo";
    document.sourceStepPath = "samples/123.stp";
    document.sourceStepSha256 = "sha256-demo";
    document.sourceLengthUnit = "mm";
    document.sourceToMeterScale = 0.001;
    document.rebarGroups = {
        makeGroup("group-001", "fix-distance", "edge:stable:001"),
        makeGroup("group-002", "fix-number", "edge:stable:002"),
    };
    return document;
}

std::filesystem::path tempFilePath(const std::string& name)
{
    const auto path = std::filesystem::temp_directory_path()
        / ("tsrs_" + name + ".tsrebar");
    std::filesystem::remove(path);
    return path;
}

std::string readText(const std::filesystem::path& path)
{
    std::ifstream input(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(input),
            std::istreambuf_iterator<char>()};
}

void writeText(const std::filesystem::path& path, const std::string& text)
{
    std::ofstream output(path, std::ios::binary);
    output << text;
}

bool nearlyEqual(double lhs, double rhs)
{
    return std::fabs(lhs - rhs) < 1e-12;
}

bool sameStringVector(const std::vector<std::string>& lhs,
                      const std::vector<std::string>& rhs)
{
    return lhs == rhs;
}

int expectGroupEquals(
    const tsrs::domain::rebar::RebarGroupDraft& actual,
    const tsrs::domain::rebar::RebarGroupDraft& expected,
    const std::string& label)
{
    if (actual.groupId != expected.groupId) {
        return fail(label + " groupId did not round-trip");
    }
    if (actual.commandId != expected.commandId) {
        return fail(label + " commandId did not round-trip");
    }
    if (actual.styleName != expected.styleName) {
        return fail(label + " styleName did not round-trip");
    }
    if (actual.grade != expected.grade) {
        return fail(label + " grade did not round-trip");
    }
    if (!nearlyEqual(actual.diameterM, expected.diameterM)) {
        return fail(label + " diameterM did not round-trip");
    }
    if (actual.bundleCount != expected.bundleCount) {
        return fail(label + " bundleCount did not round-trip");
    }
    if (!sameStringVector(actual.centerlineStableIds,
                          expected.centerlineStableIds)) {
        return fail(label + " centerlineStableIds did not round-trip");
    }
    return 0;
}

int expectRoundTripPreservesMinimalProjectState()
{
    const auto path = tempFilePath("minimal_roundtrip");
    const auto document = makeDocument();

    const auto write =
        tsrs::project::tsrebar::writeTsrebarProjectFile(path, document);
    if (!write.ok) {
        return fail("writeTsrebarProjectFile must accept minimal document: "
                    + write.diagnosticCode);
    }

    const std::string raw = readText(path);
    if (raw.find("\"format\"") == std::string::npos
        || raw.find("\"tsrebar\"") == std::string::npos) {
        return fail("written .tsrebar must declare format");
    }
    if (raw.find("TopoDS_") != std::string::npos
        || raw.find("AIS_") != std::string::npos
        || raw.find("Detail") != std::string::npos) {
        return fail(".tsrebar P0 must not persist OCCT/AIS/Detail objects");
    }

    const auto read = tsrs::project::tsrebar::readTsrebarProjectFile(path);
    if (!read.ok) {
        return fail("readTsrebarProjectFile must read what writer produced: "
                    + read.diagnosticCode);
    }
    if (read.document.projectId != document.projectId
        || read.document.sourceStepPath != document.sourceStepPath
        || read.document.sourceStepSha256 != document.sourceStepSha256
        || read.document.sourceLengthUnit != document.sourceLengthUnit
        || read.document.sourceToMeterScale != document.sourceToMeterScale) {
        return fail("project summary did not round-trip");
    }
    if (read.document.rebarGroups.size() != document.rebarGroups.size()) {
        return fail("rebar group count did not round-trip");
    }
    if (const int code = expectGroupEquals(read.document.rebarGroups[0],
                                           document.rebarGroups[0],
                                           "first rebar group")) {
        return code;
    }
    if (const int code = expectGroupEquals(read.document.rebarGroups[1],
                                           document.rebarGroups[1],
                                           "second rebar group")) {
        return code;
    }

    std::filesystem::remove(path);
    return 0;
}

int expectUnsupportedVersionIsRejected()
{
    const auto path = tempFilePath("unsupported_version");
    writeText(path,
              "{"
              "\"format\":\"tsrebar\","
              "\"formatVersion\":999,"
              "\"project\":{},"
              "\"rebarGroups\":[]"
              "}");

    const auto read = tsrs::project::tsrebar::readTsrebarProjectFile(path);
    std::filesystem::remove(path);

    if (read.ok || read.diagnosticCode
                       != tsrs::project::tsrebar::kTsrebarDiagnosticUnsupportedVersion) {
        return fail("unsupported format version must be rejected");
    }
    return 0;
}

int expectMalformedJsonIsRejected()
{
    const auto path = tempFilePath("malformed");
    writeText(path, "{not valid json");

    const auto read = tsrs::project::tsrebar::readTsrebarProjectFile(path);
    std::filesystem::remove(path);

    if (read.ok || read.diagnosticCode
                       != tsrs::project::tsrebar::kTsrebarDiagnosticInvalidFormat) {
        return fail("malformed JSON must be rejected");
    }
    return 0;
}

int expectInvalidProjectShapeIsRejected()
{
    const auto path = tempFilePath("missing_project");
    writeText(path,
              "{"
              "\"format\":\"tsrebar\","
              "\"formatVersion\":1,"
              "\"rebarGroups\":[]"
              "}");

    const auto read = tsrs::project::tsrebar::readTsrebarProjectFile(path);
    std::filesystem::remove(path);

    if (read.ok || read.diagnosticCode
                       != tsrs::project::tsrebar::kTsrebarDiagnosticInvalidFormat) {
        return fail("missing project object must be rejected");
    }
    return 0;
}

int expectInvalidRebarGroupsShapeIsRejected()
{
    const auto path = tempFilePath("invalid_rebar_groups");
    writeText(path,
              "{"
              "\"format\":\"tsrebar\","
              "\"formatVersion\":1,"
              "\"project\":{"
              "\"projectId\":\"p\","
              "\"sourceStepPath\":\"a.stp\","
              "\"sourceStepSha256\":\"sha\","
              "\"sourceLengthUnit\":\"mm\","
              "\"sourceToMeterScale\":0.001"
              "},"
              "\"rebarGroups\":{}"
              "}");

    const auto read = tsrs::project::tsrebar::readTsrebarProjectFile(path);
    std::filesystem::remove(path);

    if (read.ok || read.diagnosticCode
                       != tsrs::project::tsrebar::kTsrebarDiagnosticInvalidFormat) {
        return fail("non-array rebarGroups must be rejected");
    }
    return 0;
}

int expectInvalidRebarGroupFieldTypeIsRejected()
{
    const auto path = tempFilePath("invalid_rebar_group_field");
    writeText(path,
              "{"
              "\"format\":\"tsrebar\","
              "\"formatVersion\":1,"
              "\"project\":{"
              "\"projectId\":\"p\","
              "\"sourceStepPath\":\"a.stp\","
              "\"sourceStepSha256\":\"sha\","
              "\"sourceLengthUnit\":\"mm\","
              "\"sourceToMeterScale\":0.001"
              "},"
              "\"rebarGroups\":[{"
              "\"groupId\":\"group-001\","
              "\"commandId\":\"fix-distance\","
              "\"styleName\":\"D28\","
              "\"grade\":\"HRB400\","
              "\"diameterM\":\"0.028\","
              "\"bundleCount\":2,"
              "\"centerlineStableIds\":[\"edge:stable:001\"]"
              "}]"
              "}");

    const auto read = tsrs::project::tsrebar::readTsrebarProjectFile(path);
    std::filesystem::remove(path);

    if (read.ok || read.diagnosticCode
                       != tsrs::project::tsrebar::kTsrebarDiagnosticInvalidFormat) {
        return fail("wrong rebar group field type must be rejected");
    }
    return 0;
}

} // namespace

int main()
{
    if (const int code = expectRoundTripPreservesMinimalProjectState()) {
        return code;
    }
    if (const int code = expectUnsupportedVersionIsRejected()) {
        return code;
    }
    if (const int code = expectMalformedJsonIsRejected()) {
        return code;
    }
    if (const int code = expectInvalidProjectShapeIsRejected()) {
        return code;
    }
    if (const int code = expectInvalidRebarGroupsShapeIsRejected()) {
        return code;
    }
    if (const int code = expectInvalidRebarGroupFieldTypeIsRejected()) {
        return code;
    }
    return 0;
}
