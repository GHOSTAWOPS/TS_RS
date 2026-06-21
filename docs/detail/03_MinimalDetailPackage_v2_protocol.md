# Minimal Detail Package v2 Protocol

## 结论

`v2_empty_groups` 是当前 TODO-024 / GC-004 的 P0 minimal sheet baseline。

它已经通过旧图石 AutoCAD 插件导入按钮路径人工验证。

`v3_one_pointstb` 也已通过导入，但它只作为后续“最小 StbGroup / pointStb 兼容样本”，不是 P0 baseline。

## 适用范围

本协议只定义 minimal sheet `DetailNN.stl`。

它不定义：

```text
完整工程图协议。
完整 StbGroup。
完整 StbGeo。
StbTable。
MaterialTable。
RebarModel -> DetailExporter 映射。
```

## 最小 Sheet 必需节点

P0 minimal sheet 必须包含：

```text
DrawingRoot
StbTables
HViewPorts
ViewPort
PartDetailDrawing
General-Info
continue-line
hidden-line
central-line
section-line
hatch-line
steeljoint-line
StbDetailDrawing
StbGroups
```

`StbGroups` 可以为空：

```xml
<StbGroups stbGroupCount="0"/>
```

`StbGroup` / `StbGeo` 不是最小 sheet 必需项。

`StbTable` / `MaterialTable` 不是当前 v2 最小 sheet 必需项，但后续完整表格兼容需要单独验证。

## General-Info P0 字段

当前 v2 baseline 使用真实图石样本兼容字段集合。

P0 重点字段：

```text
ExportYesNo="T"
ExpSteelYesNo="T"
ExpSteelMark="T"
DrawingUnit="1000"
DrawingScale="100"
GeneralScale="50"
DrawingType="0"
LevelDrawing="0"
```

注意：

```text
DrawingUnit 不使用 "mm"。
DrawingUnit 使用数值形式，例如 "1000"。
```

## Line Category 容器

当前 P0 writer 对以下 line category 都保留统一 primitive 容器：

```text
continue-line
hidden-line
central-line
section-line
hatch-line
steeljoint-line
```

每类保留：

```text
lines
circles
Arcs
Ellipses
EllipseArcs
Splines
```

`section-line` 至少包含一条 `Line1`。

## ZValue 格式

`ZValue` 必须使用真实图石样本兼容格式：

```text
0.000000:0.000000:10.000000
```

不得使用空格分隔格式：

```text
0 0 0
```

当前只锁定兼容格式，不解释三段数值完整语义。

## Detail.xml

TS_RS 仍写：

```xml
<StyleRoot/>
```

原因：

```text
保持普通 Detail 包目录兼容。
```

但本次人工验证路径使用旧图石 AutoCAD 插件导入按钮和临时目录替换法，实际读取的是临时目录中的 `DetailNN.stl`。

因此：

```text
Detail.xml 不是本次按钮路径的必要条件。
```

## 人工验证摘要

```text
AutoCAD 版本：AutoCAD 2020
插件导入方式：使用图石当前可成功的 AutoCAD 插件导入按钮路径
临时目录：M:\Device\C\Users\x\AppData\Local\Temp\msohtmplcllip
截图：screenshot not attached
```

结果：

```text
v1 minimal:
  failed, unhandled e06d7363h exception

v2_empty_groups:
  passed

v3_one_pointstb:
  passed, future pointStb compatibility sample only
```

测试：

```text
Test A: only v2 Detail01.stl, standalone single sheet passed
Test B: v2 replacing real Detail01.stl passed
Test C: v2 replacing real Detail03.stl passed and v2 geometry was visible/imported
```
