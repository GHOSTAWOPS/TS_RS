# TODO-006A 旧实现项目 Qt / OCCT 可复用审计

## 结论

```text
TODO-006A 已完成。

旧实现项目可以作为 Qt / OCCT / STEP / Viewer / 选择 / IO 的工程零件库。
旧实现项目不能作为 TS_RS 的钢筋生成业务来源。
```

白话结论：

```text
能借地基，不能借钢筋脑子。

CMake、STEP 导入、AIS Viewer、选择稳定 ID、边界扫描、Detail / 工程 IO
这些有明显参考价值。

旧项目里的 RebarGroupCreator、RebarLineGroupCommandHandler、
LegacyUiCommandMap 钢筋创建路线、RebarAisPresentationAdapter
仍然是旧 VisualTS / OCCT 直接生成钢筋方向，禁止迁入 TS_RS 主线。
```

TS_RS 的钢筋生成真相仍然是：

```text
RebarSmart PDF / INI / CATNls / CATRsc / IDA / 运行确认。
```

老图石 / VisualTS 的定位仍然是：

```text
Detail 包、工程图字段、下料表、CAD 插件兼容证据。
```

## 审计范围

旧实现项目：

```text
C:\Users\ghost\Desktop\reverse_engineering\【03】图石软件\【图石钢筋1比1复刻】\app
```

本轮审计重点：

```text
1. CMake / Qt6 / OCCT 依赖发现。
2. STEP / STP 导入。
3. XCAF / TopoDS 遍历。
4. AIS Viewer / Qt 窗口嵌入。
5. 点 / 边 / 面 / 实体选择和 stable id。
6. LegacyGeometryAdapter 中可拆出的 OCCT 几何能力。
7. Detail writer / 工程 IO / binding 验证。
8. rebar 业务代码的禁迁边界。
```

审计方式：

```text
只读静态审计。
未修改旧实现项目。
未迁移代码。
未运行旧实现项目测试。
```

## A. 可直接复用

### A1. CMake 的 Qt6 / OCCT 依赖清单

可复用内容：

```text
Qt6 Core / Gui / Widgets / OpenGL / OpenGLWidgets / Test。
OpenCASCADE CONFIG。
STEP / XCAF 所需 OCCT 库清单。
Viewer 所需 OCCT 库清单。
```

证据：

```text
旧 app/CMakeLists.txt:16
旧 app/CMakeLists.txt:17
旧 app/CMakeLists.txt:44-68
旧 app/CMakeLists.txt:108-121
旧 app/CMakeLists.txt:165-182
```

迁移口径：

```text
可直接作为 TS_RS app 后续 CMake 扩展参考。
但不要原样保留旧 target 名称 tsrebar_*。
```

### A2. STEP-only 导入入口骨架

可复用内容：

```text
.stp / .step 后缀校验。
STEPCAFControl_Reader。
XCAFApp_Application + MDTV-XCAF 文档。
ReadFile -> Transfer 链路。
基础异常转 diagnostic。
```

证据：

```text
旧 app/src/geometry/occ/import/OcctStepImportService.cpp:40-44
旧 app/src/geometry/occ/import/OcctStepImportService.cpp:135-143
旧 app/src/geometry/occ/import/OcctStepImportService.cpp:151-172
旧 app/src/geometry/occ/import/OcctStepImportService.cpp:198-204
```

迁移口径：

```text
TODO-016 / STEP 导入阶段可按这个骨架迁入。
```

### A3. STEP 导入摘要和基础拓扑计数

可复用内容：

```text
solids / faces / edges / vertices 计数。
free shape 数。
length unit 读取。
readOk / transferOk / ok / error 诊断结构。
```

证据：

```text
旧 app/src/import/StepImportResult.h
旧 app/src/geometry/occ/import/OcctStepImportService.cpp:86-100
旧 app/src/geometry/occ/import/OcctStepImportService.cpp:164-170
旧 app/src/geometry/occ/import/OcctStepImportService.cpp:178-195
```

迁移口径：

```text
可以作为 STEP 导入 smoke test 的最小验收输出。
固定数量断言只能绑定特定 fixture 和 OCCT 版本，不可泛化为业务规则。
```

### A4. STEP 导入探针和 domain/rebar 边界扫描

可复用内容：

```text
step_import_probe 这类命令行 smoke gate。
domain/rebar 禁止 TopoDS / AIS / BRep / gp_ / Geom_ 泄漏的结构扫描。
```

证据：

```text
旧 app/src/step/StepImportProbe.cpp
旧 app/tests/integration/test_step_import_probe.py
旧 app/tools/check_domain_rebar_boundary.py:7-27
旧 app/tools/check_domain_rebar_boundary.py:30-58
```

迁移口径：

```text
边界扫描应尽快迁入 TS_RS。
后续代码节点每次涉及 domain/rebar 都要跑。
```

