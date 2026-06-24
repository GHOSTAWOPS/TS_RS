# TS_RS 长期 Goal 执行目标

本文档是后续 goal 模式的固定执行入口。

## 一句话目标

```text
持续完成 TS_RS 正式开发前准备工作，
直到具备稳定进入 RebarSmart 纯算法、Qt/OCCT、工程图/Detail 开发主线的条件。
```

## 当前主线

```text
新项目：
  C:\Users\ghost\Desktop\reverse_engineering\【03】图石软件\【TS_RS】

旧实现项目：
  C:\Users\ghost\Desktop\reverse_engineering\【03】图石软件\【图石钢筋1比1复刻】

RebarSmart 证据目录：
  C:\Users\ghost\Desktop\reverse_engineering\【03】图石软件\RebarSmart_DrwToolKit2022X 20260604
```

核心路线：

```text
TS_RS 是 STEP-only 独立桌面程序。
Qt6 负责 UI。
OCCT 负责 STEP/STP、显示、选择、几何计算和钢筋实体显示。
RebarSmart 作为钢筋生成逻辑证据源。
老图石 / VisualTS 只作为 Detail 包、工程图字段、CAD 插件兼容证据源。
```

## 不变边界

禁止：

```text
不调用 RebarSmart DLL。
不调用 VisualTS EXE / DLL。
不依赖 3DE / CAA / CATIA / ACIS / HOOPS / Codejock 商业运行时。
不把旧实现项目【图石钢筋1比1复刻】中的 rebar 业务逻辑迁入 TS_RS。
不让 domain/rebar 依赖 TopoDS / AIS / BRep / TopAbs。
不用“OCCT 能怎么做”替代“RebarSmart 证据是什么”。
```

允许：

```text
参考旧实现项目的 Qt / OCCT 工程零件。
参考旧实现项目的 CMake、STEP 导入、AIS Viewer、选择/高亮、剖切、稳定 ID 思路。
在 TS_RS 的 geometry/occt 或 presentation/occ 内部使用 OCCT。
用 IDA MCP、PDF、INI、CATNls/CATRsc、运行确认闭合 RebarSmart 算法证据。
```

## 开发前准备成功标准

必须全部满足：

```text
1. 旧实现项目可复用边界明确。
2. Qt / OCCT 构建和迁移路线明确。
3. app 工程骨架稳定 configure / build / ctest。
4. RebarSmart 纯算法 TODO 顺序明确。
5. SpaceListParser / Distribution / ZoneCalculator 的测试入口明确。
6. RebarSmart 字段、单位、枚举、算法 GAP 可追溯。
7. Detail 包路线保持为老图石兼容导出，不反向污染 RebarModel。
8. todo.csv 每轮只有一个 status=next。
9. 每轮都有文档、验证、commit、annotated tag、push。
10. 外部技术评审指出的 P0A / Detail / TopologyBinding / Centerline 命名边界已持续执行。
```

## 每轮执行规则

```text
1. 先读本文件。
2. 读 todo.csv，选择唯一 status=next 的任务。
3. 读该任务 evidence 指向的文档。
4. 明确本轮边界：只读 / 文档 / 代码 / 测试 / 构建。
5. 只完成一个 next 节点。
6. 节点验证、review、commit、tag、push 完成后，自动进入下一个 next。
7. 直到遇到明确阻塞条件才停止并汇报。
```

如果当前 next 是代码、测试或构建脚本节点：

```text
1. 先补测试，再实现。
2. 运行相关测试和默认 CTest。
3. commit 前必须执行 xhigh 只读 review。
4. Critical / Important 必须修复，或写明技术反驳理由。
5. 子代理完成后必须关闭。
6. commit / annotated tag / push 后，默认不执行 Web GPT Pro 外部审查。
7. 只有用户明确恢复外部高级 AI 审查时，才按外部高级 AI 审查规则打包并执行 web-gpt-pro-review。
8. 已取得的外部审查若出现 Critical / Important，必须先技术核验，再修复或写明反驳理由。
9. 外部审查修复完成后，追加 follow-up commit / tag / push，并在必要时复审。
```

如果当前 next 是只读审计或文档节点：

```text
1. 不修改代码。
2. 需要时可使用 xhigh 只读 review。
3. 输出结论必须落到 docs/。
4. todo.csv 必须推进到下一个明确节点。
5. 若节点影响路线、架构或关键门禁，先完成本地文档和 xhigh review；Web GPT Pro 外部审查当前暂停，除非用户明确恢复。
```

## 外部高级 AI 审查规则

当前执行口径：

```text
Web GPT Pro 外部审查暂停。
原因：用户明确要求“后续先不用 web gpt pro 审查了”。
暂停期间不要主动打包、上传或监听外部审查。
当前门禁改为：本地测试 + 默认 CTest + xhigh 只读 review + 文档记录 + commit/tag/push。
只有用户明确说恢复外部审查时，才重新启用下方规则。
```

