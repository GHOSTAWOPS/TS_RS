# TS_RS 新技术方案包严格评审与整改建议

> 面向：TS_RS 开发人员、Codex Goal 执行人员、项目负责人  
> 评审日期：2026-06-17  
> 评审对象：`TS_RS_external_tech_review_20260616_run001.zip` 技术方案包、RebarSmart3DE 逆向分析文档、前期确定的项目主线  
> 评审口径：外部技术评审 / 架构审查 / 风险前置审查  
> 结论级别：**有条件通过，不建议按当前 todo 原样继续推进**

---

## 0. 执行摘要

当前方案的总方向是正确的：

```text
STEP/STP
  -> OCCT 导入、显示、选择、拓扑绑定、几何计算
  -> RebarSmart-style 钢筋生成器
  -> RebarModel / ScheduleModel / DrawingModel
  -> 老图石 Detail 包兼容导出
  -> 可选使用老图石 CAD 插件导入
```

但方案不能直接放行。当前包里最主要的问题不是“路线错了”，而是：

```text
1. UI 外壳还没有真正冻结为老图石 Ribbon/功能入口。
2. P0 范围过大，8 个钢筋创建命令不适合作为第一闭环。
3. Detail 包验证放得太晚，这是最大外部兼容风险。
4. STEP TopologyBinding 风险没有足够前置。
5. RebarSmart 参数单位、枚举、默认值还没闭合，但 UI 一致性承诺过满。
6. IGeometryEngine P0 接口过窄，和 FixDistance/FixNumber 的真实目标存在落差。
7. 老图石命令名到 RebarSmart generator 的桥接表缺失。
8. RibbonLite 没有明确实现规格，可能拖成 UI 黑洞。
```

**建议继续推进，但必须先重排近期 todo：**

```text
允许继续 TODO-012 OcctGeometryEngine P0；
但 TODO-012 后不要直接进入 TODO-013 FixDistanceGenerator。
必须先插入：
  - STEP / TopologyBinding Spike
  - DetailPackageReader + round-trip Spike
  - 极简 Detail 包 autoin 人工验证
  - 老图石 Ribbon 外壳对齐文档
  - 老图石命令 -> RebarSmart Generator 映射表
  - RebarSmart 单位 / 枚举 / 默认值规则文档
```

---

## 1. 评审范围

本次审查覆盖以下包内文件和已上传证据：

```text
AGENTS.md
app_CMakeLists.txt
app_tests_CMakeLists.txt
todo.csv
README_EXTERNAL_REVIEW.md
PROMPT_FOR_EXTERNAL_AI.md

docs/adr/ADR-0001_STEP_only_RebarSmart_Detail路线.md

docs/architecture/00_系统架构总览.md
docs/architecture/01_纯算法层实现计划.md
docs/architecture/08_TODO-011_IGeometryEngine接口实现记录.md

docs/rebarsmart/00_证据索引.md
docs/rebarsmart/01_GenerateRebarData字段地图.md

docs/roadmap/00_路线图.md
docs/roadmap/01_长期Goal执行目标.md

docs/ui/00_UI决策记录.md
docs/ui/01_顶层选项卡与功能矩阵.md
docs/ui/05_OCCT视图交互与选择设计.md
docs/ui/08_定距钢筋参数面板与IDA证据.md
docs/ui/09_钢筋页签命令与RebarSmart算法映射.md
docs/ui/10_钢筋创建Dock面板RebarSmart一致性规则.md
docs/ui/11_UI可开发规格.md
docs/ui/12_定数钢筋参数面板与证据.md
```

同时参考 RebarSmart3DE 逆向分析文档中的事实：

```text
RebarSmart3DE 的核心钢筋模块包括：
- AfrRebarCore.dll
- AfrRebarCore3D.dll
- RebarCreation.dll
- AfrRebarSmart.dll

RebarSmart 的类体系包括：
GenerateRebarBase -> GenerateRebarCore -> GenerateRebarFixDistanceImp / FixNumberImp / SweepImp / FrustumImp / CornerImp / HelixImp / StirrupImp / DowelImp / TieImp / CustomizeImp / PolylineImp

GenerateRebarBase 暴露出可剥离的数据模型字段；
部分分布算法、分区算法、间距列表解析属于可单测的纯算法；
3DE 的 CATBody / CATMathPoint / CATMathVector / CATGeoFactory 等需要映射到 OCCT 的 TopoDS_Shape / gp_Pnt / gp_Vec / BRepBuilderAPI_*。
```

---

## 2. 是否和前面确定主线一致

### 2.1 主线一致的部分

当前方案和前面确定的主线基本一致：

