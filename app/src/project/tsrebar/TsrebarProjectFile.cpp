#include "project/tsrebar/TsrebarProjectFile.h"

#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QString>

#include <fstream>
#include <iterator>
#include <utility>

namespace tsrs::project::tsrebar {

namespace {

TsrebarProjectWriteResult writeOk()
{
    return {true, kTsrebarDiagnosticOk, {}};
}

TsrebarProjectWriteResult writeFail(const char* code, std::string diagnostic)
{
    return {false, code, std::move(diagnostic)};
}

TsrebarProjectReadResult readFail(const char* code, std::string diagnostic)
{
    TsrebarProjectReadResult result;
    result.ok = false;
    result.diagnosticCode = code;
    result.diagnostic = std::move(diagnostic);
    return result;
}

std::string toStdString(const QJsonValue& value)
{
    return value.toString().toStdString();
}

QJsonArray toJsonStringArray(const std::vector<std::string>& values)
{
    QJsonArray array;
    for (const std::string& value : values) {
        array.append(QString::fromStdString(value));
    }
    return array;
}

std::vector<std::string> readStringArray(const QJsonArray& array)
{
    std::vector<std::string> values;
    values.reserve(static_cast<std::size_t>(array.size()));
    for (const QJsonValue& value : array) {
        values.push_back(value.toString().toStdString());
    }
    return values;
}

bool isStringMember(const QJsonObject& object, const char* key)
{
    return object.contains(key) && object.value(key).isString();
}

bool isNumberMember(const QJsonObject& object, const char* key)
{
    return object.contains(key) && object.value(key).isDouble();
}

bool isIntegerMember(const QJsonObject& object, const char* key)
{
    const QJsonValue value = object.value(key);
    return object.contains(key) && value.isDouble()
        && value.toDouble() == static_cast<double>(value.toInt());
}

bool isStringArrayValue(const QJsonValue& value)
{
    if (!value.isArray()) {
        return false;
    }
    const QJsonArray array = value.toArray();
    for (const QJsonValue& item : array) {
        if (!item.isString()) {
            return false;
        }
    }
    return true;
}

bool isValidProjectObject(const QJsonObject& project)
{
    return isStringMember(project, "projectId")
        && isStringMember(project, "sourceStepPath")
        && isStringMember(project, "sourceStepSha256")
        && isStringMember(project, "sourceLengthUnit")
        && isNumberMember(project, "sourceToMeterScale");
}

bool isValidGroupObject(const QJsonObject& group)
{
    return isStringMember(group, "groupId")
        && isStringMember(group, "commandId")
        && isStringMember(group, "styleName")
        && isStringMember(group, "grade")
        && isNumberMember(group, "diameterM")
        && isIntegerMember(group, "bundleCount")
        && group.contains("centerlineStableIds")
        && isStringArrayValue(group.value("centerlineStableIds"));
}

bool isValidRebarGroupsValue(const QJsonValue& value)
{
    if (!value.isArray()) {
        return false;
    }
    const QJsonArray groups = value.toArray();
    for (const QJsonValue& groupValue : groups) {
        if (!groupValue.isObject()
            || !isValidGroupObject(groupValue.toObject())) {
            return false;
        }
    }
    return true;
}

QJsonObject toJsonGroup(const tsrs::domain::rebar::RebarGroupDraft& group)
{
    QJsonObject object;
    object.insert("groupId", QString::fromStdString(group.groupId));
    object.insert("commandId", QString::fromStdString(group.commandId));
    object.insert("styleName", QString::fromStdString(group.styleName));
    object.insert("grade", QString::fromStdString(group.grade));
    object.insert("diameterM", group.diameterM);
    object.insert("bundleCount", group.bundleCount);
    object.insert("centerlineStableIds",
                  toJsonStringArray(group.centerlineStableIds));
    return object;
}

tsrs::domain::rebar::RebarGroupDraft readGroup(const QJsonObject& object)
{
    tsrs::domain::rebar::RebarGroupDraft group;
    group.groupId = toStdString(object.value("groupId"));
    group.commandId = toStdString(object.value("commandId"));
    group.styleName = toStdString(object.value("styleName"));
    group.grade = toStdString(object.value("grade"));
    group.diameterM = object.value("diameterM").toDouble();
    group.bundleCount = object.value("bundleCount").toInt(1);
    group.centerlineStableIds =
        readStringArray(object.value("centerlineStableIds").toArray());
    return group;
}

QJsonObject toJsonDocument(const TsrebarProjectDocument& document)
{
    QJsonObject root;
    root.insert("format", "tsrebar");
    root.insert("formatVersion", kTsrebarFormatVersion);

    QJsonObject project;
    project.insert("projectId", QString::fromStdString(document.projectId));
    project.insert("sourceStepPath",
                   QString::fromStdString(document.sourceStepPath));
    project.insert("sourceStepSha256",
                   QString::fromStdString(document.sourceStepSha256));
    project.insert("sourceLengthUnit",
                   QString::fromStdString(document.sourceLengthUnit));
    project.insert("sourceToMeterScale", document.sourceToMeterScale);
    root.insert("project", project);

    QJsonArray groups;
    for (const auto& group : document.rebarGroups) {
        groups.append(toJsonGroup(group));
    }
    root.insert("rebarGroups", groups);
    return root;
}

TsrebarProjectDocument readDocument(const QJsonObject& root)
{
    TsrebarProjectDocument document;
    const QJsonObject project = root.value("project").toObject();
    document.projectId = toStdString(project.value("projectId"));
    document.sourceStepPath = toStdString(project.value("sourceStepPath"));
    document.sourceStepSha256 = toStdString(project.value("sourceStepSha256"));
    document.sourceLengthUnit = toStdString(project.value("sourceLengthUnit"));
    document.sourceToMeterScale =
        project.value("sourceToMeterScale").toDouble(0.001);

    const QJsonArray groups = root.value("rebarGroups").toArray();
    document.rebarGroups.reserve(static_cast<std::size_t>(groups.size()));
    for (const QJsonValue& value : groups) {
        document.rebarGroups.push_back(readGroup(value.toObject()));
    }
    return document;
}

} // namespace

TsrebarProjectWriteResult writeTsrebarProjectFile(
    const std::filesystem::path& path,
    const TsrebarProjectDocument& document)
{
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) {
        return writeFail(kTsrebarDiagnosticIoError,
                         "Unable to open .tsrebar file for writing.");
    }

