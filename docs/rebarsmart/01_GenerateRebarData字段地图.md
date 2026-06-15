# GenerateRebarData 字段地图

本文档是 TS_RS 复刻 RebarSmart 钢筋生成逻辑的公共参数入口。

核心结论：

```text
GenerateRebarData 不是某一个钢筋命令的参数。
它是所有 RebarSmart-style 钢筋生成器共享的公共参数层。

TS_RS 后续的定距、定数、角筋、拉结筋、插筋、柱锥环盘面筋、箍筋、自定义钢筋，
都必须先复用这套公共字段，再叠加各自生成器专属字段。
```

## 证据来源

本轮使用的证据：

```text
RebarSmart_DrwToolKit2022X 20260604/RebarSmart3DE_逆向分析与OCCT重建方案.md
RebarSmart_DrwToolKit2022X 20260604/ResourceRebarSmart/Default/RebarGenerate.ini
RebarSmart_DrwToolKit2022X 20260604/ResourceRebarSmart/Default/RebarGrade.ini
RebarSmart_DrwToolKit2022X 20260604/ResourceRebarSmart/Default/RebarDiameter.ini
RebarSmart_DrwToolKit2022X 20260604/ResourceRebarSmart/Default/RebarWeight.ini
RebarSmart_DrwToolKit2022X 20260604/ResourceRebarSmart/Default/GenerateRebarFixDistance.ini
RebarSmart_DrwToolKit2022X 20260604/ResourceRebarSmart/Default/GenerateRebarFixNumber.ini
RebarSmart_DrwToolKit2022X 20260604/ResourceRebarSmart/Default/GenerateRebarCorner.ini
RebarSmart_DrwToolKit2022X 20260604/ResourceRebarSmart/Default/GenerateRebarTie.ini
RebarSmart_DrwToolKit2022X 20260604/ResourceRebarSmart/Default/GenerateRebarDowel.ini
RebarSmart_DrwToolKit2022X 20260604/ResourceRebarSmart/Default/GenerateRebarFrustum.ini
RebarSmart_DrwToolKit2022X 20260604/ResourceRebarSmart/Default/GenerateRebarCustomize.ini
RebarSmart_DrwToolKit2022X 20260604/ResourceRebarSmart/Default/GenerateRebarSweep.ini
```

IDA MCP 状态：

```text
2026-06-15 检查时，当前只存在 RebarCreation.dll 会话：
  tsrs_rebarcreation

本轮没有重新打开 AfrRebarCore3D.dll / AfrRebarObjImp.dll。
所以本文档把“分析文档中声称来自 AfrRebarCore3D.dll 的结构”作为高价值二手证据，
但不把内存布局、类字段偏移写成最终事实。
```

## 字段置信等级

本文档使用三个等级：

```text
确定：
  来自 INI 落盘字段、默认值、Ratio，或多文件共同字段。

高置信推断：
  来自现有逆向分析文档的 getter/setter 结构草图，
  且能和 INI 字段语义对应。

GAP：
  需要继续用 IDA MCP / RebarSmart 实际运行 / PDF 截图确认。
```

## 模板字段数量

已用 GBK 解码检查 `Default/GenerateRebar*.ini`：

| INI | Title 字段数 | TableRow0 值数 | Ratio 数 | 说明 |
|---|---:|---:|---:|---|
| GenerateRebarFixDistance.ini | 57 | 57 | 57 | P0 定距钢筋 |
| GenerateRebarFixNumber.ini | 41 | 41 | 41 | P0 定数钢筋 |
| GenerateRebarCorner.ini | 42 | 42 | 42 | P0 角筋 |
| GenerateRebarTie.ini | 51 | 51 | 51 | P0 拉结筋 |
| GenerateRebarDowel.ini | 57 | 57 | 57 | P0 插筋 |
| GenerateRebarFrustum.ini | 41 | 41 | 41 | P0 柱锥环盘面筋 |
| GenerateRebarCustomize.ini | 10 | 10 | 10 | P0 自定义钢筋 |
| GenerateRebarSweep.ini | 54 | 54 | 54 | P1/P2 等参扫掠候选 |

结论：

```text
INI 的 Title / TableRow / Ratio 三者数量一致，可以作为字段映射依据。
不同生成器字段数不同，不能把 FixDistance 的 57 字段强套给所有生成器。
```

## 所有生成器共同字段

在已检查的 8 个 `GenerateRebar*.ini` 中，所有文件共同出现的字段：