| 主线要求 | 当前方案状态 | 评审意见 |
|---|---|---|
| STEP-only | 基本一致 | ADR 明确只支持 STEP/STP，不承诺 SFL/SAT/SAB/DWG/DXF P0。 |
| Qt6 + OCCT 独立运行时 | 一致 | 禁止 RebarSmart DLL、VisualTS、3DE、ACIS、HOOPS、Codejock。 |
| 老图石 Ribbon/功能入口作为外壳 | **部分一致** | UI 文档还偏“现代 CAD + 图石习惯”，未完全冻结为老图石 Ribbon。 |
| 钢筋模块采用 RebarSmart 思路 | 一致 | RebarSmart 作为生成逻辑、参数字段、默认值、算法证据源。 |
| 三维显示与选择用 OCCT | 一致 | STEP 导入、AIS Viewer、点线面选择、剖视显示均走 OCCT。 |
| 工程图/Detail 包兼容老图石 | 一致但验证太晚 | Detail 包定位正确，但 Reader/Writer/autoin POC 太靠后。 |
| 不调用旧 DLL/EXE | 一致 | ADR 和 AGENTS 已明确禁止。 |

### 2.2 主线偏差

当前最大的偏差在 UI 外壳：

当前 UI 文档的顶层页签是：

```text
开始 | 显示 | 钢筋 | 工程图 | 设置
```

但我们前面已经根据老图石截图确定，用户认知外壳更应该是：

```text
开始 | 显示 | 钢筋 | 查询 | 工程图
```

其中：

```text
“设置”不应作为 P0 顶层页签。
“查询”即使 P0 不实现完整功能，也应保留页签和入口。
```

理由：

```text
1. 用户会以老图石 Ribbon 作为迁移参照。
2. 查询、定位、统计、显隐、检查在老图石中是核心入口。
3. 如果删掉“查询”页签，后续功能会散落到钢筋树右键、底部表格或工程图页签，造成认知偏差。
4. “设置”更适合放在开始页签的选项按钮、右上角设置按钮或应用菜单里。
```

---

## 3. 架构边界审查

### 3.1 当前边界设计的优点

当前方案已经有比较清楚的分层：

```text
ui/
  只负责参数显示、命令入口、选择状态、预览/应用/取消。

presentation/occ/
  负责 OCCT Viewer、AIS 显示、高亮、选择、剖视显示。

step/
  负责 STEP 导入、ShapeStore、TopologyBinding。

geometry/
  提供 IGeometryEngine 和 OcctGeometryEngine。

rebarsmart/
  复刻 RebarSmart 参数、INI、分布、分区、弯钩、导引线、各 Generator。

domain/rebar/
  保存新系统钢筋业务对象，不泄漏 TopoDS / AIS / BRep 类型。

drawing/
  保存 DrawingModel、ScheduleModel 和工程图中间表达。

export/detail/
  把 DrawingModel / RebarModel / ScheduleModel 映射为 Detail 包 XML。
```

这条边界是正确的，必须继续保持。

### 3.2 当前边界仍不够硬的地方

#### 3.2.1 缺少明确的 Application Service 调用链

应明确写死：

```text
UI Command Handler
  -> Application Service / CommandService
  -> Generator
  -> RebarModel Transaction
  -> Preview / Commit / Rollback
```

禁止：

```text
UI 直接 new Generator 并改 RebarModel。
UI 直接操作 TopoDS / AIS 当业务数据。
Generator 直接读写 QWidget / AIS 对象。
```

建议新增：

```text
docs/architecture/09_ApplicationService_CommandService_边界.md
```

#### 3.2.2 IGeometryEngine P0 接口过窄

当前 TODO-011 的 P0 接口主要包含：

```text
curveLength
pointAtLength
tangentAtLength
makePolylineCurve
```

这足以支撑“中心线级别的简化生成器”，但不足以支撑完整 RebarSmart FixDistance/FixNumber，因为真实 RebarSmart 逻辑会涉及：

```text
projectCurveToFace
offsetWire / offsetCurve
intersectCurveWithFace / intersectWires
assembleWire
sweepCircle
hook / anchor
passHole
```

整改：

```text
1. TODO-013 不应叫完整 FixDistanceGenerator P0。
2. 改名为 FixDistanceCenterlineGenerator P0。
3. 明确 P0 只生成中心线，不做投影、偏移、交面、过孔、完整锚固。
4. 新增 GeometryEngine P1 接口扩展设计。
```

#### 3.2.3 Detail 包层尚未验证协议边界

Detail 包是老 CAD 插件的输入协议，不是内部 DrawingModel。这个概念正确。

但当前 Detail 包 POC 太晚。必须先做：

