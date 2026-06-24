#include "application/commands/RebarCreationCommandService.h"
#include "application/rebarsmart/RebarCreationCommandServiceFactory.h"
#include "domain/rebar/RebarModel.h"
#include "geometry/kernel/IGeometryEngine.h"

#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace {

int fail(const std::string& message)
{
    std::cerr << message << '\n';
    return 1;
}

class MockGeometryEngine final : public tsrs::geometry::IGeometryEngine {
public:
    void addStraightCurve(std::string stableId,
                          tsrs::geometry::GeometryPoint3d start,
                          tsrs::geometry::GeometryPoint3d end)
    {
        curves_.emplace(std::move(stableId), CurveRecord{start, end});
    }

    tsrs::geometry::GeometryLengthResult curveLength(
        const tsrs::geometry::GeometryRef& curveRef) const override
    {
        if (curveRef.kind != tsrs::geometry::GeometryEntityKind::Curve) {
            return {false, 0.0, std::string{tsrs::geometry::kGeometryDiagnosticWrongEntityKind}};
        }

        const auto found = curves_.find(curveRef.stableId);
        if (found == curves_.end()) {
            return {false, 0.0, std::string{tsrs::geometry::kGeometryDiagnosticMissingRef}};
        }

        return {true, distanceBetween(found->second.start, found->second.end),
                std::string{tsrs::geometry::kGeometryDiagnosticOk}};
    }

    tsrs::geometry::GeometryPointResult pointAtLength(
        const tsrs::geometry::GeometryRef& curveRef,
        double lengthM) const override
    {
        const auto length = curveLength(curveRef);
        if (!length.ok) {
            return {false, {}, length.diagnosticCode};
        }
        if (lengthM < 0.0 || lengthM > length.lengthM) {
            return {false, {}, std::string{tsrs::geometry::kGeometryDiagnosticLengthOutOfRange}};
        }

        const auto found = curves_.find(curveRef.stableId);
        const double ratio = length.lengthM == 0.0 ? 0.0 : lengthM / length.lengthM;
        return {true,
                interpolate(found->second.start, found->second.end, ratio),
                std::string{tsrs::geometry::kGeometryDiagnosticOk}};
    }

    tsrs::geometry::GeometryVectorResult tangentAtLength(
        const tsrs::geometry::GeometryRef& curveRef,
        double lengthM) const override
    {
        const auto length = curveLength(curveRef);
        if (!length.ok) {
            return {false, {}, length.diagnosticCode};
        }
        if (lengthM < 0.0 || lengthM > length.lengthM) {
            return {false, {}, std::string{tsrs::geometry::kGeometryDiagnosticLengthOutOfRange}};
        }

        const auto found = curves_.find(curveRef.stableId);
        const auto vector = vectorBetween(found->second.start, found->second.end);
        const double magnitude = vectorMagnitude(vector);
        if (magnitude == 0.0) {
            return {false, {}, std::string{tsrs::geometry::kGeometryDiagnosticInvalidInput}};
        }
        return {true,
                tsrs::geometry::GeometryVector3d{
                    vector.x / magnitude,
                    vector.y / magnitude,
                    vector.z / magnitude,
                },
                std::string{tsrs::geometry::kGeometryDiagnosticOk}};
    }

    tsrs::geometry::GeometryRefResult makePolylineCurve(
        std::vector<tsrs::geometry::GeometryPoint3d> points) override
    {
        if (points.size() < 2) {
            return {false, {}, std::string{tsrs::geometry::kGeometryDiagnosticInvalidInput}};
        }

        madePolylines_.push_back(std::move(points));
        const std::string stableId =
            "generated-centerline-" + std::to_string(madePolylines_.size());
        curves_.emplace(stableId, CurveRecord{madePolylines_.back().front(),
                                              madePolylines_.back().back()});
        return {true,
                tsrs::geometry::GeometryRef{tsrs::geometry::GeometryEntityKind::Curve,
                                            stableId},
                std::string{tsrs::geometry::kGeometryDiagnosticOk}};
    }

private:
    struct CurveRecord {
        tsrs::geometry::GeometryPoint3d start;
        tsrs::geometry::GeometryPoint3d end;
    };