历史规则保留如下，作为恢复外部审查时的执行合同。

TS_RS 的 P0 / 架构 / 协议 / 真实环境门禁节点，在外部审查恢复后，除本地测试和 xhigh 只读 review 外，
应在节点 checkpoint 后执行 Web GPT Pro 外部审查。

触发条件：

```text
1. P0 代码节点。
2. Detail / STEP / TopologyBinding / CommandService / RebarModel / .tsrebar 等共享边界节点。
3. 路线、ADR、长期 goal、todo 顺序发生实质调整。
4. 用户明确要求外部高级 AI 审查。
```

执行顺序：

```text
1. 本地实现完成。
2. 相关测试通过。
3. 默认 CTest 通过。
4. xhigh 只读 review 通过；子代理完成后关闭。
5. commit。
6. 打 annotated tag。
7. push main 和 tag。
8. 生成 external_review_packages/ 或 review_packages/ 下的审查包。
9. 审查包必须包含：
   - README_REVIEW.md
   - EXTERNAL_REVIEW_PROMPT.md
   - 当前 commit / tag / GitHub URL
   - 关键源码、测试、文档
   - git/head_full_patch.txt
   - git/head_stat.txt
   - 验证命令与结果
   - todo.csv 当前状态
10. 使用 web-gpt-pro-review skill 发送到用户指定的 ChatGPT 对话。
11. 等待外部审查完成，不重复刷新，不重复发送。
12. 如果外部审查对话掉线、页面丢失、没有返回完整评审或无法确认最新回复，
    必须记录为 dropped / no-result，不能当作通过，也不能用来关闭 Critical / Important。
13. 将外部审查结果或掉线记录落到 docs/external_reviews/。
14. 审查记录 md 必须包含：
   - 审查时间
   - 审查 todo / commit / tag
   - ChatGPT 对话 URL
   - 上传审查包路径
   - 外部审查状态：completed / dropped / blocked
   - 外部审查原文或关键摘录；若 dropped，则写明掉线原因和未取得结论
   - Critical / Important / Minor 清单
   - 每条审查 TODO 的处理状态：fixed / rebutted / deferred / blocked
   - 修复 commit / tag（如有）
15. Critical / Important 必须修复或写明技术反驳理由。
```

web-gpt-pro-review 固定要求：

```text
0. 默认外部审查对话：
   https://chatgpt.com/c/6a28c1d7-c50c-83ec-8bf4-29c4df9ea152
1. 使用用户当前登录的普通 Chrome。
2. 使用 Chrome DevTools MCP 控制当前 Chrome，不默认使用 agent-browser。
3. 上传路径优先使用 ChatGPT 原生：
   + / 添加文件等 -> 添加照片和文件。
4. 上传前核对 ChatGPT URL conversation id。
5. 不上传额外文件。
6. 不在回答生成中刷新页面或重发。
7. 首次等待 5 分钟再检查状态；之后每 3 分钟检查一次。
8. 抽取最新 assistant 回复，不混合旧回复。
```

外部审查提示词必须要求：

```text
1. 不要总结，要挑刺。
2. 按 Critical / Important / Minor 输出。
3. 每条问题给出证据、影响、建议。
4. 明确路线是否偏离。
5. 明确是否允许进入下一个 todo 节点。
6. 明确哪些必须在下一步前修复，哪些可以后置。
```

## 当前状态

```text
正常情况下以 todo.csv 中唯一 status=next 的任务为准。
当前 next：TODO-025 RebarModel minimal transaction。
TODO-024 已完成：
v2_empty_groups minimal sheet 已通过旧图石 AutoCAD 插件导入按钮路径人工验证。
done 只表示 minimal sheet DetailNN.stl protocol reached manual autoin pass。
TODO-020D 已完成外部审查整改：
TopologyBindingRegistry 当前是 P1 partial hardening，不是 production-ready graph binding。
stableId 漂移时只允许 geometryFingerprint 唯一候选自动 fallback。
localIndex / bbox 只作为 BINDING_LOW_CONFIDENCE 诊断线索，不能静默自动恢复。
已补 realistic compound / fused L-shape STEP stress tests。
TODO-020E 已完成：
StepSession / ImportedModelStore 已成为 STEP 导入会话主链路。
MainWindow 导入 STEP 后会持有 currentStepSessionId。
TODO-020G 已完成：
SelectionCommandService 已提供 stable binding 出口。
DetailPackageCommandService / RebarCreationCommandService 仍是 not implemented guardrail。
结构扫描测试已阻止 UI 直接调 generator / Detail writer。
TODO-021 已完成：
ViewerSelectionMode 已接入 OccViewerWidget。
MainWindow smoke selection 已通过 SelectionCommandService 输出 stable binding id。
真实鼠标 picking 和视觉高亮截图仍作为后续风险。
TODO-020F 已完成：
StepLengthUnitPolicy 已接入 StepImportResult / StepImportCommandResult / StepSession。
TS_RS 内部长度单位固定为 m。
本轮没有缩放 OCCT shape 坐标。
```