```text
旧包读取 -> 保守 round-trip -> CAD 插件导入验证 -> 极简包导入验证
```

在这之前，不建议大量实现 DrawingModel 到 DetailExporter 的映射。

---

## 4. 最严重的 10 个问题

下面按“风险等级、证据/依据、影响、建议修改方式”列出 10 个必须整改的问题。

---

### 问题 1：UI 外壳没有真正冻结为老图石 Ribbon

**风险等级：高**

**证据/依据：**

当前 UI 文档顶层页签为：

```text
开始 | 显示 | 钢筋 | 工程图 | 设置
```

但老图石截图和前期讨论确认，用户熟悉的主外壳是：

```text
开始 | 显示 | 钢筋 | 查询 | 工程图
```

**影响：**

```text
1. 用户感知不像新版图石。
2. 查询、定位、统计、显隐入口缺位。
3. 后续功能会被分散到钢筋树右键或其他页签。
4. Codex 可能继续按“现代 CAD Dock 工具”路线扩展 UI。
```

**建议修改方式：**

新增文档：

```text
docs/ui/13_老图石Ribbon外壳对齐规格.md
```

并修改 UI 总决策：

```text
P0 顶层页签固定为：
开始 | 显示 | 钢筋 | 查询 | 工程图

设置不作为 P0 顶层页签，放入开始/选项/右上角设置入口。
查询页签 P0 可以只有少量入口或置灰，但必须保留。
```

---

### 问题 2：P0 钢筋创建范围过大，8 个命令不适合第一闭环

**风险等级：高**

**证据/依据：**

当前 P0 钢筋页签主按钮包含：

```text
定距钢筋
定数钢筋
角筋
拉结筋
插筋
柱锥环盘面筋
箍筋
自定义钢筋
```

这些命令背后的几何输入、参数、边界和算法差异非常大。

**影响：**

```text
1. P0 容易变成“按钮都有，但每个都不完整”。
2. 无法形成可验收最小生产闭环。
3. 容易把 UI 面板、参数字段、generator、GeometryEngine 同时拖复杂。
4. 难以建立第一批 golden case。
```

**建议修改方式：**

把 P0 拆成：

```text
P0A：一个完整闭环
- STEP 导入
- Viewer 显示
- 选边/选面
- FixDistance 或 FixNumber 二选一
- RebarModel
- 保存/重开
- 下料表基础输出
- 极简 Detail 包导出
- CAD 插件导入验证

P0B：第二个生成器
- FixNumber 或 FixDistance 另一个
```

其他 6 个按钮：

```text
保留老图石 Ribbon 入口，但标记为 P1/P2 placeholder。
```

---

### 问题 3：Detail 包验证放得太晚

**风险等级：严重 / Critical**

**证据/依据：**

当前 todo 里 Detail 相关任务排在：

```text
TODO-019 Detail 包证据索引
TODO-020 DetailPackageReader
TODO-021 DetailPackageWriter round-trip
TODO-022 极简 Detail 包生成
```

这发生在 OCCT Geometry、FixDistance/FixNumber、UI、STEP Viewer 之后。

**影响：**

如果 CAD 插件不能接受新生成包，或者字段容忍度很低，会导致：

```text
1. DrawingModel 和 DetailExporter 映射返工。
2. RebarModel 的 ID 和字段设计返工。
3. 工程图兼容路线晚期暴雷。
4. 领导看到配筋可跑，但无法导入图纸。
```

**建议修改方式：**

把 Detail POC 前置，插入到 TODO-012 之后或与 TODO-012 并行：

```text
TODO-012B：DetailPackageReader P0
TODO-012C：DetailPackageWriter round-trip
TODO-012D：极简 Detail 包生成
TODO-012E：AutoCAD 插件 autoin 人工验证记录
```

验收标准：

```text
1. 能读取旧 Detail 包。
2. 能写回一个 round-trip 包。
3. round-trip 包能被 CAD 插件导入。
4. 能生成仅含一条 section-line 的极简 Detail01.stl XML。
5. 极简包能被 CAD 插件导入并画出线。
```

---

### 问题 4：STEP TopologyBinding 风险没有前置

**风险等级：严重 / Critical**

**证据/依据：**

系统架构要求：

```text
钢筋参数引用的是稳定绑定，不是 AIS 临时对象。
同一 STEP 多次导入后，关键 Face / Edge 指纹稳定。
```

但当前 todo 顺序是：

```text
TODO-012 OcctGeometryEngine P0
TODO-013 FixDistanceGenerator P0
TODO-014 FixNumberGenerator P0
TODO-015 Qt6 OCCT 主窗口
TODO-016 STEP 导入与显示
TODO-017 Viewer 选择系统
```

