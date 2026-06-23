# TODO-020D TopologyBindingRegistry P1 Hardening 实现记录

## 结论

TODO-020D 完成了 `TopologyBindingRegistry` 的 P1 加固：

```text
edge endpoint canonicalization
restore fallback semantics
fallback ambiguity diagnostics
```

本节点解决的是 TODO-020B 后留下的两个风险：

```text
1. edge fingerprint endpoint 顺序受 edge orientation 影响，可能导致 stableId 漂移。
2. TopologyBindingReference 保存了 geometryFingerprint / fallbackLocalIndex / fallbackBbox，
   但 restore() 只做 stableId exact match，fallback 字段没有真正参与恢复。
```

完成后：

```text
反向端点顺序的同一条 edge 会生成相同 geometryFingerprint / stableId。
stableId 漂移时，restore() 可以先用 geometryFingerprint 降级恢复。
geometryFingerprint 也漂移时，可以用 fallbackLocalIndex + fallbackBbox 做最后降级恢复。
多个 fallback 候选时返回 BINDING_AMBIGUOUS，不静默绑定。
```

本节点不表示：

```text
真实大型 STEP 的所有 face / edge / vertex 都不会 collision。
Viewer 选择系统已经接入。
ShapeStore / Registry 已经进入 StepSession 主链路。
RebarModel / generator 可以接收真实选择。
```

## 修改范围

代码：

```text
app/src/step/TopologyBindingRegistry.h
app/src/step/TopologyBindingRegistry.cpp
app/tests/step/topology_binding_registry_tests.cpp
```

文档 / 看板：

```text
docs/architecture/20_TODO-020D_TopologyBindingRegistryP1实现记录.md
docs/architecture/15_TODO-020B_ShapeStore与TopologyBindingRegistry实现记录.md
docs/roadmap/00_路线图.md
docs/roadmap/01_长期Goal执行目标.md
todo.csv
```

没有修改：

```text
app/src/rebarsmart
app/src/presentation/occ
app/src/ui
app/src/application
app/src/drawing/detail
```

## 实现内容

### 1. Edge Endpoint Canonicalization

TODO-020B 的 edge fingerprint 使用：

```text
start endpoint | end endpoint
```

这会受 OCCT edge orientation / 枚举方向影响。

TODO-020D 改为：

```text
quantized endpoint A
quantized endpoint B
sort(endpoint A, endpoint B)
endpoint[0] | endpoint[1]
```

因此同一几何 edge 即使端点顺序反过来，fingerprint 也稳定。

### 2. Restore Fallback 语义

`TopologyBindingLookupResult` 新增：

```cpp
bool usedFallback{false};
```

用于区分：

```text
exact stableId restore
fallback restore
```

restore 顺序：

```text
1. stableId exact match。
2. stableId 命中但 kind 不一致 -> BINDING_KIND_MISMATCH。
3. geometryFingerprint fallback。
4. 若 geometryFingerprint 多候选，用 fallbackLocalIndex + fallbackBbox 缩小。
5. 若 geometryFingerprint 无候选，用 fallbackLocalIndex + fallbackBbox 最后降级。
6. 多候选 -> BINDING_AMBIGUOUS。
7. 无候选 -> BINDING_MISSING。
```

fallback 只在唯一候选时返回成功。

### 3. Bbox Drift 容差

fallback bbox 比较当前使用：

```text
1e-5
```

原因：

```text
P0/P1 阶段需要容忍 STEP 重导入、量化、微小浮点漂移。
但该容差不是最终真实工程策略。
真实工程样本仍需后续 GC 扩展验证。
```

## 测试覆盖

`tsrs_topology_binding_registry_tests` 新增 / 加强：

```text
1. 合成 box STEP 连续导入 5 次 stableId 稳定。
2. 反向端点 edge 的 geometryFingerprint / stableId 相同。
3. binding reference serialize / deserialize 后 exact restore。
4. stableId 漂移但 geometryFingerprint 相同时 fallback restore。
5. stableId / geometryFingerprint 都漂移但 localIndex + bbox 匹配时 fallback restore。
6. fallback 支持全零 bbox。
7. missing 返回 BINDING_MISSING。
8. kind mismatch 返回 BINDING_KIND_MISMATCH。
9. stableId 重复返回 BINDING_AMBIGUOUS。
10. fallback 多候选返回 BINDING_AMBIGUOUS。
11. STEP box fixture 使用进程唯一临时文件名，避免并行测试互删临时文件。
```

## 验证记录

专项验证：

```powershell
cmd /c "call ""D:\Visual Studio 2026\Community\VC\Auxiliary\Build\vcvars64.bat"" >nul && cmake --build build/default --target tsrs_topology_binding_registry_tests --config Debug && ctest --test-dir build/default -C Debug -R tsrs_topology_binding_registry_tests --output-on-failure"
```

结果：

```text
1/1 passed
```

默认 CTest：

```powershell
cmd /c "call ""D:\Visual Studio 2026\Community\VC\Auxiliary\Build\vcvars64.bat"" >nul && ctest --test-dir build/default -C Debug --output-on-failure"
```

结果：

```text
22/22 passed
```

OCCT / AIS 泄漏检查：

```powershell
rg -n "TopoDS_|AIS_|BRep|TopAbs|gp_|Geom_" app/src/rebarsmart app/src/drawing
```

结果：

```text
无匹配。
```

## xhigh 只读 review

本节点涉及代码和测试，按长期 goal 要求执行 xhigh 只读 review。

结论：

```text
Verdict: Pass
Critical: None
Important: None
Minor:
  1. 文档中的 OCCT 泄漏检查命令包含不存在的 app/src/domain。
```

处理：

```text
Minor 已修正：
  泄漏检查命令改为 app/src/rebarsmart app/src/drawing。
```

复验：

```text
tsrs_topology_binding_registry_tests = 1/1 passed
默认 CTest = 22/22 passed
OCCT / AIS 泄漏检查 = 无匹配
```

## 已关闭的 TODO-020B GAP

```text
TODO020B-GAP-003:
  edge fingerprint endpoint 顺序可能受 orientation 影响。
  -> TODO-020D 已通过 endpoint canonicalization 关闭。

TODO020B-GAP-008:
  restore 未使用 fallback 字段。
  -> TODO-020D 已通过 geometryFingerprint / localIndex / bbox fallback 关闭。

TODO020B-GAP-009:
  edge endpoint fingerprint 尚未 canonicalize。
  -> TODO-020D 已关闭。
```

## 剩余风险

| ID | 风险 | 处理 |
| --- | --- | --- |
| TODO020D-GAP-001 | fallback bbox 容差 `1e-5` 尚未用真实复杂工程 STEP 做充分统计。 | 后续 GC-002 扩展真实工程样本验证。 |
| TODO020D-GAP-002 | face fingerprint 仍未包含 outer wire / inner wire / parent solid。 | 后续 TopologyBinding P2 或真实 collision 后再扩展。 |
| TODO020D-GAP-003 | fallback 只能降低 stableId 漂移风险，不能证明复杂模型无碰撞。 | TODO-020E 接入 StepSession 后继续积累真实样本。 |
| TODO020D-GAP-004 | Registry 仍未接入导入会话主链路。 | 下一节点 TODO-020E 处理。 |

## 下一步

下一节点：

```text
TODO-020E StepSession / ImportedModelStore 主链路
```

目标：

```text
把 ShapeStore 与 TopologyBindingRegistry 接入导入会话主链路，
让后续 MainWindow / Viewer 只拿 session / display 边界，
避免 Viewer 选择绕过 stable binding。
```