## B. 可改造复用

### B1. XCAF / TopoDS 遍历 -> ShapeStore / TopologyBinding

旧实现能力：

```text
用 XCAF free shapes 建 part。
保存 part stableEntry。
保存 TopoDS_Shape。
```

证据：

```text
旧 app/src/geometry/occ/import/OcctStepImportService.cpp:102-107
旧 app/src/geometry/occ/import/OcctStepImportService.cpp:178-195
```

改造要求：

```text
1. TS_RS 中命名为 StepImporter / ShapeStore / TopologyBinding。
2. 补 assembly 层级、location、name、color/layer 的后续扩展位。
3. stableEntry 不能单独作为长期稳定承诺。
4. 需要补 fingerprint、sourceStepId、OCCT 版本 / import policy 记录。
```

### B2. OccSelectionIndex -> TopologyBindingIndex

旧实现能力：

```text
TopExp::MapShapes 建 solid / face / edge / vertex 索引。
从 TopoDS_Shape 反查稳定 ref。
从 stable id 解析并 resolve 回 TopoDS_Shape。
```

证据：

```text
旧 app/src/geometry/occ/selection/OccSelectionIndex.cpp:47-69
旧 app/src/geometry/occ/selection/OccSelectionIndex.cpp:89-111
旧 app/src/geometry/occ/selection/OccSelectionIndex.cpp:114-190
旧 app/src/geometry/legacy/LegacySelectionRef.cpp:110-125
旧 app/src/geometry/legacy/LegacySelectionRef.cpp:128-190
```

改造要求：

```text
1. 不再使用 LegacySelectionRef 作为 TS_RS 长期内部主键名。
2. 新名建议：StableSelectionRef / TopologyRef / TopologyBindingRef。
3. stable id schema 应从 selection-v1 升级为 tsrs-topology-v1 或等价命名。
4. Viewer 只输出结构化 ref，UI 层负责文案。
```

### B3. OccViewerWidget 基础 Viewer 切片

旧实现可参考：

```text
Qt native window 绑定。
V3d_Viewer / AIS_InteractiveContext / V3d_View 初始化。
STEP parts -> AIS_Shape 显示。
FitAll / ZFitAll。
选择模式切换。
鼠标点击 -> StdSelect_BRepOwner -> TopoDS_Shape -> stable ref。
```

证据：

```text
旧 app/src/presentation/occ/OccViewerWidget.cpp:35-43
旧 app/src/presentation/occ/OccViewerWidget.cpp:50-90
旧 app/src/presentation/occ/OccViewerWidget.cpp:152-190
旧 app/src/presentation/occ/OccViewerWidget.cpp:268-299
旧 app/src/presentation/occ/OccViewerWidget.cpp:309-330
旧 app/src/presentation/occ/OccViewerWidget.cpp:345-396
旧 app/src/presentation/occ/OccViewerWidget.cpp:399-419
旧 app/src/presentation/occ/OccViewerWidget.cpp:453-470
```

改造要求：

```text
1. 不能整类复制。
2. 必须剥离 displayRebarPresentation。
3. 必须补 TS_RS 颜色策略：
   - STEP 模型统一水泥灰。
   - 选中点 / 边 / 面红色。
   - 钢筋橙黄。
   - 高亮洋红。
4. 必须补 Viewer 级显式高亮 / selected 状态。
5. 必须重新实现显示模式和剖切，不要把旧命令登记误认为已实现。
```

### B4. OccLegacyGeometryAdapter -> IGeometryEngine / OcctGeometryEngine

旧实现可参考能力：

```text
edge / face 几何摘要。
bbox。
采样。
曲线长度。
最近点 / 投影。
edge split。
wire chain。
face section preview。
edge offset preview。
edge circular sweep preview。
```

证据：

```text
旧 app/src/geometry/occ/legacy_adapter/OccLegacyGeometryAdapter.h
旧 app/src/geometry/occ/legacy_adapter/OccLegacyGeometryAdapter.cpp:701-759
旧 app/src/geometry/occ/legacy_adapter/OccLegacyGeometryAdapter.cpp:984-1050
旧 app/src/geometry/occ/legacy_adapter/OccLegacyGeometryAdapter.cpp:1973-2018
旧 app/src/geometry/occ/legacy_adapter/OccLegacyGeometryAdapter.cpp:2095-2156
旧 app/tests/integration/legacy_geometry_adapter_tests.cpp
```

改造要求：

```text
1. 只迁 OCCT 几何能力，不迁 Legacy DTO / VisualTS 命名。
2. TS_RS 生成器只能依赖 IGeometryEngine。
3. OCCT include 只能在 geometry/occt 或 presentation/occ。
4. edgeCircularSweepPreview 只能作为几何能力参考，不能变成“选边直接生成钢筋”的业务路线。
```

