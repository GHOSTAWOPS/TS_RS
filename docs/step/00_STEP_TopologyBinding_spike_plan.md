# STEP Import + TopologyBinding Spike 计划

## 结论

TODO-017 只做 spike 计划，不写 C++。

本 spike 的目标是提前验证一个高风险问题：

```text
真实 STEP / STP 导入后，
Face / Edge / Vertex 能否生成稳定引用，
并在保存 / 重开后恢复选中对象。
```

没有这个验证，不允许把真实 Viewer 选择结果接入 FixDistance / FixNumber 生成器。

## 背景

P0A 最小闭环要求：

```text
真实 STEP
  -> OCCT Viewer 水泥灰显示
  -> 选择边 / 面
  -> TopologyBinding 保存和恢复
  -> FixDistance 或 FixNumber 中心线生成
```

如果 TopologyBinding 不稳定，后续会出现：

```text
保存工程后找不回选中的面 / 边。
同一个 STEP 不同导入顺序导致 ID 变化。
钢筋组绑定到临时 AIS 对象。
RebarModel 被迫重构。
```

所以 TopologyBinding 必须前置。

## Spike 验收目标

### S1. 真实 STEP 导入

输入：

```text
至少 1 个真实 STEP / STP 样本。
优先使用用户已经从老图石导出的钢筋 STP 或结构 STP。
```

验收：

```text
OCCT 能成功导入。
能枚举 shape / solid / face / edge / vertex 数量。
导入失败时有明确 diagnostic。
```

### S2. Topology fingerprint

每个可绑定对象至少生成：

```text
kind:
  shape / solid / shell / face / edge / vertex

localIndex:
  当前导入树中的枚举序号，仅作辅助，不作为唯一真相。

geometryFingerprint:
  基于几何摘要的稳定指纹。

parentFingerprint:
  父对象摘要，用于减少不同部位相似边 / 面碰撞。

bbox:
  包围盒摘要。

measure:
  edge length / face area / vertex coordinate。
```

验收：

```text
同一 STEP 连续导入 5 次，
关键 face / edge / vertex 的 fingerprint 稳定。
```

### S3. 保存 / 恢复

保存最小 JSON：

```json
{
  "sourceStep": "sample.stp",
  "bindings": [
    {
      "role": "selectedGuideEdge",
      "kind": "edge",
      "fingerprint": "...",
      "fallback": {
        "localIndex": 123,
        "bbox": [0, 0, 0, 1, 1, 1]
      }
    }
  ]
}
```

验收：

```text
重开同一 STEP 后，
能根据 fingerprint 找回同一个 edge / face / vertex。

如果唯一匹配失败，
必须返回 ambiguous / missing diagnostic，
不能静默绑定到错误对象。
```

### S4. 与 Viewer 选择衔接

本 spike 不要求完成 Viewer 选择系统。

但必须为 TODO-021 留出接口：

```text
Viewer 点击 AIS_Shape 子对象
  -> 得到 TopoDS sub-shape
  -> TopologyBindingRegistry 查询 stable binding
  -> UI / RebarModel 使用 binding id
```

验收：

```text
计划文档明确 Viewer selection -> binding id 的交接点。
```

## Fingerprint 候选策略

### Edge

候选字段：

```text
curve type
length
start point
end point
mid point
bbox
sample points
parent face / parent shape fingerprint
```

注意：

```text
不能只用 localIndex。
不能只用长度。
同长直线很多，必须加父对象 / bbox / 采样点。
```

### Face

候选字段：

```text
surface type
area
normal at representative point
bbox
outer wire edge fingerprints
inner wire count
sample UV points
parent solid / shape fingerprint
```

注意：

```text
平面、圆柱面、锥面要分开。
相似面要用 parent + boundary 降低碰撞。
```

### Vertex

候选字段：

```text
coordinate
adjacent edge fingerprints
parent shape fingerprint
```

注意：

```text
坐标需要按 tolerance 量化。
不能用浮点原值直接拼字符串。
```

## Diagnostic 口径

必须定义稳定错误码：

```text
STEP_IMPORT_FAILED
TOPOLOGY_ENUM_EMPTY
FINGERPRINT_COLLISION
BINDING_MISSING
BINDING_AMBIGUOUS
BINDING_KIND_MISMATCH
UNSUPPORTED_SHAPE_TYPE
TOLERANCE_TOO_LOOSE
TOLERANCE_TOO_STRICT
```

错误必须包含：

```text
source file
requested binding role
expected kind
candidate count
fingerprint summary
fallback localIndex / bbox
```

## 测试计划

### 单样本稳定性

```text
同一 STEP 导入 5 次。
记录 face / edge / vertex 数量。
记录前 N 个对象 fingerprint。
断言关键对象 fingerprint 完全一致。
```

### 保存恢复

```text
选择 1 个 edge、1 个 face、1 个 vertex。
写入 binding JSON。
重启测试进程。
重新导入 STEP。
恢复 binding。
断言 kind 一致、fingerprint 一致、bbox / measure 在容差内。
```

### 碰撞诊断

```text
构造或选择存在相似边 / 面的样本。
故意只用弱 fingerprint 触发 collision。
断言返回 FINGERPRINT_COLLISION 或 BINDING_AMBIGUOUS。
```

### 样本缺失

```text
删除 / 重命名 STEP。
断言 STEP_IMPORT_FAILED 或 BINDING_MISSING，
不能崩溃。
```

## 目录建议

后续 TODO-020 / TODO-021 可按此结构实现：

```text
app/src/step/
  StepImportService.h
  StepImportService.cpp
  TopologyBinding.h
  TopologyBindingRegistry.h
  TopologyBindingRegistry.cpp

app/tests/step/
  topology_binding_spike_tests.cpp
  fixtures/
```

说明：

```text
fixtures 只放可公开的小 STEP 样本。
大型真实工程样本放 samples/local/，不进 git。
```

## 与现有边界的关系

允许：

```text
TopologyBinding / STEP import 层内部使用 OCCT 类型。
presentation/occ 使用 AIS / OCCT Viewer 类型。
测试中使用真实 STEP 样本。
```

禁止：

```text
RebarSmart generator 直接 include TopoDS_ / AIS_ / BRep / gp_ / Geom_。
RebarModel 保存 AIS object 指针。
用 localIndex 当唯一稳定 ID。
在没有 TopologyBinding 验证前，把真实选择结果接入生成器。
```

## 依赖输入

当前项目目录内未发现 `.stp` / `.step` fixture。

后续实现需要选择至少一个样本来源：

```text
1. 用户提供的真实结构 STEP / STP。
2. 用户从老图石导出的钢筋 STP。
3. 项目内生成一个最小公开 STEP fixture，仅用于 CI。
```

真实工程样本建议只放：

```text
samples/local/
```

并继续保持 `.gitignore` 不提交。

## 完成后进入的下一步

TODO-017 完成后：

```text
TODO-018：Detail 包证据索引与 round-trip policy。
```

不要直接跳到 STEP 导入实现。
