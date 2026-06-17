# 定距钢筋参数面板与 IDA 证据

本文档记录 RebarSmart `定距布筋` 参数的证据来源，以及 TS_RS 第一版右侧 Dock 参数面板设计。

核心结论：

```text
RebarSmart 原始实现是 3DE/CATIA 弹窗 GenerateRebarFixDistanceDlg。
TS_RS 不照搬 3DE 弹窗外观，但定距钢筋的参数分组、字段语义和联动规则应尽量沿用 RebarSmart。

定距钢筋不是“输一个间距就生成”。
它至少由布筋面、主引导线、辅助引导线、区间端距、优先模式、
间距 / 根数 / 间距列表、孔洞、延长、弯钩共同决定。
```

## TODO-004 审计结论

2026-06-15 复查结论：

```text
本文档继续作为 FixDistancePanel 和 FixDistanceCenterlineGenerator 的主证据入口。
当前代码已实现 centerline P0，不代表完整 RebarSmart FixDistance 复刻。

但必须收紧证据口径：
1. INI 字段、默认值、Ratio 是确定证据。
2. RebarCreation.dll 只确认命令入口和 3DE 弹窗层。
3. AfrRebarObjImp.dll 才是定距钢筋实现主入口。
4. AfrRebarCore3D.dll 提供点位、区间、端距和偏移联动能力。
5. 本文档给出的算法流程是可开发约束，不是完整最终伪代码。
```

对后续开发的直接约束：

```text
FixDistancePanel：
  完整保留 RebarSmart 定距字段槽位，落到右侧 Dock。

FixDistanceCenterlineGenerator：
  只依赖 GenerateRebarData + FixDistanceSelectionContext + IGeometryEngine。
  不直接依赖 OCCT / AIS / TopoDS。

TODO-005 已完成：
  已把 UpdateRebarCountSpaceMargin、UpdateGuideCurveZone、
  GetGuideSurfaceOffestDistance、FormGuideCurveZoneArray 拆成纯算法测试矩阵。

TODO-006：
  下一步只初始化 app 工程骨架，不实现定距算法。
```

## 证据来源

本轮使用的证据：

```text
RebarSmart_DrwToolKit2022X 20260604/win_b64/resources/msgcatalog/GenerateRebarFixDistanceDlg.CATNls
RebarSmart_DrwToolKit2022X 20260604/win_b64/resources/msgcatalog/GenerateRebarFixDistanceDlg.CATRsc
RebarSmart_DrwToolKit2022X 20260604/win_b64/resources/msgcatalog/Simplified_Chinese/RebarSmart3deWorkbenchHeader.CATNls
RebarSmart_DrwToolKit2022X 20260604/ResourceRebarSmart/Default/GenerateRebarFixDistance.ini
RebarSmart_DrwToolKit2022X 20260604/ResourceRebarSmart/2002008/GenerateRebarFixDistance.ini
docs/rebarsmart/01_GenerateRebarData字段地图.md
docs/ui/10_钢筋创建Dock面板RebarSmart一致性规则.md
```

IDA MCP 复核状态：