目标：

```text
按外部审查建议拆开阻塞边界：
TODO-020E / TODO-020G 已完成。

TODO-021 Viewer 选择系统已完成 P0 stable binding 出口。
generator->Detail 闭环仍需 RebarModel / CommandService / Detail 映射后续节点，
不因 TODO-024 done 直接放行。
```

已完成：

```text
TODO-022 DetailPackageReader P0。
已使用真实 todo66 fixture manifest 校验 hash。
已读取 Detail.xml + Detail01..04.stl。
已保留 rawXml / 原文件名 / sheetIndex。
已输出 knownSummary 统计。

TODO-023 DetailPackageWriter round-trip。
Reader -> Writer preserve-mode rawXml passthrough 已完成。
写出后 Reader 能再读。
文件数量、knownSummary、未知节点 / 属性 P0 统计不减少。

TODO-024 极简 Detail 包生成。
v2_empty_groups minimal sheet 已生成并通过人工导入验证。
GC-004 状态为 manual_autoin_passed_v2。
```

输出：

```text
docs/architecture/18_TODO-023_DetailPackageWriterRoundTrip实现记录.md
docs/architecture/19_TODO-024_MinimalDetailPackage实现记录.md
docs/validation/golden_cases/GC-004/
```

边界：

```text
不实现钢筋业务。
不接 Viewer。
不接钢筋生成器。
不把 Detail 字段反向污染 RebarModel。
不创建 DrawingModel / RebarModel 映射。
不宣称 CAD 插件兼容已完成。
```

TODO-006A 审计已完成：

```text
docs/architecture/03_旧实现项目QtOCCT可复用审计.md

结论：
  旧实现项目可作为 Qt / OCCT 工程零件库。
  旧实现项目不能作为 TS_RS 钢筋生成业务来源。
```

## 推荐后续节点

已完成：

```text
TODO-008：
  PriorityCountDistributor / PrioritySpaceDistributor。

实现记录：
  docs/architecture/05_TODO-008_PriorityDistributors实现记录.md

TODO-009：
  PriorityListDistributor。

实现记录：
  docs/architecture/06_TODO-009_PriorityListDistributor实现记录.md

TODO-010：
  GuideSurfaceOffsetCalculator / GuideCurveZoneCalculator。

实现记录：
  docs/architecture/07_TODO-010_ZoneCalculators实现记录.md

TODO-011：
  IGeometryEngine 接口设计。

实现记录：
  docs/architecture/08_TODO-011_IGeometryEngine接口实现记录.md

TODO-012：
  OcctGeometryEngine P0。

实现记录：
  docs/architecture/09_TODO-012_OcctGeometryEngineP0实现记录.md

TODO-013：
  FixDistanceCenterlineGenerator P0。

实现记录：
  docs/architecture/10_TODO-013_FixDistanceGeneratorP0实现记录.md

TODO-014：
  FixNumberCenterlineGenerator P0。

实现记录：
  docs/architecture/11_TODO-014_FixNumberGeneratorP0实现记录.md

TODO-015：
  Qt6 OCCT 主窗口骨架。

实现记录：
  docs/architecture/12_TODO-015_Qt6_OCCT主窗口骨架实现记录.md

TODO-016：
  老图石 Ribbon 外壳与命令映射收口。

输出：
  docs/roadmap/02_外部技术评审核心修正.md
  docs/roadmap/03_P0A最小闭环与前置验证门禁.md
  docs/ui/13_老图石Ribbon外壳对齐规格.md
  docs/ui/14_老图石命令到RebarSmart生成器映射表.md
  docs/ui/15_RibbonLite_P0实现规格.md

TODO-017：
  STEP Import + TopologyBinding Spike 计划。

输出：
  docs/step/00_STEP_TopologyBinding_spike_plan.md

TODO-018：
  Detail 包证据索引与 round-trip policy。

输出：
  docs/detail/00_Detail包证据索引.md
  docs/detail/01_DetailXML_observed_schema.md
  docs/detail/02_Detail_roundtrip_policy.md

TODO-019：
  RebarSmart 单位枚举默认值规则。

输出：
  docs/rebarsmart/02_单位枚举默认值规则.md

TODO-022：
  DetailPackageReader P0。

输出：
  docs/architecture/16_TODO-022_DetailPackageReaderP0实现记录.md

2026-06-17 外部严格评审整改补充：

输出：
  docs/architecture/13_ApplicationService_CommandService_边界.md
  docs/geometry/01_IGeometryEngine_P1扩展接口.md
  docs/validation/00_golden_case_strategy.md
  docs/legal/00_reverse_engineering_and_resource_reuse_boundary.md
```

