#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace tsrs::geometry {

inline constexpr std::string_view kGeometryDiagnosticOk = "GEOM_OK";
inline constexpr std::string_view kGeometryDiagnosticMissingRef = "GEOM_MISSING_REF";
inline constexpr std::string_view kGeometryDiagnosticWrongEntityKind = "GEOM_WRONG_ENTITY_KIND";
inline constexpr std::string_view kGeometryDiagnosticLengthOutOfRange = "GEOM_LENGTH_OUT_OF_RANGE";
inline constexpr std::string_view kGeometryDiagnosticInvalidInput = "GEOM_INVALID_INPUT";

enum class GeometryEntityKind {
    Curve,
    Surface,
    Point,
    Solid,
};

struct GeometryRef {
    GeometryEntityKind kind{GeometryEntityKind::Curve};
    std::string stableId;
};

struct GeometryPoint3d {
    double x{0.0};
    double y{0.0};
    double z{0.0};
};

struct GeometryVector3d {
    double x{0.0};
    double y{0.0};
    double z{0.0};
};

struct GeometryLengthResult {
    bool ok{false};
    double lengthM{0.0};
    std::string diagnosticCode;
};

struct GeometryPointResult {
    bool ok{false};
    GeometryPoint3d point;
    std::string diagnosticCode;
};

struct GeometryVectorResult {
    bool ok{false};
    GeometryVector3d vector;
    std::string diagnosticCode;
};

struct GeometryRefResult {
    bool ok{false};
    GeometryRef ref;
    std::string diagnosticCode;
};

class IGeometryEngine {
public:
    virtual ~IGeometryEngine() = default;

    virtual GeometryLengthResult curveLength(const GeometryRef& curveRef) const = 0;

    virtual GeometryPointResult pointAtLength(const GeometryRef& curveRef, double lengthM) const = 0;

    // Returns a unit tangent vector at the requested curve length.
    virtual GeometryVectorResult tangentAtLength(const GeometryRef& curveRef,
                                                 double lengthM) const = 0;

    virtual GeometryRefResult makePolylineCurve(std::vector<GeometryPoint3d> points) = 0;
};

} // namespace tsrs::geometry