```text
2026-06-15 本轮已重新打开：
  tsrs_rebarcreation     = RebarCreation.dll
  tsrs_afrorebarobjimp   = AfrRebarObjImp.dll
  tsrs_afrrebarcore3d    = AfrRebarCore3D.dll

RebarCreation.dll
  0x1800C0970 fctCreateGenerateRebarFixDistanceCmd
  0x1800C1E80 GenerateRebarFixDistanceDlg UI 初始化函数

AfrRebarObjImp.dll
  0x180035240 GenerateRebarFixDistanceImp::GenerateRebarFixDistanceImp()
  0x180035480 GenerateRebarFixDistanceImp::IniConfigure_Load()
  0x1800416C0 GenerateRebarFixDistanceImp::RebarGSRootAttributePipeline_Initialize()
  0x180041770 GenerateRebarFixDistanceImp::RebarGSRootAttributePipeline_Retrieve(...)
  0x18003AEB0 GenerateRebarFixDistanceImp::IsInputOK()
  0x180037C20 GenerateRebarFixDistanceImp::FormGuideCurveZoneArray(...)
  0x18003D1D0 GenerateRebarFixDistanceImp::CreateRebarCurveBodyPipeline()
  0x18003DA80 GenerateRebarFixDistanceImp::CreateRebarGroupPipeline(...)
  0x180042C00 GenerateRebarFixDistanceImp::RebarPatternPipeline_Retrieve(...)
  0x180044B80 GenerateRebarFixDistanceImp::RebarPatternPipeline_Update(...)
  0x180045370 GenerateRebarFixDistanceImp::RebarPatternGuideCurveAuxiliary_Retrieve(...)
  0x180045720 GenerateRebarFixDistanceImp::RebarPatternCurveZoneAuxiliary_Retrieve(...)

AfrRebarCore3D.dll
  0x1800046C0 GenerateRebarBase::RebarGSRootAttributeBasicParm_Initialize
  0x18000A2B0 GenerateRebarBase::RebarGSRootAttributeGuideSurface_Initialize
  0x180044460 GenerateRebarCore::UpdateRebarCountSpaceMargin
  0x180044C90 GenerateRebarCore::UpdateGuideCurveZone
  0x180049EC0 GenerateRebarCore::GetGuideSurfaceOffestDistance
  0x18001F280 GenerateRebarBase::RebarGSRootAttributePassHole_Initialize
  0x180021550 GenerateRebarBase::RebarGSRootAttributeExtendTangential_Initialize
  0x1800256B0 GenerateRebarBase::RebarGSRootAttributeExtendAuxiliary_Initialize
  0x18002BC60 GenerateRebarBase::RebarGSRootAttributeExtendHook_Initialize
```

证据等级：

```text
确定：
  Default/GenerateRebarFixDistance.ini 的 57 字段、TableRow0、Ratio。
  RebarCreation.dll 的定距命令入口。
  AfrRebarObjImp.dll 中 FixDistanceImp 构造函数加载 GenerateRebarFixDistance.ini。
  FixDistanceImp::IsInputOK 只检查布筋面和主引导线非空。
  AfrRebarCore3D.dll 中三类联动函数入口和主要分支。

高置信：
  RebarCreation.dll:0x1800C1E80 中的控件名清单。
  定距 Dock 分组与 RebarSmart 弹窗分组的对应关系。

GAP：
  Dlg 中文枚举名、辅助区间距离单位、孔洞模式、式样表、完整点位拓扑。
```

## RebarSmart 原始 UI 分组

`GenerateRebarFixDistanceDlg.CATNls` 直接确认原始弹窗标题：

```text
Title = 定距布筋
```

`Simplified_Chinese\RebarSmart3deWorkbenchHeader.CATNls` 用 GBK 解码后确认顶部命令标题：

```text
RebarSmart3deWorkbenchHeader.GenerateRebarFixDistanceHdr.Title = 创建定距钢筋
```

原始弹窗主要分组：

```text
定位
  布筋面
  主布筋区间
  辅助布筋区间
  轴线经孔洞处理
  轴线延长/截断
  钢筋间距参数
  应用式样

延长与弯钩
  首端辅助延长
  末端辅助延长
  首端弯钩
  末端弯钩

参数
  钢筋属性
  钢筋等级说明

式样
  样式表
```

TS_RS 第一版建议保留这些语义分组，但改造成右侧常驻 Dock 的可折叠面板。

## IDA 复核结论 2026-06-12

2026-06-15 已重新复核并保留 2026-06-12 的有效结论：

```text
RebarCreation.dll
  负责命令入口、3DE 弹窗、按钮动作和 UI 初始化。

AfrRebarObjImp.dll
  才包含 GenerateRebarFixDistanceImp 的主要业务实现。

GenerateRebarFixDistanceImp 构造函数
  加载 GenerateRebarFixDistance.ini
  调用 RebarGSRootAttributePipeline_Initialize
  调用 IniConfigure_Load

IniConfigure_Load
  读取 INI 的 RebarPatternCurObject
  再调用 RebarPatternPipeline_Update 写回当前对象字段

IsInputOK
  只检查主布筋引导线和布筋面选择是否非空
  因此参数合法性和点位生成不是在这里完成

FormGuideCurveZoneArray
  是主布筋区间 / 分区 / 点位数组生成的关键函数
  会读取导引线长度、保护层、直径、头尾区间、端部模式、孔洞/关键点等信息
```

白话结论：

