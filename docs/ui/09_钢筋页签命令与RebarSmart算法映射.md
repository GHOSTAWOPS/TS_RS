# 钢筋页签命令与 RebarSmart 算法映射

本文档记录 TS_RS 钢筋页签的完整命令清单，以及每个命令与 RebarSmart 资源、实现类、后续逆向证据的映射关系。

2026-06-16 TODO-016 / P0A 收口说明：

```text
本文档保留“完整命令映射”作为证据索引，不代表当前 P0A 验收范围。
当前开发主线以 docs/roadmap/03_P0A最小闭环与前置验证门禁.md 为准，
P0A 只验收一个最小闭环，不按 8 个钢筋命令全部可用来判断。
```

核心结论：

```text
钢筋页签完整列出 RebarSmart 钢筋生成命令。
未实现命令不从 UI 中删除，只标记实现状态。
RebarSmart 钢筋生成算法都要复刻，阶段划分只是开发顺序。
```

当前状态修正：

```text
“完整列出”是 UI 证据索引口径，不是 P0A 开发验收口径。
P0A 是否通过，只看最小闭环是否跑通。
```

## 证据来源

资源文件：

```text
RebarSmart_DrwToolKit2022X 20260604/win_b64/resources/msgcatalog/Simplified_Chinese/RebarSmart3deWorkbenchHeader.CATNls
RebarSmart_DrwToolKit2022X 20260604/win_b64/resources/msgcatalog/RebarSmart3deWorkbenchHeader.CATRsc
RebarSmart_DrwToolKit2022X 20260604/win_b64/control/RebarSmart3deFW
```

已确认中文命令标题：

```text
创建定距钢筋
创建定数钢筋
创建等参扫掠钢筋
创建柱锥环盘钢筋
创建角筋
创建多段线筋
创建拉结筋
创建箍筋
创建螺旋筋
创建插筋
创建自定义筋
创建蜗壳钢筋
创建平移复制钢筋
创建对称复制钢筋
创建旋转复制钢筋
```

## RebarSmart 原始功能区结构

从 `RebarSmart3deWorkbenchHeader.CATNls / CATRsc` 看，RebarSmart 原始工作台不是只按 4 个简单组组织。

它至少包含这些 Section / Toolbar：

```text
RebarCreationSection
  钢筋创建

RebarDuplicateingSection
  钢筋复制

RebarEditingSection
  钢筋编辑 / 钢筋修改

RebarColoringSection
  钢筋着色

RebarNumberingSection
  钢筋编号

RebarShowHideSection
  钢筋显隐

RebarReviewSection
  钢筋校审 / 视图 / 信息 / 查找

RebarClippingSection
  钢筋切图

RebarExportionSection
  钢筋导出

ExtractTools
  抽线

UtilitySection
  实用工具
```

结论：

```text
TS_RS 钢筋创建页不能只按少数泛化分组组织。
当前 P0 入口以 RebarSmart 使用说明“三维布筋”8 个命令为准。
```

## TS_RS 钢筋创建标签页边界

RebarSmart 原始工作台功能很多，但 TS_RS 的 **钢筋创建标签页** 不等于完整工作台。

钢筋创建标签页只回答一个问题：

```text
用户用什么方式创建钢筋？
```

因此它不放：

```text
钢筋编辑
钢筋编号
钢筋显隐
钢筋着色
钢筋查找
钢筋校审
钢筋切图
钢筋导出
```

这些能力后续按独立入口处理：

```text
钢筋编辑：左侧钢筋树 / 钢筋组右键 / 属性面板
钢筋编号：钢筋组右键或后续管理入口
钢筋显隐 / 着色：显示页签或钢筋树右键
钢筋查找 / 信息：左侧树、底部查询面板或右键入口
钢筋校审 / 切图 / 导出：不进入当前 MVP 主线
```

## 钢筋创建标签页 P0 布局口径

用户根据 RebarSmart 使用说明“三维布筋”目录确认：

```text
先按使用说明里的 8 个三维布筋功能做钢筋创建页主按钮。
后面不够再补充。
```

P0 冻结布局如下：

```text
钢筋创建
  定距钢筋
  定数钢筋
  角筋
  拉结筋
  插筋
  柱锥环盘面筋
  箍筋
  自定义钢筋

参考几何
  定义平面
  参考线定义
  抽线
```

说明：

