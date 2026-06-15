# 定距钢筋参数面板与 IDA 证据

本文档记录 RebarSmart `定距布筋` 参数的证据来源，以及 TS_RS 第一版右侧 Dock 参数面板设计。

核心结论：

```text
RebarSmart 原始实现是 3DE/CATIA 弹窗 GenerateRebarFixDistanceDlg。
TS_RS 不照搬 3DE 弹窗外观，但定距钢筋的参数分组、字段语义和联动规则应尽量沿用 RebarSmart。
```

## 证据来源

IDA MCP 会话：

```text
rebarsmart_creation = RebarCreation.dll
rebarsmart_afrorebarobjimp = AfrRebarObjImp.dll
afr_rebar_core3d = AfrRebarCore3D.dll
rebarsmart_workbench = RebarSmart3deWorkbench.dll
```

资源文件：

```text
C:\Users\ghost\Desktop\reverse_engineering\【03】图石软件\RebarSmart_DrwToolKit2022X 20260604\win_b64\resources\msgcatalog\GenerateRebarFixDistanceDlg.CATNls
C:\Users\ghost\Desktop\reverse_engineering\【03】图石软件\RebarSmart_DrwToolKit2022X 20260604\win_b64\resources\msgcatalog\Simplified_Chinese\RebarSmart3deWorkbenchHeader.CATNls
C:\Users\ghost\Desktop\reverse_engineering\【03】图石软件\RebarSmart_DrwToolKit2022X 20260604\ResourceRebarSmart\Default\GenerateRebarFixDistance.ini
C:\Users\ghost\Desktop\reverse_engineering\【03】图石软件\RebarSmart_DrwToolKit2022X 20260604\ResourceRebarSmart\2002008\GenerateRebarFixDistance.ini
```

关键函数：

```text
RebarCreation.dll
  0x1800C0970 fctCreateGenerateRebarFixDistanceCmd
  0x1800B64A0 GenerateRebarFixDistanceCmd 构造链
  0x1800C1E80 GenerateRebarFixDistanceDlg UI 初始化

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

本次复核确认：

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

`RebarGSRootAttributeBasicParm_Initialize` 确认：

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

`RebarGSRootAttributeGuideSurface_Initialize` 确认：

```text
保护层默认值：0.05 m
层间距默认值：0.2 m
布筋面偏移相关开关默认开启
布筋面层数 / active layer 相关字段默认存在
```

`RebarGSRootAttributeExtendHook_Initialize` 确认：

```text
首端弯钩创建：默认关闭
末端弯钩创建：默认关闭
弯钩长度默认值：3 * 钢筋直径
弯钩角度默认值：0
弯钩半径 / 倍率类字段默认值：1.2
```

`RebarGSRootAttributePassHole_Initialize` 确认：

```text
轴线经孔洞处理模式默认值：0
```

## 根数 / 间距 / 余量联动

`GenerateRebarCore::UpdateRebarCountSpaceMargin` 是定距布筋的关键联动函数。

已确认逻辑：

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
  调用 CalPointCountOnGuideCurve / CalSectionCountOnGuideCurve
  反算根数和余量
```

结论：

```text
TS_RS 定距钢筋不能只做“间距输入框”。
P0 必须至少保留：根数优先、间距优先、间距列表三种入口。
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

结论：

```text
主布筋区间和辅助布筋区间里的“左侧距离 / 右侧距离”不是普通独立数字。
它们和直径、间距、保护层、层间距、偏移模式存在联动。
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
1. 选择对象
  选择布筋面
  选择布筋引导线
  选择辅助布筋引导线

2. 布筋面
  偏移面
  保护层
  钢筋层数
  当前层
  层间距
  偏移模式
  反向

3. 主布筋区间
  布筋区间长度
  左侧距离
  右侧距离
  左侧端部模式
  右侧端部模式
  引导线反向
  左侧复制间距列表
  右侧复制间距列表

4. 辅助布筋区间
  辅助引导线
  辅助区间长度
  左 / 右端距离
  端部模式
  反向

5. 钢筋间距参数
  根数优先
  间距优先
  间距列表
  根数
  间距
  间距列表文本
  余量
  束内根数

6. 轴线处理
  轴线经孔洞处理方式
  轴线延长 / 截断

7. 延长与弯钩
  首端辅助延长
  末端辅助延长
  首端弯钩
  末端弯钩

8. 钢筋属性
  编号
  等级
  直径
  生成实体钢筋对象

9. 操作
  预览
  应用
  取消
```

默认展开：

```text
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
编号
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

## 待确认

```text
1. ComboBoxModeGuideSurfaceOffset 的每个枚举值中文含义。
2. ModeGuideCurveZoneAdjust 的每个枚举值在原 UI 中如何命名。
3. 左侧复制 / 右侧复制列表的完整语法和异常处理。
4. 辅助布筋区间对最终钢筋点位的完整影响。
5. 轴线经孔洞处理 mode=0/1/2... 的真实语义。
6. 弯钩 LxD / RxD 字符串字段的默认字符串内容和格式。
```

这些缺口后续继续通过 IDA MCP、PDF 使用说明和 RebarSmart 运行确认闭合。