```text
定距钢筋不是“输入一个间距就生成”。
它至少由 布筋面 + 主导引线 + 区间端距 + 优先模式 + 间距/根数/列表 + 延长弯钩 共同决定。
```

2026-06-15 新增复核细节：

```text
RebarCreation.dll:fctCreateGenerateRebarFixDistanceCmd
  创建命令对象名 GenerateRebarFixDistanceCmd。
  本层不是算法层。

AfrRebarObjImp.dll:GenerateRebarFixDistanceImp::GenerateRebarFixDistanceImp
  调用 GenerateRebarCore 构造。
  加载 GenerateRebarFixDistance.ini。
  调用 RebarGSRootAttributePipeline_Initialize。
  调用 IniConfigure_Load。

AfrRebarObjImp.dll:GenerateRebarFixDistanceImp::IsInputOK
  GetLstPathGuideSurface().Size() > 0
  GetLstPathGuideCurve().Size() > 0
  两者同时满足才返回 true。

AfrRebarObjImp.dll:GenerateRebarFixDistanceImp::FormGuideCurveZoneArray
  读取主区间首尾距离。
  读取是否按 segment / key point 分区。
  读取布筋面、主引导线、曲线长度、钢筋直径、保护层、引导线反向。
  调用 FormLstKeyRebarPnt、PointLengthPartition_OnVertex、
  CreateRebarCurveBodyForPartition、PointLengthPartition_OnSurfaceHole、
  PointLengthPartition_Pipeline。
  对越界点位有 0.5 * 直径 的保护性修正。
```

## UI 控件证据

`RebarCreation.dll:0x1800C1E80` 中创建了以下控件名，和 CATNls 文案对应：

```text
布筋面：
  CheckBtnGuideSurfaceOffset
  SpinnerGuideSurfaceOffsetProtectThickness
  SelectorListGuideSurface
  CheckBtnGuideSurfaceCount
  ComboBoxGuideSurfaceCount
  SpinnerGuideSurfaceSpace
  PushBtnGuideSurfaceInverse
  ComboBoxModeGuideSurfaceOffset

主布筋区间：
  EditorGuideCurveZoneLength
  EditorGuideCurveTotalLength
  PushBtnGuideCurveSelect
  SelectorListGuideCurve
  SpinnerGuideCurveZoneHead
  SpinnerGuideCurveZoneTail
  PushBtnGuideCurveInverse
  CheckBtnGuidueCurveZoneHeadSpaceListTxt
  CheckBtnGuidueCurveZoneTailSpaceListTxt
  EditorGuidueCurveZoneHeadSpaceListTxt
  EditorGuidueCurveZoneTailSpaceListTxt

辅助布筋区间：
  EditorGuideCurveAuxiliaryZoneLength
  EditorGuideCurveAuxiliaryTotalLength
  SpinnerGuideCurveAuxiliaryZoneHead
  ComboBoxCurveAuxiliaryZoneHead
  SpinnerGuideCurveAuxiliaryZoneTail
  ComboBoxCurveAuxiliaryZoneTail
  SelectorListGuideCurveAuxiliary
  PushBtnGuideCurveAuxiliaryInverse

钢筋间距参数：
  LabelRebarCount
  RadioBtnPriorityOnRebarCount
  RadioBtnPriorityOnRebarSpace
  SpinnerRebarSpace
  EditorRebarCountUserInput
  RadioBtnPriorityOnSpaceList
  EditorSpaceListTxt
  LabelSpaceList

延长与弯钩：
  FrameRebarExtendHookHead
  FrameRebarExtendHookTail
```

## 默认值证据

`GenerateRebarFixDistance.ini` 的 `RebarPatternMultiList.Title` 复核后为 57 列，不是 56 列。
`Ratio` 是界面显示值和内部米制/弧度制字段之间的换算因子：

```text
ratio = 1000.0  多用于直径，内部 m，界面 mm
ratio = 100.0   多用于间距、保护层、长度，内部 m，界面 cm
ratio = 57.29577951471995  角度 rad -> deg
ratio = -1.0    字符串、布尔或不参与数值换算
ratio = 1.0     原值使用
```

注意：