这意味着 generator 先于真实 STEP 导入、选择、绑定验证。

**影响：**

```text
1. Generator 可能先依赖 mock GeometryRef。
2. 后续真实 STEP face/edge 接入时绑定模型返工。
3. 保存 .tsrebar 后无法稳定恢复选面/选边。
4. RebarModel 和 TopologyBinding 之间的关系晚期才暴露问题。
```

**建议修改方式：**

在 TODO-013 前插入：

```text
TODO-012A：STEP Import + TopologyBinding Spike
```

验收：

```text
1. 导入一个真实 STEP。
2. 枚举 face/edge/vertex。
3. 生成 fingerprint。
4. 同一 STEP 连续导入 5 次。
5. 关键 face/edge fingerprint 稳定。
6. 保存 selectedFaceId / selectedEdgeId 到 JSON。
7. 重启后能重新定位到对应 face/edge。
```

---

### 问题 5：IGeometryEngine P0 接口和 FixDistance 目标不匹配

**风险等级：高**

**证据/依据：**

当前 IGeometryEngine P0 接口偏曲线采样：

```text
curveLength
pointAtLength
tangentAtLength
makePolylineCurve
```

但完整 RebarSmart FixDistance 流程需要：

```text
导引线构建
投影
偏移
导引线与面求交
平行生成
锚固
实体化
```

**影响：**

开发者可能出现两种错误：

```text
1. 在 generator 里直接 include OCCT，破坏解耦。
2. 把 P0 做成玩具实现，却命名为完整 FixDistance。
```

**建议修改方式：**

改名和限界：

```text
TODO-013：FixDistanceCenterlineGenerator P0
TODO-014：FixNumberCenterlineGenerator P0
```

P0 明确不做：

```text
projectCurveToFace
offsetCurve
intersectCurveWithFace
passHole
full anchor
solid rebar sweep
```

新增后续接口设计：

```text
docs/geometry/01_IGeometryEngine_P1扩展接口.md
```

候选接口：

```cpp
projectCurveToFace(...)
offsetCurveOnPlane(...)
intersectCurveWithFace(...)
parallelCurve3d(...)
sweepCircle(...)
sectionShape(...)
hlrProject(...)
```

---

### 问题 6：RebarSmart 参数单位、枚举、默认值没有闭合

**风险等级：高**

**证据/依据：**

RebarSmart 字段里存在大量单位和枚举：

```text
rebarDiameter：内部 m，INI/UI 可能 mm
rebarSpace：内部 m，UI 可能 mm
hook.angle：内部 rad，UI 显示度
modeProject：0~6 投影模式
modePriority：0~6 分布模式
modeSegment：0~5 分区模式
```

当前文档中仍有多个 GAP：

```text
ratioSpace
首点/末点
截断行为
闭合曲线
辅助布筋区间 Ratio
单位转换
```

**影响：**

```text
1. UI 面板字段虽然像 RebarSmart，但计算结果不一致。
2. 可能出现 mm/m 1000 倍错误。
3. 弯钩角度 rad/degree 混用。
4. 间距余量分配模式错误。
5. golden case 很难通过。
```

**建议修改方式：**

新增并强制所有 UI/Generator 使用：

```text
docs/rebarsmart/02_单位枚举默认值规则.md
```

字段模板：

```text
字段名：basic.rebarDiameter
UI 名称：直径
UI 单位：mm
INI 单位：mm
内部单位：m
Detail 单位：mm
默认值来源：GenerateRebarFixDistance.ini / RebarDiameter.ini
枚举来源：如有
置信等级：High / Medium / Low
GAP：是/否
测试用例：UNIT-xxx
```

规则：

```text
UI 面板不得直接解释 INI；
Generator 不得直接解释 UI 字符串；
必须经过 UnitMapper / ParamNormalizer。
```

---

### 问题 7：老图石命令到 RebarSmart Generator 的映射表缺失

**风险等级：中高**

**证据/依据：**

老图石截图里的钢筋页签包含：

```text
面配筋
线配筋
扇形筋
剖面配筋
面周边
同心圆
角度配筋
自配筋
箍筋
孔口
圆中心
插筋
螺旋筋
组合并
组拆开
段连接
段断开
钢筋拷贝
钢筋移动
增加钢筋段
```

当前 TS_RS 文档更多列的是 RebarSmart 命令：

```text
定距钢筋
定数钢筋
角筋
拉结筋
插筋
柱锥环盘面筋
箍筋
自定义钢筋
```

**影响：**

```text
1. UI 外壳说参考老图石，但按钮语义来自 RebarSmart，缺少桥。
2. 开发者不知道老图石“线配筋”到底映射到 FixDistance 还是 FixNumber。
3. 用户验收时会按老图石叫法提问题，代码里却是 RebarSmart 命名。
```