当前 Detail 前置状态：

```text
TODO-024：
  极简 Detail 包生成 + 插件导入验证。

状态：
  done。

输出：
  docs/architecture/19_TODO-024_MinimalDetailPackage实现记录.md
  docs/validation/golden_cases/GC-004/
  docs/detail/03_MinimalDetailPackage_v2_protocol.md

边界：
  v2_empty_groups 是 P0 minimal sheet baseline。
  v3_one_pointstb 只作为后续最小钢筋组兼容样本。
  不表示完整 Detail / StbGroup / StbTable / MaterialTable 兼容完成。
```

中期顺序：

```text
TODO-007 SpaceListParser
TODO-008 PriorityCount / PrioritySpace
TODO-009 PriorityList
TODO-010 GuideSurfaceOffset / GuideCurveZone
TODO-011 IGeometryEngine 接口设计
TODO-012 OcctGeometryEngine P0
TODO-013 FixDistanceCenterlineGenerator P0
TODO-014 FixNumberCenterlineGenerator P0
TODO-015 Qt6 OCCT 主窗口骨架
TODO-016 老图石 Ribbon 外壳与命令映射收口
TODO-017 STEP Import + TopologyBinding Spike 计划
TODO-018 Detail 包证据索引与 round-trip policy
TODO-019 RebarSmart 单位枚举默认值规则
TODO-020 STEP 导入与水泥灰显示
TODO-020B ShapeStore + TopologyBindingRegistry P0
TODO-020C 外部评审条件收口
TODO-022 DetailPackageReader P0
TODO-023 DetailPackageWriter round-trip
TODO-024 极简 Detail 包生成 + autoin 验证
TODO-020D TopologyBindingRegistry P1 partial hardening
TODO-020E StepSession / ImportedModelStore 主链路
TODO-020G CommandService skeleton guardrails
TODO-021 Viewer 选择系统
TODO-020F STEP import unit and scale policy
TODO-025 RebarModel minimal transaction
```

说明：

```text
TODO-013 / TODO-014 的代码文件名仍是 FixDistanceGenerator / FixNumberGenerator，
但当前能力边界只允许称为 Centerline P0。

TODO-020C 是外部评审后的条件收口节点。

TODO-022 / TODO-023 / TODO-024 前置 Detail 读写和极简 autoin 验证，
是为了先验证老图石 CAD 插件兼容出口，
不要直接跳到 Viewer 选择系统。

TODO-024 已闭合到 minimal sheet 级别：
  - TODO-020E / TODO-020G 已完成。
  - TODO-021 已完成 Viewer selection P0 stable binding 出口。
  - generator->Detail 真实闭环仍等待 RebarModel / CommandService / Detail 映射后续节点。

TODO-020E / TODO-020G 已在 TODO-021 前完成。
TODO-021 已通过 SelectionCommandService 输出 stable binding id。

TODO-020F 不阻塞 TODO-021。
它阻塞的是正式 RebarModel / Schedule / generator->Detail 链路，
用于避免 STEP 源单位、TS_RS 内部单位、RebarSmart 参数单位和 Detail 输出单位混用。
TODO-020F 已完成后，当前 next 进入 TODO-025。
```

## 用户可直接粘贴的 Goal 话术

```text
创建并执行长期 goal：

目标：
持续完成 TS_RS 正式开发前准备工作，并按 todo.csv 每轮只完成一个 status=next 节点。

工作目录：
C:\Users\ghost\Desktop\reverse_engineering\【03】图石软件\【TS_RS】

执行入口：
docs/roadmap/01_长期Goal执行目标.md
todo.csv

核心要求：
先读长期 goal 文档、todo.csv 和当前 next 的 evidence 文档。
每轮只做一个 next。
涉及代码/测试/构建脚本时，commit 前必须 xhigh 只读 review。
xhigh 只能 review，不能修改；主流程 agent 负责修改。
子代理完成后必须关闭。
完成后更新文档、todo.csv，运行验证，commit，打 annotated tag，push。
Web GPT Pro 外部审查当前暂停，除非用户明确恢复。
暂停期间以本地测试、默认 CTest、xhigh 只读 review、文档记录、commit/tag/push 作为节点门禁。
已取得的外部审查 Critical / Important 必须修复或写明技术反驳理由。

当前从 todo.csv 中唯一 status=next 的节点开始。
当前 next 为 TODO-025。
TODO-024 已为 done，GC-004 状态为 manual_autoin_passed_v2。

注意：TODO-024 done 不表示完整 Detail / StbGroup / StbTable / MaterialTable 兼容完成。
```
