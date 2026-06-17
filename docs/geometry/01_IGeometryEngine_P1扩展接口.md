# IGeometryEngine P1 扩展接口规划

本文档按外部技术评审意见补齐几何接口后续边界。

## 结论

当前 `IGeometryEngine P0` 只支撑中心线级别生成：

```text
curveLength
pointAtLength
tangentAtLength
makePolylineCurve
```

它足够支撑：

```text
FixDistanceCenterlineGenerator P0
FixNumberCenterlineGenerator P0
```

但不等于足够支撑完整 RebarSmart FixDistance / FixNumber。

完整 RebarSmart-style 几何复刻需要 P1/P2 扩展接口。

## 为什么不能提前塞进 P0

外部评审指出：

```text
如果 P0 接口过窄却宣称完整 FixDistance，
开发者会被迫在 generator 内部直接 include OCCT。
```

本项目采用两步策略：

1. P0 只承认中心线生成。
2. P1 再扩展投影、偏移、相交、扫掠等真实几何能力。

## P1 候选能力

### curve / wire 投影

用途：

```text
导引线投影到布筋面。
辅助线投影到参考面。
```

候选接口：

```cpp
GeometryResult<GeometryRef> projectCurveToFace(
    const GeometryRef& curve,
    const GeometryRef& face,
    const ProjectionOptions& options);
```

证据来源：

```text
RebarSmart guide curve projection 字段。
docs/rebarsmart/02_单位枚举默认值规则.md 中 modeProject / projection 相关 GAP。
```

### 平面 / 曲面偏移

用途：

```text
保护层偏移。
布筋面偏移。
导引线平行生成。
```

候选接口：

```cpp
GeometryResult<GeometryRef> offsetCurveOnPlane(
    const GeometryRef& curve,
    const GeometryRef& planeOrFace,
    double offsetM,
    const OffsetOptions& options);
```

### 曲线 / 面求交

用途：

```text
钢筋与边界裁剪。
布筋区间截断。
剖切 / 工程图投影准备。
```

候选接口：

```cpp
GeometryResult<IntersectionSet> intersectCurveWithFace(
    const GeometryRef& curve,
    const GeometryRef& face,
    const IntersectionOptions& options);
```

### 曲线 / 曲线求交

用途：

```text
导引线分区。
孔洞边界。
钢筋裁剪。
```

候选接口：

```cpp
GeometryResult<IntersectionSet> intersectCurves(
    const GeometryRef& a,
    const GeometryRef& b,
    const IntersectionOptions& options);
```

### 三维平行曲线

用途：

```text
定距布置。
面配筋偏移。
角筋 / 周边筋。
```

候选接口：

```cpp
GeometryResult<GeometryRef> parallelCurve3d(
    const GeometryRef& curve,
    const Vector3d& direction,
    double distanceM,
    const ParallelCurveOptions& options);
```

### 圆截面扫掠

用途：

```text
把中心线变成钢筋实体。
导出钢筋 STEP。
Viewer 中显示真实钢筋。
```

候选接口：

```cpp
GeometryResult<GeometryRef> sweepCircle(
    const GeometryRef& centerline,
    double radiusM,
    const SweepOptions& options);
```

### 剖切和 HLR 投影

用途：

```text
工程图视图。
剖切线。
隐藏线 / 可见线。
```

候选接口：

```cpp
GeometryResult<SectionResult> sectionShape(
    const GeometryRef& shape,
    const Plane3d& plane,
    const SectionOptions& options);

GeometryResult<ProjectedView> projectHiddenLine(
    const GeometryRef& shape,
    const ViewDirection& view,
    const HlrOptions& options);
```

## 分期边界

### P0 已完成

```text
曲线长度。
按弧长取点。
切线。
组装 polyline。
```

### P1 建议

```text
projectCurveToFace
offsetCurveOnPlane
intersectCurveWithFace
intersectCurves
parallelCurve3d
```

### P2 建议

```text
sweepCircle
sectionShape
projectHiddenLine
复杂 wire 修复
实体布尔裁剪
```

## 禁止事项

在 P1 接口实现前：

```text
不要在 generator 内部直接使用 OCCT 投影 / 偏移 / 相交 API。
不要把 OCCT shape 指针塞进 RebarSmart DTO。
不要把 P0 中心线生成称为完整 RebarSmart 几何复刻。
不要为了某个测试把 GeometryRef 扩成万能对象。
```

## 与当前 todo 的关系

当前 `TODO-020 / TODO-021` 仍在补 STEP 显示和 Viewer 选择。

建议后续在进入完整面配筋、真实投影、过孔、实体钢筋前，新增：

```text
TODO-0xx GeometryEngine P1 接口冻结
TODO-0xx OcctGeometryEngine P1 投影 / 偏移 / 相交 Spike
TODO-0xx sweepCircle 钢筋实体显示 Spike
```