**建议修改方式：**

新增：

```text
docs/ui/14_老图石命令到RebarSmart生成器映射表.md
```

表格字段：

```text
老图石页签
老图石分组
老图石按钮名
TS_RS CommandId
RebarSmart Generator
输入对象
输出对象
P0/P1/P2
是否 UI 占位
是否需要人工确认
证据来源
```

示例：

| 老图石按钮 | TS_RS CommandId | RebarSmart Generator | 分期 | 备注 |
|---|---|---|---|---|
| 线配筋 | CreateLineRebar | FixDistance / FixNumber | P0/P1 | P0 先走 FixDistance 中心线闭环 |
| 面配筋 | CreateFaceRebar | FixDistance / FixNumber + GuideSurface | P1 | 需要面绑定稳定后实现 |
| 箍筋 | CreateStirrup | StirrupGenerator | P1 | 闭合 wire + 弯钩 |
| 插筋 | CreateDowel | DowelGenerator | P1 | 点式、面法向 |
| 螺旋筋 | CreateHelix | HelixGenerator | P2 | 半径、螺距、圈数 |
| 钢筋移动 | MoveRebarGroup | RebarEditMoveService | P1 | 编辑服务，不是 Generator |

---

### 问题 8：Detail 包缺少保守 round-trip 策略

**风险等级：高**

**证据/依据：**

Detail 包不是标准格式：

```text
Detail.xml + DetailNN.stl
DetailNN.stl 实际是 XML 文本
根节点 DrawingRoot
包含 StbTables、MaterialTable、HViewPorts、PartDetailDrawing、StbGroups、StbGroup、Std、StbGeo 等
```

但当前方案还没有详细的：

```text
observed schema
必填字段
未知字段保留策略
round-trip policy
CAD 插件容忍度验证记录
```

**影响：**

```text
1. Reader/Writer 可能丢字段。
2. round-trip 后 CAD 插件导入失败。
3. 无法判断失败原因是 XML 结构、属性、单位还是坐标。
4. 后续 DetailExporter 容易直接写死错误字段。
```

**建议修改方式：**

新增：

```text
docs/detail/00_Detail包证据索引.md
docs/detail/01_DetailXML_observed_schema.md
docs/detail/02_Detail_roundtrip_policy.md
```

Reader 数据结构必须包含：

```text
sourceFileName
sheetIndex
rawAttributes
unknownChildren
parseDiagnostics
```

Writer 第一阶段必须采用：

```text
lossless-ish round-trip first
```

禁止第一版直接“按理解重写 XML”。

---

### 问题 9：Qt6 RibbonLite 没有工程化规格

**风险等级：中高**

**证据/依据：**

当前 UI 文档强调顶部页签，但没有明确 RibbonLite 的最小实现范围。开发者可能走向：

```text
1. 普通 QToolBar，最后不像老图石。
2. 做复杂 Ribbon 框架，拖慢 P0。
```

**影响：**

```text
1. UI 工作量失控。
2. 外壳对齐目标无法验收。
3. Codex 可能不断改 UI 结构。
```

**建议修改方式：**

新增：

```text
docs/ui/15_RibbonLite_P0实现规格.md
```

P0 限制：

```text
使用 Qt Widgets 自研最小 RibbonLite。
只支持固定页签、固定分组、固定按钮。
不做拖拽定制。
不做运行时插件化。
不做主题系统。
不做动画。
不做 QML。
```

建议结构：

```text
RibbonBar
RibbonPage
RibbonGroup
RibbonButton
CommandRegistry
```

P0 验收：

```text
能显示：开始 / 显示 / 钢筋 / 查询 / 工程图。
每页能按老图石截图展示分组和按钮。
未实现按钮可以灰显。
点击按钮只切换命令状态或显示 NotImplemented 诊断。
```

---

### 问题 10：当前 todo 顺序不够 fail-fast

**风险等级：高**

**证据/依据：**

当前 next 是：

```text
TODO-012 OcctGeometryEngine P0
TODO-013 FixDistanceGenerator P0
TODO-014 FixNumberGenerator P0
TODO-015 Qt6 OCCT 主窗口
TODO-016 STEP 导入与显示
TODO-017 Viewer 选择系统
TODO-018 FixDistancePanel UI
TODO-019 Detail 包证据索引
```

风险最大的三个点：

```text
STEP 选择绑定是否稳定
Detail 包是否能被 CAD 插件导入
RebarSmart 单位/枚举是否一致
```

都没有足够前置。

**影响：**

