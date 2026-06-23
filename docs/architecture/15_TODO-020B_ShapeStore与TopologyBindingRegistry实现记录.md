# TODO-020B ShapeStore 与 TopologyBindingRegistry 实现记录

## 结论

TODO-020B 完成了 STEP 拓扑稳定引用的 P0 切片：

```text
StepImportResult
  -> ShapeStore
  -> TopologyBindingRegistry
  -> stable binding id
  -> binding reference serialize / deserialize
  -> restore diagnostic
```

本节点只证明：

```text
同一 STEP 多次导入后，face / edge / vertex 的 P0 fingerprint 可稳定复现。
binding reference 可保存成 JSON 风格字符串并恢复。
缺失、kind mismatch、ambiguous 有稳定 diagnostic。
```

本节点不证明：

```text
真实 Viewer 子对象选择已完成。
ShapeStore / TopologyBindingRegistry 已经接入 STEP 导入 / 显示主链路。
大型工程 STEP 所有面 / 边都不会 collision。
TopologyBinding 已达到最终算法质量。
钢筋生成器可以接真实选择。
Detail 包兼容已完成。
```

重要边界：

```text
TODO-020B 当前是 registry library P0。
它证明 ShapeStore / TopologyBindingRegistry 可以独立构建和测试，
但不代表 MainWindow / StepImportCommandService / OccViewerWidget
已经形成 StepSession 主链路。

进入 TODO-021 Viewer 选择系统前，
必须让选择输出 stable binding id，
且禁止把裸 TopoDS / AIS 对象交给 generator。
```

## 实现内容

新增：

```text
app/src/step/ShapeStore.h
app/src/step/ShapeStore.cpp
app/src/step/TopologyBindingRegistry.h
app/src/step/TopologyBindingRegistry.cpp
app/tests/step/topology_binding_registry_tests.cpp
```

### ShapeStore

职责：

```text
从 StepImportResult.rootShape 枚举唯一 face / edge / vertex。
保留 sourcePath 和 rootShape。
```

实现口径：

```text
使用 TopExp::MapShapes(root, kind, indexedMap)。
```

原因：

```text
共享边 / 点可能在拓扑遍历中重复出现。
ShapeStore P0 先给唯一 sub-shape 建 binding。
```

### TopologyBindingRegistry

当前 P0 fingerprint：

```text
face:
  surface type
  area
  bbox

edge:
  curve type
  length
  bbox
  endpoint coordinates

vertex:
  coordinate
```

量化：

```text
坐标 / 长度 / 面积当前按 1e-6 量化。
```

stable id 格式：

```text
tsrs-topology-v1:{kind}:{geometryFingerprint}
```

当前 diagnostic：

```text
BINDING_MISSING
BINDING_AMBIGUOUS
BINDING_KIND_MISMATCH
TOPOLOGY_BINDING_OK
```

## 测试

新增测试：

```text
tsrs_topology_binding_registry_tests
```

覆盖：

```text
1. 合成 box STEP 连续导入 5 次。
2. box face / edge / vertex 唯一 binding 数量分别为 6 / 12 / 8。
3. face / edge / vertex stableId 列表保持一致。
4. restore 后断言 stableId / kind / geometryFingerprint / bbox / measure。
5. edge binding reference serialize / deserialize 后重新导入可恢复。
6. missing stableId 返回 BINDING_MISSING。
7. stableId 命中但 kind 不同返回 BINDING_KIND_MISMATCH。
8. 人工重复 stableId 返回 BINDING_AMBIGUOUS。
```

专项验证：

```powershell
ctest --test-dir build/default -R tsrs_topology_binding_registry_tests --output-on-failure -C Debug
```

结果：

```text
1/1 passed。
```

默认 CTest：

```powershell
ctest --test-dir build/default --output-on-failure -C Debug
```

结果：

```text
18/18 passed。
```

护栏扫描：

```powershell
rg -n "TopoDS_|AIS_|BRep|TopAbs_|gp_|Geom_" app/src/rebarsmart
rg -n "TopoDS_|AIS_|BRep|TopAbs_|gp_|Geom_" app/src/application app/src/rebarsmart
rg -n "EdgeToRebarFactory|FaceRebarGenerator|LegacyUiCommandMap|OccLegacyGeometryAdapter|VisualTS" app/src
```