### B5. 纯 DTO / 统计 / Detail / 工程 IO 思路

可参考内容：

```text
SteelData / SteelBar / SteelBarGroup / SteelBarSegment 的聚合形状。
RebarScheduleService 的统计聚合思路。
DetailWriter 的 Detail.xml + DetailNN.stl 写包、校验、失败保留旧包。
TsRebarProjectRuntime 的 manifest / JSON 包 / binding 验证 / save-open 事务。
```

证据：

```text
旧 app/src/domain/rebar/RebarDomainTypes.h
旧 app/src/domain/rebar/SteelData.h
旧 app/src/domain/rebar/SteelBarGroup.h
旧 app/src/domain/rebar/SteelBar.h
旧 app/src/domain/rebar/SteelBarSegment.h
旧 app/src/domain/rebar/RebarScheduleService.h
旧 app/src/domain/rebar/RebarScheduleService.cpp
旧 app/src/drawing/detail/DetailWriter.h:83-90
旧 app/src/drawing/detail/DetailWriter.cpp:118-137
旧 app/src/drawing/detail/DetailWriter.cpp:265-326
旧 app/src/drawing/detail/DetailWriter.cpp:1064-1110
旧 app/src/project/runtime/TsRebarProjectRuntime.h:13-93
旧 app/src/project/runtime/TsRebarProjectRuntime.cpp:24-34
旧 app/src/project/runtime/TsRebarProjectRuntime.cpp:92-123
旧 app/src/project/runtime/TsRebarProjectRuntime.cpp:795-890
旧 app/src/project/runtime/TsRebarProjectRuntime.cpp:1237-1304
```

改造要求：

```text
1. DTO 只能作为字段形状参考，不能照搬旧业务语义。
2. legacyRaw / legacyCommand / createCommand 等字段只能作为 audit metadata，
   不能成为 RebarSmart 生成判断依据。
3. Detail 字段是导出兼容字段，不是内部业务真相。
4. 工程 IO 的 binding 验证和失败保留旧包机制值得迁。
```

## C. 禁止迁入

### C1. RebarGroupCreator 整条旧生成链

禁止原因：

```text
它按旧 VisualTS / LegacyGeometryReader / selection ref 创建 line group / arc group。
这不是 RebarSmart GenerateRebarXXXImp 证据驱动的生成逻辑。
```

证据：

```text
旧 app/src/domain/rebar/RebarGroupCreator.h:92-103
旧 app/src/domain/rebar/RebarGroupCreator.h:105-143
旧 app/src/domain/rebar/RebarGroupCreator.cpp:324-365
旧 app/src/domain/rebar/RebarGroupCreator.cpp:465-638
旧 app/tests/unit/rebar_group_creator_tests.cpp
```

### C2. RebarLineGroupCommandHandler

禁止原因：

```text
它从 LegacySelectionRef 取 edge，调用 RebarGroupCreator，
然后直接把生成结果追加到 SteelData。
这是旧 line group 命令链，不是 TS_RS RebarSmart 生成器入口。
```

证据：

```text
旧 app/src/command/RebarLineGroupCommandHandler.cpp:113-136
旧 app/tests/unit/rebar_line_group_command_handler_tests.cpp
```

### C3. LegacyUiCommandMap 中的旧钢筋创建路线

禁止原因：

```text
它记录旧 VisualTS 命令和实现状态。
TS_RS 的钢筋页签要按 RebarSmart 生成器组织，不按旧 VisualTS 命令当 P0 真相。
```

证据：

```text
旧 app/src/command/LegacyUiCommandMap.cpp:303-327
旧 app/src/command/LegacyUiCommandMap.cpp:400-405
```

### C4. RebarAisPresentationAdapter 原样迁入

禁止原因：

```text
它直接从旧 SteelData 构造 TopoDS_Shape / AIS_Shape。
如果原样搬，会把旧业务对象和 OCCT 显示形状耦合成事实源。
```

证据：

```text
旧 app/src/presentation/occ/RebarAisPresentationAdapter.h:3-22
旧 app/src/presentation/occ/RebarAisPresentationAdapter.cpp:79-132
旧 app/src/presentation/occ/OccViewerWidget.cpp:3
旧 app/src/presentation/occ/OccViewerWidget.cpp:103-149
```

TS_RS 后续正确做法：

```text
RebarSmart Generator -> RebarModel -> presentation/occ/RebarPresentationAdapter
```

这个 adapter 可以使用 OCCT，但不能反向定义钢筋业务字段。

### C5. OccLegacyGeometryAdapter 整文件迁入

禁止原因：

```text
文件很大，混合了旧 VisualTS legacy DTO、旧命令语义和 OCCT API。
只能按函数拆出几何能力，不能整体迁成 TS_RS 核心。
```

证据：