```text
辅助布筋区间首端 / 末端距离的 Ratio 在 Default INI 中是 1.0，
而主布筋区间首端 / 末端距离的 Ratio 是 100.0。

这不是笔误。TS_RS 不能把辅助区间距离直接按主区间单位处理。
后续 TODO-005 / TODO-013 必须单独确认它的真实 UI 单位和内部含义。
```

完整列序：

```text
01 名称
02 基本参数_等级
03 基本参数_直径
04 基本参数_间距
05 布筋面_保护层厚度
06 基本参数_颜色
07 基本参数_线宽
08 基本参数_根数
09 基本参数_束内根数
10 钢筋实体_钢筋实体生成的标识
11 钢筋实体_钢筋实体类型
12 布筋面_布筋面偏移
13 布筋面_布筋面保护层偏移模式
14 布筋区间首端_布筋区间端部距离
15 布筋区间末端_布筋区间端部距离
16 布筋区间首端_布筋区间端部距离的调整方式
17 布筋区间末端_布筋区间端部距离的调整方式
18 布筋区间_布筋区间余量分配模式
19 布筋区间首端_区间两端的扩展布筋标识
20 布筋区间末端_区间两端的扩展布筋标识
21 布筋区间首端_区间两端的布筋间距列表
22 布筋区间末端_区间两端的布筋间距列表
23 布筋区间_布筋点分布的优先模式
24 布筋区间_钢筋间距列表
25 布筋区间_布筋点间距的比例
26 辅助布筋区间首端_布筋区间端部距离
27 辅助布筋区间末端_布筋区间端部距离
28 辅助布筋区间首端_布筋区间端部距离的调整方式
29 辅助布筋区间末端_布筋区间端部距离的调整方式
30 钢筋轴线穿孔_钢筋轴线穿孔模式
31 钢筋轴线穿孔_钢筋轴线穿孔保留曲线段号
32 切向延长(截断)首端_长度
33 切向延长(截断)末端_长度
34 切向延长(截断)首端_长度倍径
35 切向延长(截断)末端_长度倍径
36 辅助延长首端_生成端部延长
37 辅助延长末端_生成端部延长
38 辅助延长首端_长度
39 辅助延长末端_长度
40 辅助延长首端_长度倍径
41 辅助延长末端_长度倍径
42 端部弯钩首端_生成端部弯钩
43 端部弯钩末端_生成端部弯钩
44 端部弯钩首端_圆角内切
45 端部弯钩末端_圆角内切
46 端部弯钩首端_反转弯钩方向
47 端部弯钩末端_反转弯钩方向
48 端部弯钩首端_长度
49 端部弯钩末端_长度
50 端部弯钩首端_长度倍径
51 端部弯钩末端_长度倍径
52 端部弯钩首端_半径
53 端部弯钩末端_半径
54 端部弯钩首端_半径倍径
55 端部弯钩末端_半径倍径
56 端部弯钩首端_角度
57 端部弯钩末端_角度
```

Default `TableRow0=式样2` 默认值：

```text
01 名称 = 式样2
02 基本参数_等级 = C
03 基本参数_直径 = 0.028, Ratio 1000.0
04 基本参数_间距 = 0.2, Ratio 100.0
05 布筋面_保护层厚度 = 0.05, Ratio 100.0
08 基本参数_根数 = 11
09 基本参数_束内根数 = 1
10 钢筋实体_钢筋实体生成的标识 = false
11 钢筋实体_钢筋实体类型 = 0
12 布筋面_布筋面偏移 = true
13 布筋面_布筋面保护层偏移模式 = 0
18 布筋区间_布筋区间余量分配模式 = 4
21 / 22 区间两端布筋间距列表 = 20*12,25*8
23 布筋区间_布筋点分布的优先模式 = 0
24 布筋区间_钢筋间距列表 = 20*12,25*8
25 布筋区间_布筋点间距的比例 = 0.8
26 / 27 辅助布筋区间首末端距离 = 0, Ratio 1.0
30 钢筋轴线穿孔模式 = 0
31 钢筋轴线穿孔保留曲线段号 = 0
32 / 33 切向延长首末端长度 = 0
36 / 37 辅助延长首末端生成 = false
42 / 43 端部弯钩首末端生成 = false
56 / 57 端部弯钩首末端角度 = 0, Ratio 57.29577951471995
```

字段映射要求：