    static tsrs::geometry::GeometryVector3d vectorBetween(
        tsrs::geometry::GeometryPoint3d start,
        tsrs::geometry::GeometryPoint3d end)
    {
        return {end.x - start.x, end.y - start.y, end.z - start.z};
    }

    static double vectorMagnitude(tsrs::geometry::GeometryVector3d vector)
    {
        return std::sqrt(vector.x * vector.x + vector.y * vector.y
                         + vector.z * vector.z);
    }

    static double distanceBetween(tsrs::geometry::GeometryPoint3d lhs,
                                  tsrs::geometry::GeometryPoint3d rhs)
    {
        return vectorMagnitude(vectorBetween(lhs, rhs));
    }

    static tsrs::geometry::GeometryPoint3d interpolate(
        tsrs::geometry::GeometryPoint3d start,
        tsrs::geometry::GeometryPoint3d end,
        double ratio)
    {
        return {
            start.x + (end.x - start.x) * ratio,
            start.y + (end.y - start.y) * ratio,
            start.z + (end.z - start.z) * ratio,
        };
    }

    std::map<std::string, CurveRecord> curves_;
    std::vector<std::vector<tsrs::geometry::GeometryPoint3d>> madePolylines_;
};

class FakeFixDistanceGenerator final
    : public tsrs::application::IFixDistanceCenterlineGenerator {
public:
    tsrs::application::RebarCreationGenerationResult generate(
        const tsrs::application::RebarCreationFixDistanceInput& input,
        tsrs::geometry::IGeometryEngine&) const override
    {
        called = true;
        capturedStyleName = input.styleName;
        return result;
    }

    mutable bool called{false};
    mutable std::string capturedStyleName;
    tsrs::application::RebarCreationGenerationResult result{
        true,
        {
            {tsrs::geometry::GeometryRef{
                tsrs::geometry::GeometryEntityKind::Curve,
                "fake-centerline-1"}},
            {tsrs::geometry::GeometryRef{
                tsrs::geometry::GeometryEntityKind::Curve,
                "fake-centerline-2"}},
        },
        std::string{tsrs::geometry::kGeometryDiagnosticOk},
    };
};

tsrs::geometry::GeometryRef curveRef(std::string stableId)
{
    return tsrs::geometry::GeometryRef{tsrs::geometry::GeometryEntityKind::Curve,
                                       std::move(stableId)};
}

tsrs::domain::rebar::RebarGroupDraft makeDraft(std::string groupId)
{
    tsrs::domain::rebar::RebarGroupDraft draft;
    draft.groupId = std::move(groupId);
    draft.commandId = "fix-distance";
    draft.styleName = "D25";
    draft.grade = "HRB400";
    draft.diameterM = 0.025;
    draft.bundleCount = 1;
    draft.centerlineStableIds = {"centerline-1", "centerline-2"};
    return draft;
}

tsrs::application::RebarCreationPreviewFixDistanceRequest makeFixDistanceRequest(
    MockGeometryEngine& geometry)
{
    geometry.addStraightCurve("guide-main",
                              tsrs::geometry::GeometryPoint3d{0.0, 0.0, 0.0},
                              tsrs::geometry::GeometryPoint3d{1.0, 0.0, 0.0});
    geometry.addStraightCurve("guide-auxiliary",
                              tsrs::geometry::GeometryPoint3d{0.0, 0.0, 0.0},
                              tsrs::geometry::GeometryPoint3d{0.0, 1.0, 0.0});

    tsrs::application::RebarCreationPreviewFixDistanceRequest request;
    request.commandId = "fix-distance";
    request.groupId = "group-fix-distance";
    request.geometry = &geometry;
    request.input.mainGuideCurve = curveRef("guide-main");
    request.input.auxiliaryGuideCurve = curveRef("guide-auxiliary");
    request.input.styleName = "D28";
    request.input.grade = "HRB400";
    request.input.diameterM = 0.028;
    request.input.spacingM = 0.2;
    request.input.count = 11;
    request.input.bundleCount = 2;
    request.input.priorityMode =
        tsrs::application::RebarCreationFixDistancePriorityMode::Spacing;
    request.input.spacingRatio = 1.0;
    request.input.headZoneLengthM = 0.1;
    request.input.tailZoneLengthM = 0.1;
    return request;
}