```text
1. 第一批外壳按钮先放使用说明明确列出的 8 个三维布筋命令。
2. 参考几何放在钢筋创建页，因为定义平面、参考线、抽线服务于配筋创建。
3. 扫掠、多段线、螺旋、复制生成、蜗壳等先不进入 P0A 验收。
4. 扩展命令不删除，后续作为 P1/P2 或“更多创建”补充。
5. 钢筋创建标签页只承载创建类命令，不放编辑、编号、显隐、校审、导出、切图。
```

## 命令映射表

| TS_RS 命令 | RebarSmart Header | 推定实现类 / 命令 | 状态 | 备注 |
|---|---|---|---|---|
| 定距钢筋 | GenerateRebarFixDistanceHdr | GenerateRebarFixDistanceImp | 第一批外壳按钮 / P0A 候选 | 已有 `08_定距钢筋参数面板与IDA证据.md` |
| 定数钢筋 | GenerateRebarFixNumberHdr | GenerateRebarFixNumberImp | 第一批外壳按钮 / P0A 候选 | 使用说明“三维布筋”列出 |
| 角筋 | GenerateRebarCornerHdr | GenerateRebarCornerImp | 第一批外壳按钮 / P1/P2 | 资源标题为“角筋布筋 / 角筋参数” |
| 拉结筋 | GenerateRebarTieHdr | GenerateRebarTieImp | 第一批外壳按钮 / P1/P2 | 资源标题为“拉结布筋” |
| 插筋 | GenerateRebarDowelHdr | GenerateRebarDowelImp | 第一批外壳按钮 / P1/P2 | 资源标题为“预埋插筋” |
| 柱锥环盘面筋 | GenerateRebarFrustumHdr | GenerateRebarFrustumImp | 第一批外壳按钮 / P1/P2 | 使用说明写作“柱锥环盘面筋”，资源标题为“柱锥环盘钢筋” |
| 箍筋 | GenerateRebarStirrupHdr | GenerateRebarStirrupImp | 第一批外壳按钮 / P1/P2 | 使用说明“三维布筋”列出 |
| 自定义钢筋 | GenerateRebarCustomizeHdr | GenerateRebarCustomizeImp | 第一批外壳按钮 / P1/P2 | 资源标题为“自定义布筋” |
| 等参扫掠钢筋 | GenerateRebarSweepHdr | GenerateRebarSweepImp | 扩展候选 / 待复刻 | 先不进入 P0A 验收，后续按需要补充 |
| 多段线筋 | GenerateRebarPolylineHdr | GenerateRebarPolylineImp | 扩展候选 / 待复刻 | 先不进入 P0A 验收，后续按需要补充 |
| 螺旋筋 | GenerateRebarHelixHdr | GenerateRebarHelixImp | 扩展候选 / 待复刻 | 先不进入 P0A 验收，后续按需要补充 |
| 蜗壳钢筋 | GenerateRebarVoluteHdr | GenerateRebarVoluteImp 待确认 | 扩展候选 / 待确认 | RebarCreation 有入口，CATRsc 图标行被注释，本轮未在 AfrRebarObjImp 查到实现类 |
| 平移复制钢筋 | GenerateRebarCopyTranslateHdr | GenerateRebarCopyTranslateImp | 扩展候选 / 已确认为创建类复制 | 选源钢筋组 + 起点 / 终点，生成新钢筋组 |
| 对称复制钢筋 | GenerateRebarCopySymmetryHdr | GenerateRebarCopySymmetryImp | 扩展候选 / 已确认为创建类复制 | 选源钢筋组 + 对称面，生成新钢筋组 |
| 旋转复制钢筋 | GenerateRebarCopyRotationHdr | GenerateRebarCopyRotationImp | 扩展候选 / 已确认为创建类复制 | 选源钢筋组 + 旋转轴 / 旋转角，生成新钢筋组 |

## 第一批创建入口 IDA MCP 复核

本轮已用 IDA MCP 复核第一批 8 个命令入口和实现类：

```text
这是历史证据记录，不代表 TODO-016 需要把 8 个命令全部实现。
```

```text
RebarCreation.dll
  fctCreateGenerateRebarFixDistanceCmd = 0x1800C0970
  fctCreateGenerateRebarFixNumberCmd   = 0x1800E94A0
  fctCreateGenerateRebarCornerCmd      = 0x180061C20
  fctCreateGenerateRebarTieCmd         = 0x1801ADF00
  fctCreateGenerateRebarDowelCmd       = 0x1800929D0
  fctCreateGenerateRebarFrustumCmd     = 0x180108CF0
  fctCreateGenerateRebarStirrupCmd     = 0x18016D360
  fctCreateGenerateRebarCustomizeCmd   = 0x180082CE0

AfrRebarObjImp.dll
  GenerateRebarFixDistanceImp
  GenerateRebarFixNumberImp
  GenerateRebarCornerImp
  GenerateRebarTieImp
  GenerateRebarDowelImp
  GenerateRebarFrustumImp
  GenerateRebarStirrupImp
  GenerateRebarCustomizeImp
```