```text
纯算法和 UI 都能跑，但真实闭环晚期失败。
```

**建议修改方式：**

调整近期 todo 为三线并行：

```text
A 线：算法线
B 线：STEP / TopologyBinding 线
C 线：Detail 包协议线
```

推荐新顺序见本文第 6 节。

---

## 5. 修订后的 P0 / P1 / P2 分期建议

### 5.1 P0：最小真实闭环

P0 只做一条能够对领导、业务和开发都证明可行的链路：

```text
STEP 导入
  -> OCCT Viewer 显示
  -> 选择边/面
  -> TopologyBinding 保存/恢复
  -> FixDistance 或 FixNumber 生成中心线钢筋
  -> RebarModel 保存
  -> 下料表基础输出
  -> 极简 Detail 包导出
  -> 老图石 CAD 插件 autoin 导入验证
```

P0 必须验收的东西：

```text
1. 能打开真实 STEP。
2. 同一 STEP 多次导入后选中 edge/face 可稳定恢复。
3. 能生成一组钢筋中心线。
4. 能显示钢筋预览。
5. 能保存 .tsrebar 并重开恢复。
6. 能输出基础 schedule。
7. 能生成 Detail01.stl XML。
8. CAD 插件能导入极简 Detail 包。
```

P0 不承诺：

```text
完整 8 个钢筋命令。
完整弯钩/过孔/投影/偏移。
完整工程图标注。
复杂 CAD 插件兼容。
完整图石 UI 行为。
```

### 5.2 P1：常用生产能力

```text
第二个生成器：FixNumber 或 FixDistance
基础面配筋
箍筋
插筋
拉结筋
钢筋移动 / 复制 / 删除
基础编号
Detail 包 StbGroup / StbTable / MaterialTable 映射
基础剖切/投影视图
Excel/CSV 下料表
```

### 5.3 P2：复杂复刻和外观完善

```text
角筋
柱锥环盘面筋
螺旋筋
扫掠筋
自定义筋完整行为
复杂标注布局
隐藏线/剖面线质量
图石 UI 操作细节
老 CAD 插件高兼容
工程图美化
历史格式兼容
```

---

## 6. 建议调整后的 todo 顺序

当前 TODO-012 可以继续，因为 OcctGeometryEngine P0 是合理下一步。

但 TODO-012 后应插入风险前置项。

### 6.1 建议新近期任务队列

```csv
id,priority,phase,status,task,goal_setpoint,acceptance,boundary,dependencies
TODO-012,P0,M3,next,OcctGeometryEngine P0,实现曲线长度/按弧长取点/切线/组装线,CTest 通过且记录 OCCT 容差,只改 geometry/occt,TODO-011
TODO-012A,P0,M3,pending,STEP Import + TopologyBinding Spike,验证真实 STEP 的 face/edge 稳定绑定,同一 STEP 导入 5 次关键 face/edge fingerprint 稳定且可保存/恢复,不接钢筋生成,TODO-012
TODO-012B,P0,M3,pending,DetailPackageReader P0,读取旧 Detail 包 XML 并保留 unknown 属性/节点,能解析 DrawingRoot/StbTables/HViewPorts/StbGroups,不接 OCCT/不写包,TODO-012
TODO-012C,P0,M3,pending,DetailPackageWriter round-trip,读取旧包后写回新包,round-trip 包结构不丢关键节点且 CAD 插件人工导入记录通过,保守写回不重构 XML,TODO-012B
TODO-012D,P0,M3,pending,极简 Detail 包 autoin 验证,生成仅含一条 section-line 的 Detail01.stl XML,CAD 插件 autoin 能导入并显示线,人工验证记录必须提交,TODO-012C
TODO-012E,P0,M3,pending,老图石 Ribbon 外壳对齐规格,冻结开始/显示/钢筋/查询/工程图五页签和分组按钮,新增 docs/ui/13/14/15,只写文档,无
TODO-012F,P0,M3,pending,RebarSmart 单位枚举默认值规则,建立字段单位/枚举/默认值转换表,UI/Generator 必须引用该表,只写文档和测试计划,TODO-002
TODO-013,P0,M3,pending,FixDistanceCenterlineGenerator P0,用 IGeometryEngine 生成简单定距中心线钢筋,单元/集成测试通过,不做完整投影/偏移/过孔/Detail,TODO-012A TODO-012F
TODO-014,P0,M3,pending,FixNumberCenterlineGenerator P0,用 IGeometryEngine 生成简单定数中心线钢筋,单元/集成测试通过,不做 UI/Detail,TODO-013
TODO-015,P0,M4,pending,Qt6 老图石 RibbonLite + OCCT 主窗口骨架,实现五页签外壳/Viewer/左树/右Dock/底部Dock,可启动并显示空 Viewer,不做复杂 UI 框架,TODO-012E
TODO-016,P0,M4,pending,STEP 导入显示选择闭环,打开 STEP/STP 并水泥灰显示，支持 edge/face 选择,可保存选择绑定并恢复,TODO-012A TODO-015
TODO-017,P0,M4,pending,FixDistance UI 最小闭环,点击老图石线配筋/定距入口后显示右 Dock 并预览中心线钢筋,UI 不写算法,TODO-013 TODO-016
TODO-018,P0,M5,pending,RebarModel -> 极简 DetailExporter,把中心线钢筋和一条 section-line 输出 Detail 包,CAD 插件导入验证,不做完整工程图标注,TODO-012D TODO-017
```