int expectServiceRequiresModel()
{
    const tsrs::application::RebarCreationCommandService service;
    const auto result =
        service.previewDraft({"cmd-missing-model", makeDraft("group-a")});

    if (result.ok) {
        return fail("previewDraft without model must fail");
    }
    if (result.diagnosticCode
        != tsrs::application::kRebarCreationCommandDiagnosticModelMissing) {
        return fail("missing model diagnostic mismatch: "
                    + result.diagnosticCode);
    }

    const auto commit = service.commitPreview();
    if (commit.ok || commit.diagnosticCode
                         != tsrs::application::kRebarCreationCommandDiagnosticModelMissing) {
        return fail("commitPreview without model must return MODEL_MISSING");
    }

    const auto cancel = service.cancelPreview();
    if (cancel.ok || cancel.diagnosticCode
                         != tsrs::application::kRebarCreationCommandDiagnosticModelMissing) {
        return fail("cancelPreview without model must return MODEL_MISSING");
    }

    return 0;
}

int expectPreviewCommitCancelUseModelTransaction()
{
    tsrs::domain::rebar::RebarModel model;
    tsrs::application::RebarCreationCommandService service(&model);

    const auto preview =
        service.previewDraft({"cmd-preview", makeDraft("group-preview")});
    if (!preview.ok) {
        return fail("previewDraft must accept valid draft: "
                    + preview.diagnosticCode);
    }
    if (!model.hasPreview()) {
        return fail("previewDraft must create RebarModel preview");
    }
    if (model.committedGroupCount() != 0 || model.isDirty()) {
        return fail("previewDraft must not commit or dirty model");
    }

    const auto cancel = service.cancelPreview();
    if (!cancel.ok) {
        return fail("cancelPreview must clear active preview: "
                    + cancel.diagnosticCode);
    }
    if (model.hasPreview() || model.committedGroupCount() != 0
        || model.isDirty()) {
        return fail("cancelPreview must leave model clean with no groups");
    }

    const auto secondPreview =
        service.previewDraft({"cmd-commit", makeDraft("group-commit")});
    if (!secondPreview.ok) {
        return fail("second previewDraft must accept valid draft");
    }

    const auto commit = service.commitPreview();
    if (!commit.ok) {
        return fail("commitPreview must commit active preview: "
                    + commit.diagnosticCode);
    }
    if (model.hasPreview()) {
        return fail("commitPreview must clear model preview");
    }
    if (model.committedGroupCount() != 1) {
        return fail("commitPreview must create committed rebar group");
    }
    if (!model.isDirty()) {
        return fail("commitPreview must mark RebarModel dirty");
    }

    return 0;
}

int expectFixDistancePreviewUsesGeneratorAndTransaction()
{
    tsrs::domain::rebar::RebarModel model;
    const tsrs::application::RebarCreationCommandService service =
        tsrs::application::RebarCreationCommandServiceFactory::create(&model);
    MockGeometryEngine geometry;

    const auto preview = service.previewFixDistance(makeFixDistanceRequest(geometry));
    if (!preview.ok) {
        return fail("previewFixDistance must generate preview: "
                    + preview.diagnosticCode);
    }
    if (!model.hasPreview() || model.committedGroupCount() != 0
        || model.isDirty()) {
        return fail("previewFixDistance must create preview without commit/dirty");
    }

    const auto commit = service.commitPreview();
    if (!commit.ok) {
        return fail("commitPreview must commit generated preview: "
                    + commit.diagnosticCode);
    }
    if (model.committedGroupCount() != 1 || !model.isDirty()) {
        return fail("committed generated preview must update RebarModel");
    }

    const auto& group = model.committedGroups().front();
    if (group.groupId != "group-fix-distance"
        || group.commandId != "fix-distance"
        || group.styleName != "D28"
        || group.grade != "HRB400"
        || group.bundleCount != 2
        || group.diameterM != 0.028) {
        return fail("generated preview must preserve request group fields");
    }
    if (group.centerlineStableIds.size() != 5
        || group.centerlineStableIds.front() != "generated-centerline-1"
        || group.centerlineStableIds.back() != "generated-centerline-5") {
        return fail("generated preview must store generated centerline refs");
    }
    return 0;
}