说明：

```text
RebarCreation.dll 主要确认命令入口和 UI/ProgressTask 链。
AfrRebarObjImp.dll 中的 GenerateRebarXXXImp 才是后续复刻算法的主要目标。
```

## GenerateRebarCopy 系列放置结论

GenerateRebarCopy 系列长期放到钢筋创建标签页的 `复制生成` / `更多创建` 入口。

但历史 UI 决策已经把第一批外壳按钮收窄为使用说明“三维布筋”的 8 个命令，因此 GenerateRebarCopy 系列 **不进入第一批外壳按钮**。

资源证据：

```text
GenerateRebarCopyTranslateDlg.CATNls
  标题：平移复制钢筋
  定位字段：起点 / 终点 / 钢筋组
  参数字段：钢筋属性 / 生成钢筋实体
  式样字段：样式表

GenerateRebarCopySymmetryDlg.CATNls
  标题：对称复制钢筋
  定位字段：旋转面 / 钢筋组

GenerateRebarCopyRotationDlg.CATNls
  标题：旋转复制钢筋
  定位字段：旋转轴 / 旋转角 / 钢筋组
```

IDA MCP 证据：

```text
RebarCreation.dll
  fctCreateGenerateRebarCopyTranslateCmd = 0x180051460
  fctCreateGenerateRebarCopySymmetryCmd = 0x180046670
  fctCreateGenerateRebarCopyRotationCmd = 0x1800392C0

AfrRebarObjImp.dll
  GenerateRebarCopyTranslateImp::IsInputOK = 0x180010C10
  GenerateRebarCopySymmetryImp::IsInputOK = 0x18000DB20
  GenerateRebarCopyRotationImp::IsInputOK = 0x180001450
  GenerateRebarCopyTranslateImp::CreateRebarGroupPipeline = 0x180011B00
  GenerateRebarCopyTranslateImp::RebarGSRootAttributePipeline_Initialize = 0x1800121C0
```

语义结论：

```text
GenerateRebarCopy 系列不是 DuplicateRebar 的简单别名。
它们拥有独立 GenerateRebarCopyTranslate/Symmetry/RotationImp 实现类。
IsInputOK 会检查源钢筋组 GSRebarSource，以及平移点 / 对称面 / 旋转轴等创建参数。
CreateRebarGroupPipeline 会调用 GenerateRebarGroup 生成新的钢筋组。
```

## DuplicateRebar 系列放置结论

DuplicateRebar 系列不放到钢筋创建标签页。

证据来自资源文件：

```text
DuplicateRebarTranslateDlg.CATNls
DuplicateRebarSymmetryDlg.CATNls
DuplicateRebarRotationDlg.CATNls
DuplicateRebarFullParametersDlg.CATNls
```

这些窗口明确围绕：

```text
所有钢筋组
指定钢筋组
钢筋编号范围
选择钢筋组
```

因此 TS_RS 放置为：

```text
钢筋组右键 / 右侧 Dock 钢筋组操作区：
  克隆复制
  平移复制
  对称复制
  旋转复制
```

单根钢筋右键不放完整复制命令。

GenerateRebarCopy 系列不与 DuplicateRebar 合并。

## 右侧 Dock 原则

每个钢筋生成命令原则上使用独立参数面板：

```text
FixDistancePanel
FixNumberPanel
SweepPanel
FrustumPanel
CornerPanel
PolylinePanel
TiePanel
StirrupPanel
HelixPanel
DowelPanel
CustomizePanel
VolutePanel
CopyTranslatePanel
CopySymmetryPanel
CopyRotationPanel
```

硬规则：

```text
每个钢筋创建 Dock 面板，都必须与 RebarSmart 对应 GenerateRebarXXXDlg 弹窗保持一致。
```

一致范围：

```text
参数分组一致
字段槽位一致
选择对象类型一致
默认值 / 单位 / 枚举语义一致
联动和校验逻辑按 RebarSmart 证据复刻
```

允许变化：

```text
弹窗改成右侧 Dock
确定改成应用
对象选择适配 OCCT Viewer
增加 Dock 顶部摘要、错误提示、折叠调试区
```

不允许：

```text
随便删字段
随便合并分组
自研一套简化参数面板替代 RebarSmart 弹窗
```