    const QJsonDocument json(toJsonDocument(document));
    const QByteArray bytes = json.toJson(QJsonDocument::Indented);
    output.write(bytes.constData(), bytes.size());
    if (!output) {
        return writeFail(kTsrebarDiagnosticIoError,
                         "Unable to write .tsrebar JSON payload.");
    }
    return writeOk();
}

TsrebarProjectReadResult readTsrebarProjectFile(
    const std::filesystem::path& path)
{
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return readFail(kTsrebarDiagnosticIoError,
                        "Unable to open .tsrebar file for reading.");
    }

    const std::string raw{std::istreambuf_iterator<char>(input),
                          std::istreambuf_iterator<char>()};

    QJsonParseError error;
    const QJsonDocument json =
        QJsonDocument::fromJson(QByteArray::fromStdString(raw), &error);
    if (error.error != QJsonParseError::NoError || !json.isObject()) {
        return readFail(kTsrebarDiagnosticInvalidFormat,
                        "Invalid .tsrebar JSON document.");
    }

    const QJsonObject root = json.object();
    if (root.value("format").toString() != "tsrebar") {
        return readFail(kTsrebarDiagnosticInvalidFormat,
                        "Unsupported .tsrebar format marker.");
    }
    if (root.value("formatVersion").toInt() != kTsrebarFormatVersion) {
        return readFail(kTsrebarDiagnosticUnsupportedVersion,
                        "Unsupported .tsrebar format version.");
    }
    if (!root.contains("project") || !root.value("project").isObject()
        || !isValidProjectObject(root.value("project").toObject())) {
        return readFail(kTsrebarDiagnosticInvalidFormat,
                        "Invalid .tsrebar project object.");
    }
    if (!root.contains("rebarGroups")
        || !isValidRebarGroupsValue(root.value("rebarGroups"))) {
        return readFail(kTsrebarDiagnosticInvalidFormat,
                        "Invalid .tsrebar rebarGroups array.");
    }

    TsrebarProjectReadResult result;
    result.ok = true;
    result.diagnosticCode = kTsrebarDiagnosticOk;
    result.document = readDocument(root);
    return result;
}

} // namespace tsrs::project::tsrebar
