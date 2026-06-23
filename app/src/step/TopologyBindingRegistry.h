#pragma once

#include "step/ShapeStore.h"

#include <array>
#include <string>
#include <vector>

namespace tsrs::step {

inline constexpr const char* kTopologyKindFace = "face";
inline constexpr const char* kTopologyKindEdge = "edge";
inline constexpr const char* kTopologyKindVertex = "vertex";

inline constexpr const char* kTopologyDiagnosticOk = "TOPOLOGY_BINDING_OK";
inline constexpr const char* kTopologyDiagnosticBindingMissing = "BINDING_MISSING";
inline constexpr const char* kTopologyDiagnosticBindingAmbiguous = "BINDING_AMBIGUOUS";
inline constexpr const char* kTopologyDiagnosticKindMismatch = "BINDING_KIND_MISMATCH";

using TopologyBbox = std::array<double, 6>;

struct TopologyBinding {
    std::string kind;
    int localIndex{0};
    std::string stableId;
    std::string geometryFingerprint;
    TopologyBbox bbox{0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    double measure{0.0};
};

struct TopologyBindingReference {
    std::string role;
    std::string kind;
    std::string stableId;
    std::string geometryFingerprint;
    int fallbackLocalIndex{-1};
    TopologyBbox fallbackBbox{0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
};

struct TopologyBindingLookupResult {
    bool ok{false};
    bool usedFallback{false};
    std::string diagnosticCode{kTopologyDiagnosticBindingMissing};
    std::string diagnostic;
    int candidateCount{0};
    TopologyBinding binding;
};

class TopologyBindingRegistry final {
public:
    [[nodiscard]] static TopologyBindingRegistry build(const ShapeStore& store);
    [[nodiscard]] static TopologyBindingRegistry fromBindings(
        std::vector<TopologyBinding> bindings);

    [[nodiscard]] bool ok() const { return ok_; }
    [[nodiscard]] const std::string& diagnostic() const { return diagnostic_; }
    [[nodiscard]] const std::vector<TopologyBinding>& faces() const { return faces_; }
    [[nodiscard]] const std::vector<TopologyBinding>& edges() const { return edges_; }
    [[nodiscard]] const std::vector<TopologyBinding>& vertices() const { return vertices_; }
    [[nodiscard]] const std::vector<TopologyBinding>& allBindings() const { return allBindings_; }

    [[nodiscard]] TopologyBindingLookupResult restore(
        const TopologyBindingReference& reference) const;

private:
    bool ok_{true};
    std::string diagnostic_;
    std::vector<TopologyBinding> faces_;
    std::vector<TopologyBinding> edges_;
    std::vector<TopologyBinding> vertices_;
    std::vector<TopologyBinding> allBindings_;
};

[[nodiscard]] TopologyBindingReference makeBindingReference(
    const std::string& role,
    const TopologyBinding& binding);

[[nodiscard]] std::string serializeBindingReference(
    const TopologyBindingReference& reference);

[[nodiscard]] TopologyBindingReference deserializeBindingReference(
    const std::string& serialized);

} // namespace tsrs::step