```text
名称
基本参数_等级
基本参数_直径
基本参数_间距
基本参数_颜色
基本参数_线宽
基本参数_根数
基本参数_束内根数
钢筋实体_钢筋实体生成的标识
钢筋实体_钢筋实体类型
```

对应 TS_RS 公共 DTO：

| INI 字段 | 建议 DTO 字段 | 类型 | 单位 / 取值 | 置信 |
|---|---|---|---|---|
| 名称 | `styleName` | string | 模板名，如 `式样XX` | 确定 |
| 基本参数_等级 | `basic.grade` | string | `A/B/C/D/E/G/H/I/HA/DX/#3...` | 确定 |
| 基本参数_直径 | `basic.diameterM` | double | 内部米；UI 多按毫米显示 | 确定 |
| 基本参数_间距 | `basic.spacingM` | double | 内部米；UI 多按厘米/毫米显示待确认 | 确定 |
| 基本参数_颜色 | `basic.color` | int | INI 默认 `255` | 确定 |
| 基本参数_线宽 | `basic.lineWeight` | int | INI 默认常见 `2` | 确定 |
| 基本参数_根数 | `basic.count` | int | 根数 | 确定 |
| 基本参数_束内根数 | `basic.bundleCount` | int | 并筋 / 束内根数 | 确定 |
| 钢筋实体_钢筋实体生成的标识 | `solid.createSolid` | bool | `true/false` | 确定 |
| 钢筋实体_钢筋实体类型 | `solid.mode` | int | 具体枚举含义待确认 | 确定字段 / GAP 枚举 |

## 单位与 Ratio 规则

INI 中每个字段都有对应 `Ratio`。已观察到：

```text
基本参数_直径:
  value=0.032
  ratio=1000.0
  UI 显示大概率为 32 mm

基本参数_间距:
  value=0.2
  ratio=100.0
  UI 显示大概率为 20 cm 或按界面约定转换

布筋面_保护层厚度:
  value=0.05
  ratio=100.0
  UI 显示大概率为 5 cm

端部弯钩首端_角度:
  value=0
  ratio=57.29577951471995
  这是 rad -> degree 的换算系数
```

TS_RS 约定：

```text
1. DTO 内部长度统一用米。
2. DTO 内部角度统一用弧度。
3. UI 显示值必须通过 INI Ratio / 面板规则换算。
4. 写测试时，禁止把 UI 显示单位和内部单位混用。
```

GAP：

```text
Ratio=100.0 的字段到底在 RebarSmart UI 上显示为 cm 还是工程习惯值，
需要结合 PDF / 实际 3DE 界面继续确认。
```

## 公共 DTO 草案

后续 C++ 可按这个结构拆文件，但本轮不写 C++：

```cpp
struct RebarBasicParams {
    std::string styleName;
    std::string grade;
    std::string remark;
    std::string guid;

    double diameterM;
    double spacingM;
    double secondarySpacingM;

    int count;
    int secondaryCount;
    int bundleCount;
    int secondaryBundleCount;

    int color;
    int lineWeight;
    int rebarType;
};

struct RebarSolidParams {
    bool createSolid;
    int mode;
};

struct GuideSurfaceParams {
    bool offsetEnabled;
    int offsetMode;
    double coverThicknessM;
};

struct DistributionParams {
    int priorityMode;
    int marginMode;
    double headDistanceM;
    double tailDistanceM;
    int headAdjustMode;
    int tailAdjustMode;
    bool headExtendEnabled;
    bool tailExtendEnabled;
    std::string headSpaceList;
    std::string tailSpaceList;
    std::string spacingList;
    double spacingRatio;
};

struct SecondaryDistributionParams {
    double headDistanceM;
    double tailDistanceM;
    int headAdjustMode;
    int tailAdjustMode;
    int marginMode;
    int priorityMode;
    std::string spacingList;
    double spacingRatio;
};

struct PassHoleParams {
    int mode;
    int keepSegmentNo;
};

struct TangentialExtendParams {
    double headLengthM;
    double tailLengthM;
    std::string headLengthDiameterExpr;
    std::string tailLengthDiameterExpr;
};

struct AuxiliaryExtendParams {
    bool headEnabled;
    bool tailEnabled;
    double headLengthM;
    double tailLengthM;
    std::string headLengthDiameterExpr;
    std::string tailLengthDiameterExpr;
};

struct HookParams {
    bool headEnabled;
    bool tailEnabled;
    bool headFilletInset;
    bool tailFilletInset;
    bool headReverse;
    bool tailReverse;
    double headLengthM;
    double tailLengthM;
    std::string headLengthDiameterExpr;
    std::string tailLengthDiameterExpr;
    double headRadiusM;
    double tailRadiusM;
    std::string headRadiusDiameterExpr;
    std::string tailRadiusDiameterExpr;
    double headAngleRad;
    double tailAngleRad;
};

struct GenerateRebarData {
    RebarBasicParams basic;
    RebarSolidParams solid;
    GuideSurfaceParams guideSurface;
    DistributionParams distribution;
    SecondaryDistributionParams secondaryDistribution;
    PassHoleParams passHole;
    TangentialExtendParams tangentialExtend;
    AuxiliaryExtendParams auxiliaryExtend;
    HookParams hook;
};
```