### 6.2 关键插入任务说明

#### TODO-012A：STEP / TopologyBinding Spike

目的：先验证 `TopologyBinding` 可行，不然所有选面配筋都不可靠。

输出：

```text
app/src/step/TopologyFingerprint.*
app/tests/topology_binding_spike_tests.cpp
docs/step/00_STEP_TopologyBinding_spike_report.md
```

验收：

```text
真实 STEP 样本至少 1 个。
同一文件导入 5 次。
关键 face/edge id 稳定。
保存 JSON 后重开能恢复。
```

#### TODO-012B/C/D：Detail 包三连

目的：先证明 CAD 插件兼容链路能跑。

输出：

```text
app/src/drawing/detail/DetailPackageReader.*
app/src/drawing/detail/DetailPackageWriter.*
app/tests/detail_package_reader_tests.cpp
docs/detail/00_Detail包证据索引.md
docs/detail/02_Detail_roundtrip_policy.md
docs/detail/03_autoin_人工验证记录.md
```

验收：

```text
能读取旧包。
能 round-trip。
能生成极简包。
CAD 插件能导入。
```

---

## 7. 必须补充的文档清单

在继续大规模开发前，建议至少补下面 10 份文档。

| 优先级 | 文件 | 目的 |
|---|---|---|
| P0 | `docs/ui/13_老图石Ribbon外壳对齐规格.md` | 固定五页签、分组、按钮外壳。 |
| P0 | `docs/ui/14_老图石命令到RebarSmart生成器映射表.md` | 桥接老图石按钮名和 RebarSmart generator。 |
| P0 | `docs/ui/15_RibbonLite_P0实现规格.md` | 防止 UI 黑洞。 |
| P0 | `docs/detail/00_Detail包证据索引.md` | 汇总 Detail 包样本、字段、插件命令。 |
| P0 | `docs/detail/01_DetailXML_observed_schema.md` | 记录观察到的 XML schema。 |
| P0 | `docs/detail/02_Detail_roundtrip_policy.md` | 定义 unknown 字段保留策略。 |
| P0 | `docs/step/00_STEP_TopologyBinding_spike_plan.md` | 定义 face/edge 稳定性验证。 |
| P0 | `docs/rebarsmart/02_单位枚举默认值规则.md` | 固化单位、枚举、默认值。 |
| P0 | `docs/validation/00_golden_case_strategy.md` | 规定 golden case 格式和比较容差。 |
| P0 | `docs/legal/00_reverse_engineering_and_resource_reuse_boundary.md` | 明确禁止复用旧 DLL/图标/资源/商标。 |

---

## 8. 建议给 Codex 的开发约束

在 `AGENTS.md` 中补充或强化：

```text
## TS_RS 当前强制路线

1. UI 外壳采用老图石 Ribbon：开始 / 显示 / 钢筋 / 查询 / 工程图。
2. RebarSmart 只是钢筋生成证据源，不调用 DLL。
3. VisualTS / 老图石只是 Detail 包和出图规则证据源，不调用 EXE/DLL。
4. OCCT 是唯一几何与显示底座。
5. Detail 包只是 exporter，不是内部主模型。
6. UI handler 不得写算法，只能调用 service / generator。
7. Generator 不得依赖 QWidget / AIS / TopoDS，除非位于 geometry/occt 内部。
8. RebarModel 不得泄漏 AIS / TopoDS / BRep 类型。
9. 新功能必须带测试或人工验证记录。
10. 不确定的旧行为必须标 GAP，不允许编造。
```

每个 Codex goal 必须包含：

```text
目标
输入证据
允许修改文件
禁止修改文件
验收标准
测试命令
GAP 记录
```

---

## 9. 对当前方案的放行意见

### 9.1 可以继续的部分