```text
这些字段必须能映射到 GenerateRebarData。
选择对象不放进 GenerateRebarData，而放进 FixDistanceSelectionContext：
  guideSurfaceRefs
  guideCurveRefs
  guideCurveAuxiliaryRefs
```

`RebarGSRootAttributeBasicParm_Initialize` 历史复核结论：

```text
钢筋编号默认值：字符串 "1"
钢筋等级默认值：C
钢筋直径默认值：0.032 m
钢筋间距默认值：0.2 m
显示颜色默认值：RGB(255, 0, 0)
线宽默认值：2
根数默认值：1
束内根数默认值：1
```

`RebarGSRootAttributeGuideSurface_Initialize` 历史复核结论：

```text
保护层默认值：0.05 m
层间距默认值：0.2 m
布筋面偏移相关开关默认开启
布筋面层数 / active layer 相关字段默认存在
```

`RebarGSRootAttributeExtendHook_Initialize` 历史复核结论：

```text
首端弯钩创建：默认关闭
末端弯钩创建：默认关闭
弯钩长度默认值：3 * 钢筋直径
弯钩角度默认值：0
弯钩半径 / 倍率类字段默认值：1.2
```

`RebarGSRootAttributePassHole_Initialize` 历史复核结论：

```text
轴线经孔洞处理模式默认值：0
```

说明：

```text
上面 BasicParm / GuideSurface / ExtendHook / PassHole 初始化函数本轮只复查了入口地址，
没有重新逐行展开伪代码。
后续如果要把默认值写入 C++ 测试，优先以 INI 默认值为准，
再用 IDA 复核初始化函数是否覆盖。
```

## 根数 / 间距 / 余量联动

`GenerateRebarCore::UpdateRebarCountSpaceMargin` 是定距布筋的关键联动函数。

2026-06-15 已复核逻辑：

```text
先 UpdateGuideCurveZone
再读取 GuideCurveZoneLength
再读取引导线曲线长度
可用长度 = 曲线总长 - 头端区间长度 - 尾端区间长度
```

优先模式：

```text
mode == 1：根数优先
  读取 RebarCount
  如果根数 > 1：
    间距 = 可用长度 / (根数 - 1)
  否则：
    间距 = 2 * 可用长度
  余量 = 0

mode == 2：间距列表优先
  读取 RebarSpaceList
  调用 GetPointLength_ProiriyList 生成点位
  根数 = 点位数量
  余量 = 曲线总长 - 尾端区间长度 - 最后一个点位长度

其他 / 默认：间距优先
  读取 RebarSpace
  读取 RatioSpace
  调用 CalPointCountOnGuideCurve / CalSectionCountOnGuideCurve
  反算根数和余量
```

结论：

```text
TS_RS 定距钢筋不能只做“间距输入框”。
P0 必须至少保留：根数优先、间距优先、间距列表三种入口。
```

对 TODO-005 的测试拆分要求：

```text
SpaceListParser:
  输入 20*12,25*8 这类文本，输出分段距离序列。

PrioritySpaceDistributor:
  覆盖 mode 默认 / 0 的间距优先路径。
  覆盖 count、space、ratioSpace、head/tail zone 对根数和余量的影响。

PriorityCountDistributor:
  覆盖 mode == 1：
    count > 1: space = usableLength / (count - 1)
    count <= 1: space = 2 * usableLength

PriorityListDistributor:
  覆盖 mode == 2：
    list 生成点位。
    count = pointCount。
    margin = curveLength - tailZone - lastPointLength。
```

## 区间端部调整逻辑

`GenerateRebarCore::UpdateGuideCurveZone` 读取：

```text
钢筋直径
钢筋间距
ModeGuideCurveZoneAdjust
GuideCurveZoneLength
IndexGuideSurfaceLayerActive
```

已确认端部模式：

```text
mode 1：端距 = 0
mode 2：端距 = 0.5 * 钢筋直径
mode 3：端距 = 0.5 * 钢筋直径 + 钢筋间距
mode 4：端距 = 当前布筋面偏移距离
```

`GetGuideSurfaceOffestDistance` 确认布筋面偏移距离与以下参数相关：

```text
钢筋直径
保护层
层间距
布筋面偏移模式
当前层索引
```

2026-06-15 复核的偏移模式数值分支：