说明：

```text
上面是 TS_RS 的“开发 DTO 草案”，不是 RebarSmart 原始类内存布局。
它的目标是服务纯算法层、UI Dock 参数绑定、.tsrebar 保存。
```

## 字段分组地图

### 基本参数

| INI 字段 | DTO 字段 | 出现范围 | 置信 |
|---|---|---|---|
| 名称 | `basic.styleName` 或模板外层 `styleName` | 所有已查 INI | 确定 |
| 基本参数_等级 | `basic.grade` | 所有已查 INI | 确定 |
| 基本参数_直径 | `basic.diameterM` | 所有已查 INI | 确定 |
| 基本参数_间距 | `basic.spacingM` | 所有已查 INI | 确定 |
| 基本参数_颜色 | `basic.color` | 所有已查 INI | 确定 |
| 基本参数_线宽 | `basic.lineWeight` | 所有已查 INI | 确定 |
| 基本参数_根数 | `basic.count` | 所有已查 INI | 确定 |
| 基本参数_束内根数 | `basic.bundleCount` | 所有已查 INI | 确定 |
| 基本参数_次方向的间距 | `basic.secondarySpacingM` | Dowel / Sweep | 确定 |
| 基本参数_次方向的根数 | `basic.secondaryCount` | Dowel / Sweep | 确定 |
| 基本参数_次方向钢筋束内根数 | `basic.secondaryBundleCount` | Dowel / Sweep | 确定 |

### 钢筋实体

| INI 字段 | DTO 字段 | 出现范围 | 置信 |
|---|---|---|---|
| 钢筋实体_钢筋实体生成的标识 | `solid.createSolid` | 所有已查 INI | 确定 |
| 钢筋实体_钢筋实体类型 | `solid.mode` | 所有已查 INI | 确定字段 / GAP 枚举 |

### 布筋面

| INI 字段 | DTO 字段 | 出现范围 | 置信 |
|---|---|---|---|
| 布筋面_保护层厚度 | `guideSurface.coverThicknessM` | FixDistance / FixNumber / Dowel / Frustum / Sweep | 确定 |
| 布筋面_布筋面偏移 | `guideSurface.offsetEnabled` | FixDistance / FixNumber / Dowel / Frustum / Sweep | 确定 |
| 布筋面_布筋面保护层偏移模式 | `guideSurface.offsetMode` | FixDistance / FixNumber / Dowel / Frustum / Sweep | 确定字段 / GAP 枚举 |

说明：

```text
Corner / Tie / Customize 的 INI 不含布筋面字段。
这些生成器可能不以面偏移作为核心输入，不能强行显示同一组布筋面字段。
```

### 布筋区间

| INI 字段 | DTO 字段 | 出现范围 | 置信 |
|---|---|---|---|
| 布筋区间首端_布筋区间端部距离 | `distribution.headDistanceM` | 多数创建命令 | 确定 |
| 布筋区间末端_布筋区间端部距离 | `distribution.tailDistanceM` | 多数创建命令 | 确定 |
| 布筋区间首端_布筋区间端部距离的调整方式 | `distribution.headAdjustMode` | 多数创建命令 | 确定字段 / GAP 枚举 |
| 布筋区间末端_布筋区间端部距离的调整方式 | `distribution.tailAdjustMode` | 多数创建命令 | 确定字段 / GAP 枚举 |
| 布筋区间_布筋区间余量分配模式 | `distribution.marginMode` | 多数创建命令 | 确定字段 / GAP 枚举 |
| 布筋区间首端_区间两端的扩展布筋标识 | `distribution.headExtendEnabled` | 多数创建命令 | 确定 |
| 布筋区间末端_区间两端的扩展布筋标识 | `distribution.tailExtendEnabled` | 多数创建命令 | 确定 |
| 布筋区间首端_区间两端的布筋间距列表 | `distribution.headSpaceList` | 多数创建命令 | 确定 |
| 布筋区间末端_区间两端的布筋间距列表 | `distribution.tailSpaceList` | 多数创建命令 | 确定 |
| 布筋区间_布筋点分布的优先模式 | `distribution.priorityMode` | 多数创建命令 | 确定字段 / 高置信枚举 |
| 布筋区间_钢筋间距列表 | `distribution.spacingList` | 多数创建命令 | 确定 |
| 布筋区间_布筋点间距的比例 | `distribution.spacingRatio` | 多数创建命令 | 确定 |