```text
旧 app/src/geometry/occ/legacy_adapter/OccLegacyGeometryAdapter.cpp:3-37
旧 app/src/geometry/occ/legacy_adapter/OccLegacyGeometryAdapter.cpp:1973-2018
旧 app/src/geometry/occ/legacy_adapter/OccLegacyGeometryAdapter.cpp:2095-2156
```

### C6. 旧生成链测试作为 TS_RS 业务验收

禁止原因：

```text
旧测试验证的是“从 legacy edge / old command 创建 SteelData”。
TS_RS 业务验收必须验证 RebarSmart 参数、分布、分区、导引、弯钩和默认值。
```

证据：

```text
旧 app/tests/unit/rebar_group_creator_tests.cpp
旧 app/tests/unit/rebar_line_group_command_handler_tests.cpp
旧 app/tests/integration/line_group_display_smoke_tests.cpp
```

## 迁移顺序建议

推荐顺序：

```text
1. 先迁 / 改造 domain/rebar 边界扫描。
2. 迁 CMake 中 Qt6 / OCCT / STEP libs 经验。
3. 迁 STEP 导入骨架，改掉固定 C:/Temp。
4. 建 ShapeStore + TopologyBinding。
5. 改造 OccSelectionIndex 为 TopologyBindingIndex。
6. 抽最小 OccViewerWidget：
   - 初始化
   - STEP 水泥灰显示
   - FitAll
   - 点 / 边 / 面选择
   - stable ref 回显
7. 再做 IGeometryEngine / OcctGeometryEngine。
8. RebarSmart 纯算法完成后，才接 FixDistance / FixNumber 生成器。
9. Detail / 工程 IO 后续按 M5 迁移思想，不提前污染 RebarModel。
```

## 对 todo.csv 的影响

本轮完成后：

```text
TODO-006A -> done
TODO-007 -> next
```

理由：

```text
旧实现项目的 Qt / OCCT 可复用边界已经明确。
下一步应回到 RebarSmart 纯算法层，先实现 SpaceListParser。
```

明确不插入的事项：

```text
不先迁 Viewer。
不先迁 STEP import。
不先迁 Detail writer。
不先迁旧 rebar DTO。
```

原因：

```text
TODO-007 到 TODO-010 是纯算法层，
不接 Qt，不接 OCCT，不接 Detail。
先把 RebarSmart 分布算法闭环，后面 GeometryEngine / UI 才不会空转。
```

## 验证记录

本轮验证：

```text
1. git status 基线干净。
2. 本轮开始时 todo.csv 中 TODO-006A 是唯一 next。
3. 旧实现项目 app 源码已只读扫描。
4. 三个 xhigh 只读子代理完成并关闭：
   - CMake / STEP / XCAF 审计。
   - Viewer / 选择 / AIS 审计。
   - rebar / Detail / 工程 IO 禁迁审计。
```

本轮未做：

```text
未修改 C++。
未迁移旧实现代码。
未运行 CTest。
未运行旧实现项目 app。
```

## 残余风险

### R1. stableEntry + subShapeIndex 稳定性仍需实测

风险：

```text
旧实现稳定 ID 主要依赖 XCAF label entry + TopTools index。
跨 OCCT 版本、装配层级、STEP 重新导出后可能漂移。
```

后续动作：

```text
TODO-016 / TODO-017 阶段补真实 STEP 样本 reimport gate。
```

### R2. 旧 Viewer 没有完整高亮 / 显示模式 / Viewer 剖切

风险：

```text
旧实现有选择模式和 pick ref，但未完整实现 TS_RS 需要的红色选中、
洋红高亮、线框 / 渲染 / 上色、XYZ 剖切和剖切面旋转。
```

后续动作：

```text
TODO-015 到 TODO-017 按 TS_RS UI 文档重新实现，不把旧命令登记当完成。
```

### R3. 旧 DTO 字段可能把旧路线伪装成元数据回流

风险：

```text
legacyRaw / legacyCommand / createCommand / sourceCurveIds 这类字段
很容易从 audit metadata 变成业务判断依据。
```

后续动作：

```text
RebarSmart DTO 另建，不原样搬旧 SteelData。
旧字段如需保留，只放 evidence / audit / compatibility 区。
```

### R4. Detail writer 字段不是内部业务真相

风险：

```text
旧 DetailWriter 字段高度贴近旧 XML。
如果提前迁入，可能让 Detail 包反向污染 RebarModel。
```

后续动作：

```text
M5 阶段先建 DetailPackageReader/Writer/Exporter 边界，
输入来自 DrawingModel / RebarModel / ScheduleModel。
```

## 最终判断

```text
旧实现项目不是废料。
它是 Qt / OCCT 工程零件库。

但它也不是 TS_RS 的业务母体。
TS_RS 的钢筋创建逻辑必须从 RebarSmart 证据重新实现。
```