```text
offsetMode == 0:
  offset = protectThickness + 0.5 * diameter

offsetMode == 1:
  offset = protectThickness - 0.5 * diameter

offsetMode == 2:
  offset = protectThickness + 0.5 * diameter

offsetMode == 3 或 4:
  offset = CalFirstLayerProtectThicknessNet() + 0.5 * diameter

offsetMode == 5:
  offset = protectThickness

最终返回：
  offset + activeLayerIndex * guideSurfaceLayerSpacing
```

说明：

```text
上面只确认数值分支，不确认 RebarSmart UI 中每个枚举的中文名称。
UI 面板可以先显示“模式 0/1/2/3/4/5”，中文名等实际运行或 CATNls 继续确认。
```

结论：

```text
主布筋区间和辅助布筋区间里的“左侧距离 / 右侧距离”不是普通独立数字。
它们和直径、间距、保护层、层间距、偏移模式存在联动。
```

辅助区间补充：

```text
AfrRebarObjImp.dll 中存在独立的辅助引导线和辅助区间函数：
  GetListPathGuideCurveAuxiliary
  GetGuideCurveAuxiliary
  GuideCurveAuxiliary_Create / Remove
  GetPointLengthAuxiliary
  GetPointAndDirectionOnAuxiliaryCurve
  RebarGSRootAttributeCurveZoneAuxiliary_*
  RebarPatternCurveZoneAuxiliary_*

这说明辅助布筋区间不是纯 UI 装饰字段。
但它如何和主引导线点位配对、如何影响最终钢筋轴线，
仍需在 TODO-005 / TODO-013 继续深挖。
```

## TS_RS P0 右侧 Dock 面板

定距钢筋进入命令后，右侧 Dock 采用可折叠分组。

用户已确认：

```text
完整保留 RebarSmart 定距布筋的参数分组和字段槽位。
```

设计口径：

```text
不为了 MVP 删除 RebarSmart 参数分组。
算法未闭合的字段可以先禁用、只读或默认折叠，但字段槽位必须保留。
```

右侧 Dock 分组：

```text
1. 当前命令摘要
  当前命令：定距钢筋
  当前对象：新建钢筋组 / 已选钢筋组
  选择状态：布筋面 N，主引导线 M，辅助引导线 K
  状态提示：等待选择 / 可预览 / 参数错误

2. 选择对象
  选择布筋面
  选择布筋引导线
  选择辅助布筋引导线
  清除选择
  定位到选择

3. 布筋面
  偏移面
  保护层
  钢筋层数
  当前层
  层间距
  偏移模式
  反向

4. 主布筋区间
  布筋区间长度
  左侧距离
  右侧距离
  左侧端部模式
  右侧端部模式
  引导线反向
  左侧复制间距列表
  右侧复制间距列表

5. 辅助布筋区间
  辅助引导线
  辅助区间长度
  左 / 右端距离
  端部模式
  反向

6. 钢筋间距参数
  根数优先
  间距优先
  间距列表
  根数
  间距
  间距列表文本
  间距比例
  余量
  束内根数

7. 轴线处理
  轴线经孔洞处理方式
  孔洞保留曲线段号
  轴线延长 / 截断

8. 延长与弯钩
  切向延长 / 截断首端
  切向延长 / 截断末端
  首端辅助延长
  末端辅助延长
  首端弯钩
  末端弯钩

9. 钢筋属性
  名称 / 式样名
  等级
  直径
  颜色
  线宽
  生成实体钢筋对象
  钢筋实体类型

10. 式样
  样式表
  应用
  替换
  添加
  删除
  保存
  重置

11. 操作
  预览
  应用
  取消
```

默认展开：

```text
当前命令摘要
选择对象
布筋面
主布筋区间
辅助布筋区间
钢筋间距参数
钢筋属性
操作
```

默认折叠：

```text
轴线处理
延长与弯钩
式样
```

辅助布筋区间补充说明：

```text
辅助布筋区间在 P0 不再作为折叠的高级项处理。
它必须进入算法路径，和主布筋区间一起影响最终点位和端距结果。
```

边界：

```text
完整保留是 UI / DTO / 字段槽位层面的约束。
辅助布筋区间主路径必须进入 P0 算法还原范围。
孔洞、端部延长、弯钩和式样表也属于 RebarSmart 定距钢筋算法复刻目标。
第一版可以按证据成熟度分批实现，但不能从长期目标中删除。
这些能力后续必须按 IDA / PDF / 运行证据逐项闭合。
```