高置信枚举：

```text
distribution.priorityMode:
  0 = 间距优先，从尾端开始
  1 = 间距优先，从头端开始
  2 = 间距优先，头端余量居中
  3 = 两端向中间，对称
  4 = 均匀分配，自动调间距
  5 = 均匀分配 + 多一根
  6 = 间距优先，截断到尾端
```

证据：

```text
RebarSmart3DE_逆向分析与OCCT重建方案.md
  GetPointLength_ProiriySpace
  RebarDistributeParams::modePriority
```

### 次方向布筋区间

这些字段只在需要二维分布的生成器中出现：

```text
GenerateRebarDowel.ini
GenerateRebarSweep.ini
```

| INI 字段 | DTO 字段 | 置信 |
|---|---|---|
| 次方向布筋区间首端_次方向的布筋区间端部距离 | `secondaryDistribution.headDistanceM` | 确定 |
| 次方向布筋区间末端_次方向的布筋区间端部距离 | `secondaryDistribution.tailDistanceM` | 确定 |
| 次方向布筋区间首端_次方向的布筋区间端部距离的调整方式 | `secondaryDistribution.headAdjustMode` | 确定字段 / GAP 枚举 |
| 次方向布筋区间末端_次方向的布筋区间端部距离的调整方式 | `secondaryDistribution.tailAdjustMode` | 确定字段 / GAP 枚举 |
| 次方向布筋区间_次方向的布筋区间余量分配模式 | `secondaryDistribution.marginMode` | 确定字段 / GAP 枚举 |
| 次方向布筋区间_次方向的布筋点分布的优先模式 | `secondaryDistribution.priorityMode` | 确定字段 / GAP 枚举 |
| 次方向布筋区间_次方向的钢筋间距列表 | `secondaryDistribution.spacingList` | 确定 |
| 次方向布筋区间_插筋的布筋点间距的比例 | `secondaryDistribution.spacingRatio` | 确定 |

### 钢筋轴线穿孔

| INI 字段 | DTO 字段 | 出现范围 | 置信 |
|---|---|---|---|
| 钢筋轴线穿孔_钢筋轴线穿孔模式 | `passHole.mode` | FixDistance / FixNumber / Dowel / Frustum / Sweep | 确定字段 / GAP 枚举 |
| 钢筋轴线穿孔_钢筋轴线穿孔保留曲线段号 | `passHole.keepSegmentNo` | FixDistance / FixNumber / Dowel / Frustum / Sweep | 确定 |

分析文档给出的高置信枚举：

```text
passHole.mode:
  0 = 无
  1 = 自动
  2 = 手动
```

GAP：

```text
需要用 PDF / 实际运行确认中文 UI 是否就是“无 / 自动 / 手动”。
```

### 切向延长 / 截断

| INI 字段 | DTO 字段 | 置信 |
|---|---|---|
| 切向延长(截断)首端_长度 | `tangentialExtend.headLengthM` | 确定 |
| 切向延长(截断)末端_长度 | `tangentialExtend.tailLengthM` | 确定 |
| 切向延长(截断)首端_长度倍径 | `tangentialExtend.headLengthDiameterExpr` | 确定 |
| 切向延长(截断)末端_长度倍径 | `tangentialExtend.tailLengthDiameterExpr` | 确定 |

说明：

```text
FixDistance.ini 中存在“长度倍径”字段。
FixNumber / Corner / Tie / Dowel / Frustum / Sweep 默认 INI 多数只保留长度字段。
TS_RS DTO 应保留倍径槽位，但面板是否显示要按具体 Dlg 证据决定。
```

### 辅助延长