结果：

```text
均无匹配。
```

## 边界检查

本节点只改：

```text
app/src/step
app/tests/step
app/CMakeLists.txt
app/tests/CMakeLists.txt
docs/...
todo.csv
```

没有修改：

```text
app/src/rebarsmart
app/src/ui
app/src/presentation/occ
app/src/application
```

必须保持：

```text
RebarSmart generator 不 include TopoDS_ / AIS_ / BRep / TopAbs / gp_ / Geom_。
Viewer selection 不能绕过 binding id 直接把 TopoDS 传给 generator。
```

## 当前 GAP

| ID | 缺口 | 处理 |
| --- | --- | --- |
| TODO020B-GAP-001 | P0 fingerprint 对真实复杂工程 STEP 的 collision 风险未充分验证。 | 后续用 `123.stp` 和结构 STEP 做 GC-002 扩展。 |
| TODO020B-GAP-002 | face fingerprint 尚未包含 outer wire / inner wire / parent solid。 | P1 扩展，不阻塞 P0B。 |
| TODO020B-GAP-003 | edge fingerprint endpoint 顺序可能受 orientation 影响。 | 已由 TODO-020D endpoint canonicalization 关闭。 |
| TODO020B-GAP-004 | 当前 serialize / deserialize 是受控 JSON 风格字符串，不是完整工程文件 schema。 | TODO-026 `.tsrebar` save/open 时统一收口。 |
| TODO020B-GAP-005 | 真实 Viewer selection -> binding id 尚未接通。 | TODO-021 处理；但 TODO-021 必须等 Detail 前置验证后再做。 |
| TODO020B-GAP-006 | `StepImportCommandService` 仍通过 `StepDisplayModel` 间接暴露 `TopoDS_Shape` 到 application/presentation 边界。 | 这是 TODO-020 已记录的显示链路近似；不在 TODO-020B 扩散重构，后续 ShapeStore / display model 边界收口节点处理。 |
| TODO020B-GAP-007 | registry 尚未接入 StepSession / ImportedModelStore 主链路。 | 新增后续 TODO，TODO-021 前必须避免 Viewer 选择绕过 registry。 |
| TODO020B-GAP-008 | restore 当前以 stableId exact match 为主，未使用 geometryFingerprint / fallbackLocalIndex / fallbackBbox 做降级恢复。 | 已由 TODO-020D restore fallback 语义关闭。 |
| TODO020B-GAP-009 | edge endpoint fingerprint 尚未 canonicalize，方向翻转可能导致 stableId 漂移。 | 已由 TODO-020D endpoint canonicalization 关闭。 |

## xhigh 只读 review

本轮涉及代码、测试、CMake 和 roadmap / validation 文档，按项目规则在 commit 前执行 xhigh 只读 review。

review 结论：

```text
Critical: 无。
Important:
  1. topology_binding_registry_tests 断言偏弱。
  2. StepImportCommandService 既有边界间接暴露 TopoDS_Shape。
```

处理：

```text
Important-1 已修复：
  增加 box exact counts。
  增加全量 face / edge / vertex stableId 列表稳定断言。
  增加 restore 后 geometryFingerprint / bbox / measure 断言。

Important-2 记录为既有边界近似：
  TODO-020 已说明 StepDisplayModel 暂持 TopoDS_Shape 只限显示链路。
  TODO-020B 不扩大到 application / presentation 重构。
```

复验：

```text
tsrs_topology_binding_registry_tests: 1/1 passed。
默认 CTest: 18/18 passed。
```

## 下一步

按外部评审后的 `todo.csv`，下一步不是 Viewer 选择系统，而是：

```text
TODO-022 DetailPackageReader P0
TODO-023 DetailPackageWriter round-trip
TODO-024 极简 Detail 包生成 + autoin 人工验证
```

原因：

```text
Detail 包兼容是外部输出硬风险。
必须在真实 Viewer 选择闭环和钢筋生成接入前先 fail-fast。
```