## 阶段分级建议

P0 必做：

```text
布筋面选择
保护层
层间距
布筋面反向
主布筋引导线
主区间左右距离
辅助布筋引导线
辅助区间左右距离
辅助区间端部调整
辅助引导线反向
根数优先
间距优先
间距列表
根数
间距
余量只读显示
束内根数
名称 / 式样名
等级
直径
生成实体钢筋对象
预览 / 应用 / 取消
```

P0 仍需保留字段槽位，并列为后续必须复刻项：

```text
轴线经孔洞处理
轴线延长 / 截断
首末端辅助延长
首末端弯钩
应用式样
```

P0 开发口径：

```text
“P0 必做”指第一条可运行主路径。
“P0 保留字段槽位”指 UI / DTO / 保存格式不能删这些字段。

如果某字段算法证据不足：
  UI 可以禁用或显示“待复刻”。
  DTO / 保存结构仍需保留。
  不能在代码里假装该字段不存在。
```

P1 再展开：

```text
端部模式完整枚举
左 / 右侧复制间距列表完整解析
辅助区间异常分支和边界 case
孔洞处理完整算法
切向延长 / 辅助延长 / 弯钩所有 LxD / RxD 倍率字段
式样表应用 / 替换 / 添加 / 删除 / 保存 / 重置
```

总原则：

```text
RebarSmart 定距钢筋相关算法都要复刻。
P0 / P1 / P2 只是实现顺序，不是功能取舍。
```

## 重要边界

```text
RebarSmart 原始 UI 是 GenerateRebarFixDistanceDlg 弹窗。
TS_RS 右侧 Dock 是新系统交互设计，不是 RebarSmart 原样行为。

字段语义和联动规则来自 RebarSmart。
布局方式服务 TS_RS 的 OCCT 选择、预览、应用流程。
```

运行时边界：

```text
TS_RS 不调用 RebarSmart DLL。
TS_RS 不依赖 3DE / CAA。
本文档中的 DLL 只作为逆向证据源。
FixDistanceCenterlineGenerator 后续不能直接 include OCCT / AIS 类型。
```

## 待确认

| GAP | 内容 | 建议证据 |
|---|---|---|
| GAP-RS-FD-001 | `ComboBoxModeGuideSurfaceOffset` 每个枚举值的中文名 | Dlg.CATNls / 实际 3DE |
| GAP-RS-FD-002 | `ModeGuideCurveZoneAdjust` 每个枚举值的中文名 | Dlg.CATNls / 实际 3DE |
| GAP-RS-FD-003 | 辅助布筋区间首末端距离 `Ratio=1.0` 的真实单位和 UI 显示 | INI + 实际 3DE 截图 |
| GAP-RS-FD-004 | 左侧复制 / 右侧复制列表的完整语法和异常处理 | AfrRebarCore3D / 实际运行 |
| GAP-RS-FD-005 | 辅助布筋区间对最终钢筋点位的完整影响 | AfrRebarObjImp / 运行样例 |
| GAP-RS-FD-006 | 轴线经孔洞处理 mode=0/1/2... 的真实语义 | PDF / IDA / 实际运行 |
| GAP-RS-FD-007 | 弯钩 LxD / RxD 字符串字段的默认字符串内容和格式 | INI / Dlg / 实际运行 |
| GAP-RS-FD-008 | `FormGuideCurveZoneArray` 中孔洞、关键点、端点保护修正的完整伪代码 | AfrRebarObjImp |
| GAP-RS-FD-009 | `PointLengthPartition_*` 系列函数的真实行为 | AfrRebarCore3D / AfrRebarCore |
| GAP-RS-FD-010 | 2002008 项目覆盖配置与 Default 配置的差异是否影响默认面板 | 项目级 INI 对比 |

这些缺口后续继续通过 IDA MCP、PDF 使用说明和 RebarSmart 运行确认闭合。

## 下一步

```text
TODO-005 已完成：
  docs/architecture/01_纯算法层实现计划.md

下一步 TODO-006：
  初始化 app 工程骨架。

边界：
  只创建最小 CMake / CTest 和目录骨架。
  不实现 SpaceListParser、分布算法或定距钢筋生成器。
```
