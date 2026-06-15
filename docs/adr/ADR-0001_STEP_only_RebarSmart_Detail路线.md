# ADR-0001：STEP-only + RebarSmart 钢筋生成 + 图石 Detail 兼容路线

状态：Accepted

日期：2026-06-15

## 决策

TS_RS 采用独立 Qt6 + OCCT 桌面程序路线：

```text
STEP/STP
  -> OCCT 导入、显示、选择、几何计算
  -> RebarSmart-style 钢筋生成器
  -> RebarModel / ScheduleModel / DrawingModel
  -> 图石 Detail 包兼容导出
```

其中：

```text
Qt6 负责界面、命令入口、Dock 面板。
OCCT 负责 STEP 导入、AIS 显示、点线面选择、几何计算、钢筋实体化和导出 STEP。
RebarSmart 只作为钢筋生成逻辑、参数字段、默认值、算法证据源。
老图石 / VisualTS 只作为 Detail 包、工程图字段、CAD 插件兼容证据源。
```

## 运行时允许依赖

```text
C++20
Qt6
OCCT
自研 RebarModel / DrawingModel / DetailPackageExporter
必要的开源 XML / JSON / CSV / XLSX 辅助库
```

## 运行时禁止依赖

```text
RebarSmart DLL
VisualTS EXE / DLL
3DE / CAA / CATIA / 3DEXPERIENCE runtime
ACIS / Spatial
HOOPS
Codejock
旧 AutoCAD 插件作为核心依赖
```

老 AutoCAD 插件只作为短期兼容导入和人工验证工具。

## 证据源分工

```text
RebarSmart3DE：
  钢筋创建逻辑、GenerateRebarXXXDlg 参数、INI 默认值、GenerateRebarXXXImp 算法。

老图石 / VisualTS：
  Detail.xml + DetailNN.stl XML 包、工程图、表格、剖切、投影、CAD 插件导入口径。

父目录旧开发：
  只能参考 Qt6 / OCCT 工程写法、STEP 导入、AIS Viewer、选择 ID。
  不迁入父目录钢筋业务生成器作为 TS_RS 主线。
```

## 非目标

```text
不复刻旧 VisualTS UI 像素级外观。
不复刻旧图石所有菜单和操作流程为 P0。
不直接调用 RebarSmart 或 VisualTS 二进制。
不把 Detail 包作为内部主保存格式。
不承诺 P0 打开旧 SFL / SAT / SAB / DWG / DXF。
不把 OCCT 能做到什么当成钢筋业务规则。
```

## 模块边界

```text
ui/
  只负责参数显示、选择状态、命令入口、预览 / 应用 / 取消。

presentation/occ/
  只负责 AIS Viewer、颜色、高亮、选择、剖视显示。

step/
  负责 STEP 导入、ShapeStore、TopologyBinding。

geometry/
  提供 IGeometryEngine 和 OCCT 实现。
  RebarSmart 生成器只能依赖几何接口，不能直接写 UI 或 AIS。

rebarsmart/
  复刻 RebarSmart 参数模型、INI、分布、分区、弯钩、导引线、各 GenerateRebarXXX 生成器。

domain/rebar/
  保存新系统钢筋业务对象，不泄漏 TopoDS / AIS / BRep 类型。

drawing/
  保存 DrawingModel、ScheduleModel 和工程图中间表达。

export/detail/
  把 DrawingModel / RebarModel / ScheduleModel 映射为 Detail 包 XML。
```

## 后果

正面：

```text
避开 3DE / CAA / ACIS / HOOPS 授权依赖。
钢筋生成主证据源比旧 VisualTS 更清晰。
UI 可以按现代 CAD 软件重做。
Detail 包优先兼容老 CAD 插件，比直接写 DWG/DXF 更快验证。
```

代价：

```text
RebarSmart 算法仍需 IDA / PDF / 运行截图逐项闭合。
OCCT 几何结果和 3DE 可能存在差异，需要 golden 验证。
Detail 包字段容忍度需要 AutoCAD 插件实测。
需要维护 RebarModel、DrawingModel、DetailModel 之间的映射。
```

## 当前落地顺序

```text
M0：路线与架构冻结。
M1：RebarSmart 证据索引、参数模型、INI 字段。
M2：纯算法层，先不接 OCCT。
M3：GeometryEngine + FixDistance / FixNumber。
M4：Qt6 / OCCT 操作闭环。
M5：Detail 包 Reader / Writer / Exporter。
M6：扩展筋型、工程图质量、golden。
```
