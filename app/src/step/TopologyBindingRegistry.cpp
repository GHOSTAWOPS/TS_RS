#include "step/TopologyBindingRegistry.h"

#include <BRepBndLib.hxx>
#include <BRepGProp.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRep_Tool.hxx>
#include <Bnd_Box.hxx>
#include <GProp_GProps.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Vertex.hxx>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

constexpr double kQuantization = 1.0e-6;

long long quantize(double value)
{
    return static_cast<long long>(std::llround(value / kQuantization));
}

std::string q(double value)
{
    return std::to_string(quantize(value));
}

std::string qbbox(const tsrs::step::TopologyBbox& bbox)
{
    std::ostringstream stream;
    stream << q(bbox[0]) << ',' << q(bbox[1]) << ',' << q(bbox[2]) << ','
           << q(bbox[3]) << ',' << q(bbox[4]) << ',' << q(bbox[5]);
    return stream.str();
}

tsrs::step::TopologyBbox bboxOfShape(const TopoDS_Shape& shape)
{
    Bnd_Box box;
    BRepBndLib::Add(shape, box);
    if (box.IsVoid()) {
        return {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    }

    Standard_Real xmin = 0.0;
    Standard_Real ymin = 0.0;
    Standard_Real zmin = 0.0;
    Standard_Real xmax = 0.0;
    Standard_Real ymax = 0.0;
    Standard_Real zmax = 0.0;
    box.Get(xmin, ymin, zmin, xmax, ymax, zmax);
    return {xmin, ymin, zmin, xmax, ymax, zmax};
}

tsrs::step::TopologyBbox bboxOfVertex(const TopoDS_Shape& shape)
{
    const gp_Pnt point = BRep_Tool::Pnt(TopoDS::Vertex(shape));
    return {point.X(), point.Y(), point.Z(), point.X(), point.Y(), point.Z()};
}

double edgeLength(const TopoDS_Shape& shape)
{
    GProp_GProps props;
    BRepGProp::LinearProperties(shape, props);
    return props.Mass();
}

double faceArea(const TopoDS_Shape& shape)
{
    GProp_GProps props;
    BRepGProp::SurfaceProperties(TopoDS::Face(shape), props);
    return props.Mass();
}

int edgeCurveType(const TopoDS_Shape& shape)
{
    BRepAdaptor_Curve curve(TopoDS::Edge(shape));
    return static_cast<int>(curve.GetType());
}

int faceSurfaceType(const TopoDS_Shape& shape)
{
    BRepAdaptor_Surface surface(TopoDS::Face(shape));
    return static_cast<int>(surface.GetType());
}

std::string edgeEndpointFingerprint(const TopoDS_Shape& shape)
{
    TopoDS_Vertex first;
    TopoDS_Vertex last;
    TopExp::Vertices(TopoDS::Edge(shape), first, last);
    if (first.IsNull() || last.IsNull()) {
        return "open";
    }

    const gp_Pnt p1 = BRep_Tool::Pnt(first);
    const gp_Pnt p2 = BRep_Tool::Pnt(last);
    std::array<std::string, 2> endpoints{
        q(p1.X()) + "," + q(p1.Y()) + "," + q(p1.Z()),
        q(p2.X()) + "," + q(p2.Y()) + "," + q(p2.Z())};
    std::sort(endpoints.begin(), endpoints.end());
    return endpoints[0] + "|" + endpoints[1];
}

std::string vertexFingerprint(const TopoDS_Shape& shape)
{
    const gp_Pnt point = BRep_Tool::Pnt(TopoDS::Vertex(shape));
    return "vertex|p=" + q(point.X()) + "," + q(point.Y()) + "," + q(point.Z());
}

tsrs::step::TopologyBinding makeFaceBinding(const tsrs::step::IndexedShape& indexed)
{
    tsrs::step::TopologyBinding binding;
    binding.kind = tsrs::step::kTopologyKindFace;
    binding.localIndex = indexed.localIndex;
    binding.bbox = bboxOfShape(indexed.shape);
    binding.measure = faceArea(indexed.shape);
    binding.geometryFingerprint = "face|surface=" + std::to_string(faceSurfaceType(indexed.shape))
        + "|area=" + q(binding.measure) + "|bbox=" + qbbox(binding.bbox);
    binding.stableId = "tsrs-topology-v1:face:" + binding.geometryFingerprint;
    return binding;
}

tsrs::step::TopologyBinding makeEdgeBinding(const tsrs::step::IndexedShape& indexed)
{
    tsrs::step::TopologyBinding binding;
    binding.kind = tsrs::step::kTopologyKindEdge;
    binding.localIndex = indexed.localIndex;
    binding.bbox = bboxOfShape(indexed.shape);
    binding.measure = edgeLength(indexed.shape);
    binding.geometryFingerprint = "edge|curve=" + std::to_string(edgeCurveType(indexed.shape))
        + "|length=" + q(binding.measure) + "|bbox=" + qbbox(binding.bbox)
        + "|ends=" + edgeEndpointFingerprint(indexed.shape);
    binding.stableId = "tsrs-topology-v1:edge:" + binding.geometryFingerprint;
    return binding;
}

tsrs::step::TopologyBinding makeVertexBinding(const tsrs::step::IndexedShape& indexed)
{
    tsrs::step::TopologyBinding binding;
    binding.kind = tsrs::step::kTopologyKindVertex;
    binding.localIndex = indexed.localIndex;
    binding.bbox = bboxOfVertex(indexed.shape);
    binding.measure = 0.0;
    binding.geometryFingerprint = vertexFingerprint(indexed.shape);
    binding.stableId = "tsrs-topology-v1:vertex:" + binding.geometryFingerprint;
    return binding;
}

template <typename MakeBinding>
std::vector<tsrs::step::TopologyBinding> makeBindings(
    const std::vector<tsrs::step::IndexedShape>& shapes,
    MakeBinding makeBinding)
{
    std::vector<tsrs::step::TopologyBinding> bindings;
    bindings.reserve(shapes.size());
    for (const tsrs::step::IndexedShape& shape : shapes) {
        bindings.push_back(makeBinding(shape));
    }
    return bindings;
}

void appendAll(
    std::vector<tsrs::step::TopologyBinding>& all,
    const std::vector<tsrs::step::TopologyBinding>& bindings)
{
    all.insert(all.end(), bindings.begin(), bindings.end());
}

std::string escapeJson(const std::string& input)
{
    std::string escaped;
    escaped.reserve(input.size());
    for (char ch : input) {
        if (ch == '\\' || ch == '"') {
            escaped.push_back('\\');
        }
        escaped.push_back(ch);
    }
    return escaped;
}

std::string extractJsonString(const std::string& serialized, const std::string& key)
{
    const std::string marker = "\"" + key + "\":\"";
    const std::size_t start = serialized.find(marker);
    if (start == std::string::npos) {
        return {};
    }

    std::string value;
    bool escaping = false;
    for (std::size_t i = start + marker.size(); i < serialized.size(); ++i) {
        const char ch = serialized[i];
        if (escaping) {
            value.push_back(ch);
            escaping = false;
            continue;
        }
        if (ch == '\\') {
            escaping = true;
            continue;
        }
        if (ch == '"') {
            return value;
        }
        value.push_back(ch);
    }
    return value;
}

int extractJsonInt(const std::string& serialized, const std::string& key)
{
    const std::string marker = "\"" + key + "\":";
    const std::size_t start = serialized.find(marker);
    if (start == std::string::npos) {
        return -1;
    }
    const std::size_t valueStart = start + marker.size();
    const std::size_t valueEnd = serialized.find_first_of(",}", valueStart);
    return std::stoi(serialized.substr(valueStart, valueEnd - valueStart));
}

tsrs::step::TopologyBbox extractBbox(const std::string& serialized)
{
    const std::string marker = "\"fallbackBbox\":[";
    const std::size_t start = serialized.find(marker);
    if (start == std::string::npos) {
        return {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    }

    const std::size_t valueStart = start + marker.size();
    const std::size_t valueEnd = serialized.find(']', valueStart);
    std::stringstream stream(serialized.substr(valueStart, valueEnd - valueStart));
    tsrs::step::TopologyBbox bbox{0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    char comma = ',';
    for (std::size_t i = 0; i < bbox.size(); ++i) {
        stream >> bbox[i];
        if (i + 1 < bbox.size()) {
            stream >> comma;
        }
    }
    return bbox;
}

bool bboxNearlyEqual(
    const tsrs::step::TopologyBbox& lhs,
    const tsrs::step::TopologyBbox& rhs,
    double tolerance = 1.0e-5)
{
    for (std::size_t i = 0; i < lhs.size(); ++i) {
        if (std::fabs(lhs[i] - rhs[i]) > tolerance) {
            return false;
        }
    }
    return true;
}

std::vector<tsrs::step::TopologyBinding> filterByKind(
    const std::vector<tsrs::step::TopologyBinding>& bindings,
    const std::string& kind)
{
    std::vector<tsrs::step::TopologyBinding> filtered;
    for (const tsrs::step::TopologyBinding& binding : bindings) {
        if (binding.kind == kind) {
            filtered.push_back(binding);
        }
    }
    return filtered;
}

std::vector<tsrs::step::TopologyBinding> findStableIdCandidates(
    const std::vector<tsrs::step::TopologyBinding>& bindings,
    const std::string& stableId)
{
    std::vector<tsrs::step::TopologyBinding> candidates;
    for (const tsrs::step::TopologyBinding& binding : bindings) {
        if (binding.stableId == stableId) {
            candidates.push_back(binding);
        }
    }
    return candidates;
}

std::vector<tsrs::step::TopologyBinding> findGeometryFingerprintCandidates(
    const std::vector<tsrs::step::TopologyBinding>& bindings,
    const tsrs::step::TopologyBindingReference& reference)
{
    std::vector<tsrs::step::TopologyBinding> candidates;
    if (!reference.geometryFingerprint.empty()) {
        for (const tsrs::step::TopologyBinding& binding : bindings) {
            if (binding.kind != reference.kind) {
                continue;
            }
            if (binding.geometryFingerprint == reference.geometryFingerprint) {
                candidates.push_back(binding);
            }
        }
    }
    return candidates;
}

std::vector<tsrs::step::TopologyBinding> findLocalIndexBboxCandidates(
    const std::vector<tsrs::step::TopologyBinding>& bindings,
    const tsrs::step::TopologyBindingReference& reference)
{
    const bool hasLocalIndex = reference.fallbackLocalIndex >= 0;
    if (!hasLocalIndex) {
        return {};
    }

    std::vector<tsrs::step::TopologyBinding> candidates;
    for (const tsrs::step::TopologyBinding& binding : bindings) {
        if (binding.kind == reference.kind
            && binding.localIndex == reference.fallbackLocalIndex
            && bboxNearlyEqual(binding.bbox, reference.fallbackBbox)) {
            candidates.push_back(binding);
        }
    }
    return candidates;
}

std::optional<tsrs::step::TopologyBindingLookupResult> makeResultFromCandidates(
    const std::vector<tsrs::step::TopologyBinding>& candidates,
    const tsrs::step::TopologyBindingReference& reference,
    bool usedFallback)
{
    if (candidates.empty()) {
        return std::nullopt;
    }

    tsrs::step::TopologyBindingLookupResult result;
    result.usedFallback = usedFallback;
    result.candidateCount = static_cast<int>(candidates.size());
    if (candidates.size() > 1) {
        result.diagnosticCode = tsrs::step::kTopologyDiagnosticBindingAmbiguous;
        result.diagnostic = usedFallback
            ? "Topology binding fallback matched multiple candidates for role: " + reference.role
            : "Topology binding matched multiple candidates: " + reference.stableId;
        return result;
    }
    if (candidates.front().kind != reference.kind) {
        result.diagnosticCode = tsrs::step::kTopologyDiagnosticKindMismatch;
        result.diagnostic = "Topology binding kind mismatch. expected=" + reference.kind
            + ", actual=" + candidates.front().kind;
        return result;
    }

    result.ok = true;
    result.diagnosticCode = tsrs::step::kTopologyDiagnosticOk;
    result.diagnostic.clear();
    result.binding = candidates.front();
    return result;
}

} // namespace

namespace tsrs::step {

TopologyBindingRegistry TopologyBindingRegistry::build(const ShapeStore& store)
{
    TopologyBindingRegistry registry;
    registry.faces_ = makeBindings(store.faces(), makeFaceBinding);
    registry.edges_ = makeBindings(store.edges(), makeEdgeBinding);
    registry.vertices_ = makeBindings(store.vertices(), makeVertexBinding);
    appendAll(registry.allBindings_, registry.faces_);
    appendAll(registry.allBindings_, registry.edges_);
    appendAll(registry.allBindings_, registry.vertices_);

    std::map<std::string, int> stableIdCounts;
    for (const TopologyBinding& binding : registry.allBindings_) {
        ++stableIdCounts[binding.stableId];
    }
    for (const auto& [stableId, count] : stableIdCounts) {
        if (count > 1) {
            registry.ok_ = false;
            registry.diagnostic_ = "Duplicate topology stable id: " + stableId;
            break;
        }
    }
    return registry;
}

TopologyBindingRegistry TopologyBindingRegistry::fromBindings(
    std::vector<TopologyBinding> bindings)
{
    TopologyBindingRegistry registry;
    for (const TopologyBinding& binding : bindings) {
        if (binding.kind == kTopologyKindFace) {
            registry.faces_.push_back(binding);
        } else if (binding.kind == kTopologyKindEdge) {
            registry.edges_.push_back(binding);
        } else if (binding.kind == kTopologyKindVertex) {
            registry.vertices_.push_back(binding);
        }
        registry.allBindings_.push_back(binding);
    }
    return registry;
}

TopologyBindingLookupResult TopologyBindingRegistry::restore(
    const TopologyBindingReference& reference) const
{
    TopologyBindingLookupResult result;
    result.diagnosticCode = kTopologyDiagnosticBindingMissing;
    result.diagnostic = "No topology binding matched stable id: " + reference.stableId;

    const std::vector<TopologyBinding> stableIdCandidates =
        findStableIdCandidates(allBindings_, reference.stableId);
    if (const std::optional<TopologyBindingLookupResult> exactResult =
            makeResultFromCandidates(stableIdCandidates, reference, false)) {
        return *exactResult;
    }

    if (!reference.stableId.empty() && !stableIdCandidates.empty()
        && filterByKind(stableIdCandidates, reference.kind).empty()) {
        result.candidateCount = static_cast<int>(stableIdCandidates.size());
        result.diagnosticCode = kTopologyDiagnosticKindMismatch;
        result.diagnostic = "Topology binding kind mismatch. expected=" + reference.kind;
        return result;
    }

    const std::vector<TopologyBinding> geometryFingerprintCandidates =
        findGeometryFingerprintCandidates(allBindings_, reference);
    if (const std::optional<TopologyBindingLookupResult> fallbackResult =
            makeResultFromCandidates(geometryFingerprintCandidates, reference, true)) {
        return *fallbackResult;
    }

    const std::vector<TopologyBinding> localIndexBboxCandidates =
        findLocalIndexBboxCandidates(allBindings_, reference);
    if (const std::optional<TopologyBindingLookupResult> fallbackResult =
            makeResultFromCandidates(localIndexBboxCandidates, reference, true)) {
        if (fallbackResult->ok) {
            TopologyBindingLookupResult lowConfidence = *fallbackResult;
            lowConfidence.ok = false;
            lowConfidence.diagnosticCode = kTopologyDiagnosticBindingLowConfidence;
            lowConfidence.diagnostic =
                "Topology binding localIndex+bbox fallback is low confidence for role: "
                + reference.role;
            return lowConfidence;
        }
        return *fallbackResult;
    }

    result.candidateCount = 0;
    return result;
}

TopologyBindingReference makeBindingReference(
    const std::string& role,
    const TopologyBinding& binding)
{
    TopologyBindingReference reference;
    reference.role = role;
    reference.kind = binding.kind;
    reference.stableId = binding.stableId;
    reference.geometryFingerprint = binding.geometryFingerprint;
    reference.fallbackLocalIndex = binding.localIndex;
    reference.fallbackBbox = binding.bbox;
    return reference;
}

std::string serializeBindingReference(const TopologyBindingReference& reference)
{
    std::ostringstream stream;
    stream << "{\"role\":\"" << escapeJson(reference.role) << "\",";
    stream << "\"kind\":\"" << escapeJson(reference.kind) << "\",";
    stream << "\"stableId\":\"" << escapeJson(reference.stableId) << "\",";
    stream << "\"geometryFingerprint\":\"" << escapeJson(reference.geometryFingerprint)
           << "\",";
    stream << "\"fallbackLocalIndex\":" << reference.fallbackLocalIndex << ",";
    stream << "\"fallbackBbox\":[";
    for (std::size_t i = 0; i < reference.fallbackBbox.size(); ++i) {
        if (i > 0) {
            stream << ',';
        }
        stream << std::setprecision(17) << reference.fallbackBbox[i];
    }
    stream << "]}";
    return stream.str();
}

TopologyBindingReference deserializeBindingReference(const std::string& serialized)
{
    TopologyBindingReference reference;
    reference.role = extractJsonString(serialized, "role");
    reference.kind = extractJsonString(serialized, "kind");
    reference.stableId = extractJsonString(serialized, "stableId");
    reference.geometryFingerprint = extractJsonString(serialized, "geometryFingerprint");
    reference.fallbackLocalIndex = extractJsonInt(serialized, "fallbackLocalIndex");
    reference.fallbackBbox = extractBbox(serialized);
    return reference;
}

} // namespace tsrs::step