```text
1. STEP-only + Qt6 + OCCT 独立运行时路线。
2. RebarSmart 作为钢筋生成主证据源。
3. 老图石 Detail 包作为工程图兼容出口。
4. IGeometryEngine / RebarModel / DrawingModel / DetailPackageExporter 分层。
5. 纯算法层 SpaceList / Distributor / ZoneCalculator 的小步单测策略。
6. TODO-012 OcctGeometryEngine P0。
```

### 9.2 不建议直接继续的部分

```text
1. 不建议 TODO-012 后直接做 TODO-013 完整 FixDistanceGenerator。
2. 不建议 P0 同时推进 8 个钢筋命令。
3. 不建议等到 M5 才做 Detail 包 Reader/Writer。
4. 不建议 UI 顶层页签继续使用“开始/显示/钢筋/工程图/设置”。
5. 不建议先做复杂 Dock 面板，再验证单位/枚举/默认值。
```

### 9.3 结论

```text
评审结论：有条件通过。
```

条件是：

```text
1. UI 外壳改成老图石五页签：开始 / 显示 / 钢筋 / 查询 / 工程图。
2. P0 收敛为一条最小真实闭环。
3. Detail 包 round-trip 和极简 autoin 验证前置。
4. STEP TopologyBinding Spike 前置。
5. FixDistance/FixNumber P0 改名为 CenterlineGenerator，避免过度承诺。
6. 单位/枚举/默认值文档必须补齐。
7. 老图石命令到 RebarSmart Generator 映射表必须补齐。
```

---

## 10. 建议的一句话执行口径

给开发团队的口径：

```text
TS_RS 当前不是继续堆功能，而是先证明最小闭环：
STEP 导入 -> 老图石 Ribbon 入口 -> OCCT 选边/面 -> RebarSmart-style 中心线生成 -> RebarModel 保存 -> Detail 极简包导出 -> CAD 插件导入。

P0 只做一条链路跑通。
其他老图石按钮先做外壳和映射，不进入第一闭环。
```

给 Codex 的口径：

```text
不要泛化开发，不要重构全仓库。
每个 goal 只解决一个风险点。
先做协议和绑定验证，再做完整功能。
不确定旧行为必须标 GAP。
```

---

## 11. 附录：开发人员下一步任务模板

### 11.1 任务：修订 UI 外壳文档

```text
目标：
新增 docs/ui/13_老图石Ribbon外壳对齐规格.md。

必须写清楚：
- 顶层页签：开始 / 显示 / 钢筋 / 查询 / 工程图。
- 每个页签的分组。
- 每个按钮的 P0/P1/P2 状态。
- 未实现按钮的灰显策略。
- 设置入口放在哪里。

禁止：
- 实现 UI 代码。
- 修改 CMake。
- 删除现有 UI 文档。
```

### 11.2 任务：DetailPackageReader P0

```text
目标：
读取老图石 Detail 包 XML。

输入：
- Detail.xml
- Detail01.stl / Detail02.stl / ...

输出：
- DetailPackage
- DetailSheet
- DetailGeneralInfo
- DetailPrimitive
- DetailRebarGroup
- DetailStbTable
- DetailMaterialTable

要求：
- DetailNN.stl 按 XML 读取，不按 STL 网格读取。
- 保留 unknown attributes / unknown children。
- 不接 OCCT。
- 不接 UI。
- 不生成新包。
```

### 11.3 任务：STEP TopologyBinding Spike

```text
目标：
验证真实 STEP 的 face/edge 稳定绑定。

要求：
- 导入真实 STEP。
- 枚举 TopoDS_Face / TopoDS_Edge。
- 为每个拓扑生成 fingerprint。
- 同文件导入 5 次。
- 对指定 face/edge 的 fingerprint 做稳定性比较。
- 写出 spike report。
```

### 11.4 任务：FixDistanceCenterlineGenerator P0

```text
目标：
只生成中心线，不做完整 RebarSmart 复刻。

输入：
- guide curve ref
- spacing
- head/tail offset
- distribution mode

输出：
- vector<RebarSegment centerlines>

禁止：
- 投影到面
- 过孔
- 完整弯钩
- 实体 sweep
- Detail 输出
- UI 代码
```

---

## 12. 最终建议

建议项目继续推进，但先把路线从“功能实现优先”改成“风险验证优先”。

最先要打掉三个最大不确定性：

```text
1. Detail 包是否能由新代码生成并被老 CAD 插件导入。
2. STEP face/edge 是否能稳定绑定、保存、恢复。
3. RebarSmart 参数单位/枚举/default 是否能被统一解释。
```

这三个不验证，后面写再多钢筋命令都可能返工。

通过这三个验证后，再扩展 FixDistance/FixNumber、UI 面板、工程图和更多筋型，整体成功率会高很多。