int expectFixDistancePreviewConsumesGeneratorPort()
{
    tsrs::domain::rebar::RebarModel model;
    FakeFixDistanceGenerator generator;
    tsrs::application::RebarCreationCommandService service(&model, &generator);
    MockGeometryEngine geometry;

    const auto preview = service.previewFixDistance(makeFixDistanceRequest(geometry));
    if (!preview.ok || !generator.called || generator.capturedStyleName != "D28") {
        return fail("previewFixDistance must consume IFixDistanceCenterlineGenerator port");
    }

    const auto commit = service.commitPreview();
    if (!commit.ok || model.committedGroupCount() != 1) {
        return fail("fake generator preview must commit through RebarModel");
    }

    const auto& group = model.committedGroups().front();
    if (group.centerlineStableIds.size() != 2
        || group.centerlineStableIds.front() != "fake-centerline-1"
        || group.centerlineStableIds.back() != "fake-centerline-2") {
        return fail("command service must map generator port result into RebarModel");
    }
    return 0;
}

int expectFixDistancePreviewRequiresGeometry()
{
    tsrs::domain::rebar::RebarModel model;
    FakeFixDistanceGenerator generator;
    tsrs::application::RebarCreationCommandService service(&model, &generator);
    MockGeometryEngine geometry;
    auto request = makeFixDistanceRequest(geometry);
    request.geometry = nullptr;

    const auto preview = service.previewFixDistance(request);
    if (preview.ok || preview.diagnosticCode
                          != tsrs::application::
                              kRebarCreationCommandDiagnosticGeometryMissing) {
        return fail("previewFixDistance without geometry must return GEOMETRY_MISSING");
    }
    if (model.hasPreview()) {
        return fail("failed previewFixDistance must not create preview state");
    }
    return 0;
}

int expectFixDistancePreviewRequiresGeneratorPort()
{
    tsrs::domain::rebar::RebarModel model;
    tsrs::application::RebarCreationCommandService service(&model);
    MockGeometryEngine geometry;

    const auto preview = service.previewFixDistance(makeFixDistanceRequest(geometry));
    if (preview.ok || preview.diagnosticCode
                          != tsrs::application::
                              kRebarCreationCommandDiagnosticGeneratorMissing) {
        return fail("previewFixDistance without generator port must return GENERATOR_MISSING");
    }
    if (model.hasPreview()) {
        return fail("missing generator port must not create preview state");
    }
    return 0;
}

int expectFixDistanceGeneratorFailureDoesNotCreatePreview()
{
    tsrs::domain::rebar::RebarModel model;
    const tsrs::application::RebarCreationCommandService service =
        tsrs::application::RebarCreationCommandServiceFactory::create(&model);
    MockGeometryEngine geometry;
    auto request = makeFixDistanceRequest(geometry);
    request.input.auxiliaryGuideCurve = curveRef("missing-auxiliary");

    const auto preview = service.previewFixDistance(request);
    if (preview.ok || preview.diagnosticCode
                          != "RS_FIX_DISTANCE_AUXILIARY_CURVE_INVALID") {
        return fail("generator diagnostic must pass through command service");
    }
    if (model.hasPreview() || model.committedGroupCount() != 0) {
        return fail("failed generator result must not create RebarModel state");
    }
    return 0;
}

} // namespace

int main()
{
    if (const int code = expectServiceRequiresModel()) {
        return code;
    }
    if (const int code = expectPreviewCommitCancelUseModelTransaction()) {
        return code;
    }
    if (const int code = expectFixDistancePreviewUsesGeneratorAndTransaction()) {
        return code;
    }
    if (const int code = expectFixDistancePreviewConsumesGeneratorPort()) {
        return code;
    }
    if (const int code = expectFixDistancePreviewRequiresGeometry()) {
        return code;
    }
    if (const int code = expectFixDistancePreviewRequiresGeneratorPort()) {
        return code;
    }
    if (const int code = expectFixDistanceGeneratorFailureDoesNotCreatePreview()) {
        return code;
    }
    return 0;
}