| INI 字段 | DTO 字段 | 置信 |
|---|---|---|
| 辅助延长首端_生成端部延长 | `auxiliaryExtend.headEnabled` | 确定 |
| 辅助延长末端_生成端部延长 | `auxiliaryExtend.tailEnabled` | 确定 |
| 辅助延长首端_长度 | `auxiliaryExtend.headLengthM` | 确定 |
| 辅助延长末端_长度 | `auxiliaryExtend.tailLengthM` | 确定 |
| 辅助延长首端_长度倍径 | `auxiliaryExtend.headLengthDiameterExpr` | 确定 |
| 辅助延长末端_长度倍径 | `auxiliaryExtend.tailLengthDiameterExpr` | 确定 |

GAP：

```text
分析文档中的 refDirHead / refSurHead / refDirTail / refSurTail
在本轮 INI 中没有直接落盘字段。
这些应归到 geometry binding / selection binding，不应直接塞进纯参数 DTO。
```

### 端部弯钩

| INI 字段 | DTO 字段 | 置信 |
|---|---|---|
| 端部弯钩首端_生成端部弯钩 | `hook.headEnabled` | 确定 |
| 端部弯钩末端_生成端部弯钩 | `hook.tailEnabled` | 确定 |
| 端部弯钩首端_圆角内切 | `hook.headFilletInset` | 确定 |
| 端部弯钩末端_圆角内切 | `hook.tailFilletInset` | 确定 |
| 端部弯钩首端_反转弯钩方向 | `hook.headReverse` | 确定 |
| 端部弯钩末端_反转弯钩方向 | `hook.tailReverse` | 确定 |
| 端部弯钩首端_长度 | `hook.headLengthM` | 确定 |
| 端部弯钩末端_长度 | `hook.tailLengthM` | 确定 |
| 端部弯钩首端_长度倍径 | `hook.headLengthDiameterExpr` | 确定 |
| 端部弯钩末端_长度倍径 | `hook.tailLengthDiameterExpr` | 确定 |
| 端部弯钩首端_半径 | `hook.headRadiusM` | 确定 |
| 端部弯钩末端_半径 | `hook.tailRadiusM` | 确定 |
| 端部弯钩首端_半径倍径 | `hook.headRadiusDiameterExpr` | 确定 |
| 端部弯钩末端_半径倍径 | `hook.tailRadiusDiameterExpr` | 确定 |
| 端部弯钩首端_角度 | `hook.headAngleRad` | 确定 |
| 端部弯钩末端_角度 | `hook.tailAngleRad` | 确定 |

说明：

```text
角度字段 Ratio 为 57.29577951471995，
所以 DTO 内部按 rad 保存，UI 按 degree 显示。
```

## 生成器专属字段

本文档只建立公共字段地图，但不能丢掉专属字段入口。
后续专项文档必须继续展开这些字段。

### Corner 专属

```text
角筋参数_原点X
角筋参数_原点Y
角筋参数_首端长度
角筋参数_末端长度
角筋参数_角度
角筋参数_平面模式
```

建议 DTO：

```text
CornerParams
```

### Tie 专属

```text
拉结筋参数_拉结筋尾钩是否反向
拉结筋参数_多布筋引导线1的起始位置
拉结筋参数_多布筋引导线2的起始位置
拉结筋参数_多布筋引导线1的终止位置
拉结筋参数_多布筋引导线2的终止位置
拉结筋参数_第一组布筋引导线配对文本
拉结筋参数_第二组布筋引导线配对文本
拉结筋参数_多布筋引导线1的间隔数
拉结筋参数_多布筋引导线2的间隔数
拉结筋参数_拉结筋匹配模式
拉结筋参数_拉结筋点对模式
拉结筋参数_首端尾钩模式
拉结筋参数_末端尾钩模式
拉结筋参数_首端尾钩长度
拉结筋参数_末端尾钩长度
```

建议 DTO：

```text
TieParams
```

### Dowel 专属

```text
插筋参数_插筋主长
插筋参数_插筋方向模式
插筋参数_插筋布置模式
插筋参数_插筋在大空隙处填补
插筋参数_插筋在小空隙处移除
```

建议 DTO：

```text
DowelParams
```

### Sweep 专属

```text
扫略筋参数_钢筋轴线曲线拟合模式
扫略筋参数_钢筋轴线曲线拟合点数
```

建议 DTO：

```text
SweepParams
```

### FixDistance / FixNumber / Frustum / Customize

当前默认 INI 未发现明显专属字段：

```text
FixDistance:
  公共字段最完整，包含辅助布筋区间和倍径字段。

FixNumber:
  公共字段子集，根数是关键输入，间距可能用于默认 / 反算 / UI 显示。

Frustum:
  默认 INI 看起来是公共字段子集，几何选择和锥面语义应在 Dlg / 算法层确认。

Customize:
  只有最小公共字段，具体自定义轴线应来自用户选择 / 几何输入，不在当前 INI 中展开。
```

