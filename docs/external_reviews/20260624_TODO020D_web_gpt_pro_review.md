# TODO-020D Web GPT Pro 外部审查记录

## 基本信息

- 时间：2026-06-24 09:12 CST
- TODO：TODO-020D TopologyBindingRegistry P1 hardening
- Commit：`2de4e53fb35ddeb38e825b80224fb5a61ad6ebf3`
- Tag：`tsrs-todo-020d-topology-binding-p1-20260623`
- GitHub：<https://github.com/GHOSTAWOPS/TS_RS.git>
- ChatGPT 对话：<https://chatgpt.com/c/6a28c1d7-c50c-83ec-8bf4-29c4df9ea152>
- 上传包：`review_packages/TODO-020D_web_gpt_pro_review_20260623_170659.zip`
- 状态：completed

## 外审结论

Verdict: CONDITIONAL GO

外审认为 TODO-020D 方向正确，但当前不能宣称为工程级稳定 binding，只能视为
`P1 partial hardening / non-production ready`。进入 TODO-020E 前必须补齐：

1. ambiguity handling。
2. `localIndex + bbox` fallback 降级为 last-resort/debug 语义。
3. 至少 2 个 real/realistic STEP topology tests。

## Critical / Important / Minor 事项

### Critical-1 fallback restore 语义不足

- 反馈：当前 fallback 是链式尝试，存在 first-match-wins 风险。
- 风险：真实 STEP 中重复 edge/face 或相似 bbox 会造成静默误绑定。
- 处理状态：fixed for TODO-020D scope。
- 本地处理结果：`restore()` 保留 geometryFingerprint 唯一候选自动 fallback；
  geometryFingerprint 多候选直接 `BINDING_AMBIGUOUS`，不再用 `localIndex + bbox` 缩窄；纯
  `localIndex + bbox` 唯一候选改为 `BINDING_LOW_CONFIDENCE` 且 `ok=false`；
  多候选返回 `BINDING_AMBIGUOUS`。完整 scoring / graph hash 后置。

### Critical-2 geometryFingerprint 维度偏低

- 反馈：fingerprint 缺 edge adjacency、face loop、vertex connectivity。
- 风险：同 bbox/同长度但拓扑不同会误绑定。
- 处理状态：follow-up。
- 本地处理方向：TODO-020D 不做完整 topology graph hash；补文档说明当前仍是 heuristic
  matcher，并把 graph hash 作为后续真实 STEP GC/TopologyBinding P2。

### Critical-3 endpoint canonicalization 只解决方向，不解决拓扑冲突

- 反馈：只覆盖 A->B 与 B->A，不覆盖同端点多 edge、STEP healing split、seam edge。
- 风险：真实 STEP 中 stableId 仍可能漂移。
- 处理状态：partial fixed / follow-up。
- 本地处理结果：端点方向问题已由 canonicalization 关闭；多候选 ambiguous 已有测试覆盖。
  同端点多 edge、STEP healing split、seam edge clustering 后置到真实 STEP collision 样本驱动。

### Critical-4 bbox tolerance 未验证真实 STEP 分布

- 反馈：`1e-5` 可能过紧或过松。
- 风险：false negative 或 false positive。
- 处理状态：follow-up。
- 本地处理方向：TODO-020D 不引入 adaptive tolerance；补样本统计和 P2 adaptive tolerance
  任务，不把 `1e-5` 宣称为生产结论。

### Important-5 测试偏 box world，缺真实 STEP topology stress

- 反馈：现有测试主要来自 `BRepPrimAPI_MakeBox`。
- 风险：binding correctness 不等于真实工程模型正确。
- 处理状态：fixed with realistic generated stress。
- 本地处理结果：新增 `compound` 和 `fused_lshape` 两个程序生成 realistic STEP stress，
  覆盖 repeated import / restore determinism。它们不是人工业务 golden STEP，真实工程
  GC-STEP-TOPO 样本继续后置。

### Important-6 `localIndex + bbox` 存在误安全

- 反馈：localIndex 在 STEP re-import 中不是稳定保证。
- 风险：静默错误绑定。
- 处理状态：fixed。
- 本地处理结果：纯 `localIndex + bbox` fallback 返回 `BINDING_LOW_CONFIDENCE`，
  `ok=false`，只作为重新选择/人工确认诊断，不静默恢复。

### Important-7 diagnostics 缺 `AMBIGUOUS / MULTI_MATCH / LOW_CONFIDENCE`

- 反馈：诊断不足会让 TODO-021 UI 静默选错。
- 处理状态：partial fixed。
- 本地处理结果：已有 `BINDING_AMBIGUOUS`，新增 `BINDING_LOW_CONFIDENCE`。
  未新增单独 `MULTI_CANDIDATE_MATCH`，当前用 `candidateCount` + `BINDING_AMBIGUOUS`
  表达多候选语义。

### Minor-8 文档完成宣称过绝对

- 反馈：`P1 hardening` 容易被理解成 production ready。
- 处理状态：fixed。
- 本地处理结果：实现记录、roadmap、README 均改为 `P1 partial hardening` 口径，
  明确不宣称 production-ready graph binding。

### Minor-9 tests 未验证 restore determinism

- 反馈：缺 repeated import / multi-run determinism。
- 处理状态：fixed。
- 本地处理结果：新增 compound / fused L-shape repeated import 与 restore determinism 测试。

### Minor-10 ShapeStore integration 未做 cross-check