第一批创建入口点击后的 Dock 切换：

```text
定距钢筋
  -> FixDistancePanel

定数钢筋
  -> FixNumberPanel

角筋
  -> CornerPanel

拉结筋
  -> TiePanel

插筋
  -> DowelPanel

柱锥环盘面筋
  -> FrustumPanel

箍筋
  -> StirrupPanel

自定义钢筋
  -> CustomizePanel
```

P0 面板与 RebarSmart 弹窗对应关系：

```text
FixDistancePanel
  -> GenerateRebarFixDistanceDlg

FixNumberPanel
  -> GenerateRebarFixNumberDlg

CornerPanel
  -> GenerateRebarCornerDlg

TiePanel
  -> GenerateRebarTieDlg

DowelPanel
  -> GenerateRebarDowelDlg

FrustumPanel
  -> GenerateRebarFrustumDlg

StirrupPanel
  -> GenerateRebarStirrupDlg

CustomizePanel
  -> GenerateRebarCustomizeDlg
```

交互规则：

```text
1. 点击钢筋标签页命令后，右侧 Dock 立即切换到对应生成器。
2. Dock 顶部固定显示当前命令、当前对象、选择状态、状态提示四行摘要。
3. Dock 内统一提供“预览 / 应用 / 取消”。
4. 需要用户在 OCCT Viewer 中选择面、线、点、方向时，Dock 显示当前步骤和选择结果。
5. 参数面板保留 RebarSmart 字段语义，但布局采用 TS_RS 右侧 Dock。
```

Dock 顶部摘要：

```text
当前命令：定数钢筋
当前对象：新建钢筋组
选择状态：布筋面 2，引导线 1，参考线 0
状态提示：可预览
```

调试信息默认折叠：

```text
对象 ID
commandId
selectedFaceIds
selectedEdgeIds
generatorClass
evidenceStatus
gapStatus
```

应用后的连续创建规则：

```text
点击“应用”并成功生成一组钢筋后：
  -> 保留当前生成器面板
  -> 保留上一次参数值
  -> 清空已选择几何对象 / 目标对象
  -> 清除临时预览
  -> 状态回到“等待选择对象”
```

说明：

```text
这样用户连续创建同类型钢筋时，不需要重复输入直径、等级、间距、保护层等参数。
如果用户点击“取消”，则退出当前生成器，右侧 Dock 回到当前选择对象摘要或空闲状态。
```

公共字段不在 UI 层硬复制算法，而是复用公共参数区块：

```text
钢筋属性
布筋面 / 保护层 / 层间距
导引线 / 辅助导引线
端距 / 区间
间距 / 根数 / 余量
延长 / 截断
弯钩
式样表
预览 / 应用 / 取消
```

边界：

```text
UI 层只承载参数、选择状态和命令入口。
钢筋生成算法必须放到 generator / service / domain 层。
UI handler 不允许直接写几何算法。
```

## 后续逐项补证据

每个命令后续都要补齐：

```text
1. Dlg.CATNls 参数分组和中文控件名。
2. ini 模板字段和默认值。
3. RebarCreation.dll 命令入口。
4. AfrRebarObjImp.dll 或相关 DLL 的 GenerateRebarXXXImp 实现类。
5. AfrRebarCore3D.dll 公共字段 / 公共算法调用。
6. PDF 使用说明中的操作流程。
7. TS_RS 右侧 Dock 面板结构。
8. DTO 字段清单。
9. 算法缺口和 IDA 待确认项。
```

钢筋创建 Dock 面板一致性规则见：

```text
docs/ui/10_钢筋创建Dock面板RebarSmart一致性规则.md
```

## 当前下一步

钢筋创建页第一批外壳按钮已阶段冻结；P0A 验收另见 `docs/roadmap/03_P0A最小闭环与前置验证门禁.md`。
钢筋创建命令点击后统一切换右侧 Dock，已阶段冻结。
点击“应用”生成后保留当前生成器和参数，已阶段冻结。
Dock 顶部固定四行摘要，已阶段冻结。

冻结结果：

```text
钢筋创建：
  定距钢筋
  定数钢筋
  角筋
  拉结筋
  插筋
  柱锥环盘面筋
  箍筋
  自定义钢筋

参考几何：
  定义平面
  参考线定义
  抽线
```

下一轮 UI grill-me 建议讨论：

```text
1. 先展开哪个命令的参数面板细节，建议从“定距钢筋”继续。
2. 是否为定数钢筋新建专项文档，记录用户提供的 3DE 截图、CATNls 和 IDA 证据。
```