GAP：

```text
Frustum / Customize 的真正几何参数可能不在 Default INI 里。
需要继续查 Dlg.CATNls / CATRsc / AfrRebarObjImp.dll。
```

## 材料与规格字段

### 等级

`RebarGrade.ini`：

```text
A, B, C, D, E, G, H, I, HA, DX, #3, #4, #5, #6, #7, #8, #9, #10, O, P, Q, R, S, T
```

TS_RS 建议：

```text
RebarMaterialCatalog.gradeCodes
```

### 直径

`RebarDiameter.ini` 分组：

```text
BarCHN
BarASM
BarEUR
Strand
Anchor
```

TS_RS 建议：

```text
RebarMaterialCatalog.diameterSets
```

### 重量

`RebarWeight.ini` 分组：

```text
BarCHN
BarASM
BarEUR
Strand
Anchor
```

TS_RS 建议：

```text
RebarMaterialCatalog.weightByDiameter
```

说明：

```text
重量字段大概率用于下料表 / 统计。
生成器不应硬编码重量表，应从 ResourceRebarSmart-compatible catalog 加载。
```

## UI Dock 绑定规则

后续 UI 面板必须这样绑定字段：

```text
RebarSmart Dlg / INI 字段
  ↓
TS_RS Dock 控件
  ↓
GenerateRebarData / GeneratorSpecificParams
  ↓
Generator / Service
```

禁止：

```text
UI 直接写生成算法。
UI 直接依赖 OCCT TopoDS / AIS 作为业务字段。
为了简化界面删除 RebarSmart 字段槽位。
```

允许：

```text
RebarSmart 弹窗改为 TS_RS 右侧 Dock。
字段暂不可实现时保留槽位并标“待复刻”。
对象选择字段使用 TS_RS TopologyBinding / ReferenceGeometryBinding。
```

## 需要继续确认的 GAP

| GAP | 内容 | 建议证据 |
|---|---|---|
| GAP-RS-DATA-001 | `solid.mode` 枚举含义 | Dlg.CATNls / IDA / 实际运行 |
| GAP-RS-DATA-002 | `guideSurface.offsetMode` 枚举含义 | Dlg.CATNls / IDA / 实际运行 |
| GAP-RS-DATA-003 | 端部距离调整方式枚举 | AfrRebarCore / 实际运行 |
| GAP-RS-DATA-004 | 余量分配模式枚举 | AfrRebarCore / 实际运行 |
| GAP-RS-DATA-005 | `Ratio=100.0` 在 UI 上的显示单位 | PDF / 实际 3DE 界面 |
| GAP-RS-DATA-006 | `FixNumber` 中间距字段在定数模式下的真实用途 | FixNumber Dlg / IDA |
| GAP-RS-DATA-007 | `Frustum` 几何参数来源 | Frustum Dlg / AfrRebarObjImp |
| GAP-RS-DATA-008 | `Customize` 自定义轴线 / 点列来源 | Customize Dlg / AfrRebarObjImp |
| GAP-RS-DATA-009 | 分析文档中的 `GuideCurveParams` 投影参考对象如何落到 TS_RS binding | AfrRebarCore3D / UI 截图 |
| GAP-RS-DATA-010 | `passHole.mode` 中文枚举和真实几何行为 | PDF / IDA / 实际运行 |

## 对后续开发的要求

```text
1. TODO-005 纯算法开发计划必须引用本文档。
2. SpaceListParser 只处理 `distribution.spacingList` 一类字符串，不处理 UI。
3. PrioritySpaceDistributor 只处理 `distribution.priorityMode` 和首尾端距等纯数学输入。
4. 生成器 DTO 必须拆成公共 GenerateRebarData + 专属 Params。
5. domain/rebar 不能依赖 RebarSmart INI 字段名，INI 字段名只在 adapter / params 层出现。
6. 每个钢筋创建 Dock 面板必须按对应 Dlg / INI 字段保留槽位。
```

## 下一步

```text
TODO-005：
  纯算法开发计划。

重点确认：
  SpaceListParser、PrioritySpaceDistributor、PriorityCountDistributor、
  PriorityListDistributor、GuideCurveZoneCalculator、GuideSurfaceOffsetCalculator
  的输入 / 输出 / 单位 / GAP / 测试矩阵。

边界：
  只写计划，不写 C++。
```
