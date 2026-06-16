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
```

如果当前 next 是只读审计或文档节点：

```text
1. 不修改代码。
2. 需要时可使用 xhigh 只读 review。
3. 输出结论必须落到 docs/。
4. todo.csv 必须推进到下一个明确节点。
```

## 当前 next

```text
TODO-013：
  FixDistanceGenerator P0。
```

目标：

```text
基于已完成的 IGeometryEngine / OcctGeometryEngine P0，
实现简单定距钢筋中心线生成器。
```

输出：

```text
app/src/rebarsmart/generators 下的 FixDistanceGenerator P0。
app/tests 下的生成器单元/集成测试。
必要的实现记录 / todo.csv 更新。
```

边界：

```text
不接 Qt。
不接 Detail。
不实现 FixNumberGenerator。
不把未确认的 RebarSmart 边界行为写成确定结论。
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
```

当前 next：

```text
TODO-013：
  FixDistanceGenerator P0。

边界：
  不接 Qt。
  不接 Detail。
  不实现 FixNumberGenerator。
  不把未确认的 RebarSmart 边界行为写成确定结论。
```

中期顺序：

```text
TODO-007 SpaceListParser
TODO-008 PriorityCount / PrioritySpace
TODO-009 PriorityList
TODO-010 GuideSurfaceOffset / GuideCurveZone
TODO-011 IGeometryEngine 接口设计
TODO-012 OcctGeometryEngine P0
TODO-013 FixDistanceGenerator P0
TODO-014 FixNumberGenerator P0
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

当前从 TODO-007 开始：
实现 SpaceListParser。
只改 app/src/rebarsmart 与 app/tests/rebarsmart。
先补测试，再实现。
不接 Qt，不接 OCCT，不接 Detail。
```
