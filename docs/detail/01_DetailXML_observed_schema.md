# Detail XML 观察 Schema

## 结论

当前观察到的 Detail 包不是：

```text
标准 STL
DWG
DXF
RebarModel 内部保存格式
```

而是：

```text
Detail.xml
  -> 空 StyleRoot 占位或样式文件。

DetailNN.stl
  -> XML 文本，root 为 DrawingRoot。
```

## 包结构

```text
Detail.xml
Detail01.stl
Detail02.stl
Detail03.stl
...
```

P0 不假设 `DetailNN` 上限。

Reader 应按目录扫描：

```text
Detail[0-9][0-9].stl
```

并按数字序排序。

## Detail.xml

当前真实样本：

```xml
<StyleRoot/>
```

策略：

```text
Reader:
  读取并保存原文 / root name / attributes / unknown children。

Writer:
  P0 默认写 <StyleRoot/>。
  如果 Reader 输入中存在非空样式节点，必须保守 round-trip，不主动丢弃。
```

## DetailNN.stl root

```xml
<DrawingRoot>
  <StbTables>...</StbTables>
  <HViewPorts>...</HViewPorts>
</DrawingRoot>
```

要求：

```text
Reader 必须把文件扩展名 .stl 视为兼容命名。
Reader 必须按 XML 解析。
Reader 不得走 STL mesh parser。
```

## StbTables

可能结构：

```xml
<StbTables>
  <StbTable ...>
    <StbRow1 ...>
      <StbSeg1 ...>
        <Line .../>
      </StbSeg1>
    </StbRow1>
  </StbTable>
  <MaterialTable ...>
    <MatRow1 .../>
  </MaterialTable>
</StbTables>
```

也可能为空：

```xml
<StbTables/>
```

P0 观察字段：

```text
StbTable:
  count
  HeightValue0
  HeightValueCount
  Volume1225
  NumCombineGoJians
  SteelNetArea
  ...

StbRow#:
  rsdID
  ComponentName
  SteelWay
  diameter
  length
  segNum
  sameGrpNum
  stbNumSum
  lenSum
  stbLevel
  stbLayer
  stbProfile
  stbUse
  smallTable
  mirrorType
  mirrorSEFlag

StbSeg#:
  lenRange
  deltaLen
  shapeType

MaterialTable:
  rowCount
  Mass
  Volume722
  MassNum

MatRow#:
  diameter
  lenSum
  countSum
  singleMass
  massSum
  stbLevel
```

## HViewPorts

可能结构：

```xml
<HViewPorts>
  <ViewPort>
    <PartDetailDrawing num="...">...</PartDetailDrawing>
    <StbDetailDrawing>...</StbDetailDrawing>
  </ViewPort>
</HViewPorts>
```

P0 只要求：

```text
读取 ViewPort 数量。
读取 PartDetailDrawing / StbDetailDrawing 是否存在。
保留未知节点。
```

## PartDetailDrawing

包含：

```text
General-Info
continue-line
hidden-line
central-line
section-line
hatch-line
Others
steeljoint-line
```

General-Info 已观察字段：

```text
CompanyName
ExportYesNo
ExpSteelYesNo
ExpSteelMark
Dimension*
Model_FileName
BasePoint_X
BasePoint_Y
Range_Min_X / Range_Max_X / Range_Min_Y / Range_Max_Y
CutPlaneDirX0 / CutPlaneDirY0 / CutPlaneDirZ0
CutPlaneDirX / CutPlaneDirY / CutPlaneDirZ
TopDirX / TopDirY / TopDirZ
DrawingName
DrawingUnit
DrawingScale
GeneralScale
DrawingType
LevelDrawing
CutPlanePosX / CutPlanePosY / CutPlanePosZ
DrawTaoTong
```

线容器：

```text
continue-line
hidden-line
central-line
section-line
hatch-line
```

每个线容器可能包含：

```text
lines
circles
Arcs
Ellipses
EllipseArcs
Splines
```

Line 字段：

```text
start_x
start_y
end_x
end_y
ZValue
```

## StbDetailDrawing

结构：

```xml
<StbDetailDrawing>
  <StbGroups stbGroupCount="...">
    <StbGroup1 ...>
      <Std1 segCount="...">
        <StbGeo1 .../>
      </Std1>
      <FaceEdge .../>
    </StbGroup1>
  </StbGroups>
</StbDetailDrawing>
```

StbGroup# 已观察字段：

```text
rsdID
groupID
diameter
diameter2
interval
barcount
segcount
stbNum
stbNumAct
stbLevel
stbLayer
stbProfile
stbUse
RangeLess180
ComponentName
PJSteelName
SteelWay
stbType
stbOffsetInOut
```

Std#：

```text
segCount
```

StbGeo# lineStb 已观察字段：

```text
segID
stbSeqNum
shapeType
start_x / start_y / start_z
end_x / end_y / end_z
offset_x / offset_y / offset_z
```

StbGeo# pointStb 已观察字段：

```text
segID
stbSeqNum
shapeType
point_x / point_y / point_z
offset_x / offset_y / offset_z
offset_x2 / offset_y2 / offset_z2
```

FaceEdge：

```text
shapeType
start_x / start_y
end_x / end_y
```

## P0 Schema 策略

Reader P0 不强建完整 typed model。

必须提供：

```text
fileName
sheetIndex
rootName
rawXml
knownSummary
rawAttributes
unknownChildren
parseDiagnostics
```

knownSummary 至少包括：

```text
stbTableCount
stbRowCount
materialRowCount
viewPortCount
sectionLineCount
stbGroupCount
stbGeoCount
faceEdgeCount
```

## 不确定项

```text
1. 节点顺序是否被旧 CAD 插件严格依赖。
2. StbGroup# / StbRow# 的序号是否必须连续。
3. count / stbGroupCount / rowCount 是否必须和实际节点数量一致。
4. ZValue 的三个数值语义仍需确认。
5. lineStb / pointStb / arcStb 等 stbType 的完整取值仍需确认。
6. Detail02+ 无 StbTable 是否为固定策略还是样本偶然。
```