- 反馈：未来可能出现 binding exists but shape missing。
- 处理状态：follow-up。
- 本地处理方向：这是 TODO-020E 主链路目标，不在 TODO-020D 里提前实现；在 TODO-020E 验收中保留
  ShapeStore/TopologyBindingRegistry consistency check。

## 外审原文

```text
Verdict: CONDITIONAL GO

TODO-020D（TopologyBindingRegistry P1 hardening）方向是对的，但实现存在“表面正确 + 语义不足 + fallback伪完成”的三重风险。
当前不能算工程级稳定 binding，只能算 P1 half-hardening。

Critical
1. fallback restore 是“顺序存在”，但不是“语义可靠恢复”

证据
TopologyBindingRegistry.cpp（核心 restore 逻辑）：

stableId exact match
→ kind match
→ geometryFingerprint match
→ bbox + localIndex fallback

问题
fallback 是“链式尝试”，但缺少：
- confidence score
- multi-candidate disambiguation strategy
- geometryFingerprint 权重规则

当前实现本质是：
first match wins

影响
在真实 STEP（尤其工业装配）中：
edge/face 高重复 + 类似 bbox
→ fallback 会“误绑定但不报错”

这会直接污染：
TODO-021 Viewer selection
Rebar binding
后续 RebarModel（隐性错误）

建议
必须改为：
struct Candidate {
    id
    score
    matchType
};
return bestCandidate if score > threshold else AMBIGUOUS;

2. geometryFingerprint 过于“低维”，无法支撑 P1 hardening

证据
当前 fingerprint（从代码结构推断）：
bbox + length + type + endpoints

问题
缺少关键拓扑维度：
- edge adjacency context
- face loop structure
- vertex connectivity graph
- orientation canonicalization proof

影响
会出现：
same bbox + same length but different topology → mis-binding

建议
至少升级为：
topology graph hash (edge adjacency + orientation normalized)

3. endpoint canonicalization 只解决“方向”，没解决“拓扑冲突”

证据
TopologyBindingRegistry.cpp：
normalize(edge first/last order)

问题
只处理：
A→B vs B→A

但没有处理：
多 edge share same endpoints
split edge after STEP healing
seam edges duplication
tolerance-based merge edges

影响
在 real STEP：
stableId still drifts silently

建议
必须补：
edge clustering + vertex graph grouping

4. bbox fallback tolerance = 1e-5 仍然未验证真实 STEP 分布

证据
tests 使用：
1e-5 tolerance

问题
OCCT + STEP import 实际误差范围：
1e-5 可能过紧（float noise）
也可能过松（small feature collapse）

影响
fallback 会：
false negative（匹配失败）
false positive（错误绑定）

建议
必须引入：
adaptive tolerance (based on model scale)

Important
5. tests 仍然偏“box world”，缺真实 STEP topology stress

证据
test uses:
BRepPrimAPI_MakeBox
STEPControl_Writer

问题
目前测试覆盖：
perfect cube topology

没有：
fillet edge
multi-shell
assembly STEP
duplicated faces
non-manifold edges

影响
当前 binding correctness ≠ real-world correctness

建议
增加：
GC-STEP-TOPO-001 (real STEP assembly)
GC-STEP-TOPO-002 (non-manifold edges)
GC-STEP-TOPO-003 (fillet-heavy model)

6. fallback localIndex + bbox 逻辑存在“误安全”

证据
restore fallback：
localIndex + bbox match

问题
localIndex 在 STEP re-import 中：
is not stable guarantee

影响
会产生：
silent wrong binding (最危险类型)

建议
localIndex 必须降级为：
last-resort debug only, not production binding

7. diagnostics 没有“AMBIGUOUS / MULTI_MATCH / LOW_CONFIDENCE”

证据
目前只有：
OK / MISSING / FALLBACK

问题
缺少：
AMBIGUOUS_BINDING
MULTI_CANDIDATE_MATCH
LOW_CONFIDENCE_MATCH

影响
下一阶段 TODO-021 会：
silently pick wrong edge in UI

Minor
8. TODO-020D 文档“完成宣称”略过于绝对

证据
markdown：
“TODO-020D 完成 P1 hardening”

问题
实际仍：
no scoring system
no ambiguity resolution
no real STEP validation

建议
改为：
P1 partial hardening (non-production ready)

9. tests 没验证 “restore determinism”

问题
当前只测：
match success
basic drift

没测：
repeated import stability
multi-run determinism

10. ShapeStore integration 假设“已稳定”，但未做 cross-check

问题
TODO-020D 没有验证：
ShapeStore ↔ TopologyBindingRegistry consistency

风险
future TODO-021 会出现：
binding exists but shape missing
or shape exists but binding missing

是否可以进入 TODO-020E？
Verdict: CONDITIONAL GO

必须先修（进入 TODO-020E 前）
1. 必须增加 ambiguity handling
2. localIndex fallback 必须降级为 debug
3. 必须增加至少 2 个 real STEP topology tests

可以后置
scoring system优化
full graph matching
adaptive tolerance refinement
performance optimization

最终建议
GO to TODO-020E (conditionally)
BUT:
必须先补 ambiguity + real STEP tests
否则 TODO-021 会崩在 UI 层

一句话外部审查总结
TODO-020D 已经从“能跑的 binding”升级到“半结构化 binding”，但还没达到“可用于 UI selection 的确定性拓扑系统”。
```
