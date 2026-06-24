# TODO-021 Viewer 选择系统实现记录

## 结论

TODO-021 已完成 Viewer 选择系统的 P0 切片：

```text
ViewerSelectionMode
  -> OccViewerWidget 内部翻译为 OCCT selection mode
  -> MainWindow 使用当前 StepSession 的 TopologyBindingRegistry
  -> SelectionCommandService
  -> stable binding id
```

本节点完成的是选择模式、颜色语义和 stable binding 出口，不接 RebarSmart
generator，不接 Detail，不创建 RebarModel。

## 本节点不表示

```text
完整鼠标 picking 工作流已人工验收。
选择结果已接入定距 / 定数钢筋生成器。
选择结果已写入 RebarModel。
框选、反选、选择集管理已完成。
高亮材质已完成真实视觉验收。
```

## 修改范围

代码：

```text
app/src/presentation/occ/OccViewerWidget.h
app/src/presentation/occ/OccViewerWidget.cpp
app/src/ui/mainwindow/MainWindow.h
app/src/ui/mainwindow/MainWindow.cpp
```

测试：

```text
app/tests/presentation/occ_viewer_selection_tests.cpp
app/tests/ui/main_window_skeleton_tests.cpp
app/tests/CMakeLists.txt
```

没有修改：

```text
app/src/rebarsmart
app/src/drawing/detail
app/src/domain/rebar
app/src/geometry/occt generator logic
```

## 实现内容

### 1. ViewerSelectionMode

新增 TS_RS 自有选择枚举：

```text
None
Vertex
Edge
Face
Shape
```

外层 UI / 测试只使用 `ViewerSelectionMode`。
`TopAbs_*` 只在 `presentation/occ` 内部使用，避免 UI API 暴露 OCCT 枚举。

### 2. OccViewerWidget 选择模式

新增：

```text
setSelectionMode(ViewerSelectionMode)
selectionMode()
selectionModeStatusText()
```

内部实现：

```text
context_->Deactivate()
ViewerSelectionMode::Vertex -> AIS_Shape::SelectionMode(TopAbs_VERTEX)
ViewerSelectionMode::Edge   -> AIS_Shape::SelectionMode(TopAbs_EDGE)
ViewerSelectionMode::Face   -> AIS_Shape::SelectionMode(TopAbs_FACE)
ViewerSelectionMode::Shape  -> Activate(0)
ViewerSelectionMode::None   -> 不激活选择模式
```

### 3. 颜色语义

新增：

```text
defaultViewerSelectionColors()
```

当前锁定语义：

```text
selectedSemantic = selection-red
highlightSemantic = highlight-magenta
```

这对应 UI 文档里的：

```text
红色：点 / 线 / 面选中
洋红：高亮、定位、查找结果、悬停强调
```

### 4. MainWindow stable binding 选择出口

新增 smoke 观察口：

```text
resolveFirstSelectionForSmoke(kind)
currentSelectionStableId()
currentSelectionKind()
```

`resolveFirstSelectionForSmoke("edge")` 使用当前 `StepSession`：

```text
StepSession.topologyBindings.edges().front()
  -> makeBindingReference(...)
  -> SelectionCommandService.resolveSelection(...)
  -> currentSelectionStableId
```

该路径证明 MainWindow 后续可以从 Viewer / session 选择进入
`SelectionCommandService`，并得到 stable binding id。

## TDD 记录

先新增红灯测试：

```text
app/tests/presentation/occ_viewer_selection_tests.cpp
app/tests/ui/main_window_skeleton_tests.cpp
```

红灯失败：

```text
OccViewerWidget 缺 selectionMode / setSelectionMode / selectionModeStatusText。
MainWindow 缺 resolveFirstSelectionForSmoke / currentSelectionStableId / currentSelectionKind。
```

然后补最小实现，目标测试转绿。

## 测试覆盖

`tsrs_occ_viewer_selection_tests` 覆盖：

```text
1. Viewer selection mode 默认是 None。
2. setSelectionMode(Edge) 后 selectionMode() 返回 Edge。
3. Edge 选择模式状态文案包含“边”。
4. selected 颜色语义为 selection-red。
5. highlight 颜色语义为 highlight-magenta。
```

`tsrs_main_window_skeleton_tests` 新增覆盖：

```text
1. MainWindow 导入 STEP 后已有 StepSession。
2. MainWindow 能通过 SelectionCommandService 解析第一条 edge binding。
3. currentSelectionStableId 非空。
4. currentSelectionKind == edge。
```

边界扫描：

```text
app/src/ui 不出现 TopAbs_ / AIS_ / TopoDS_ / BRep / gp_ / Geom_。
app/src/ui / app/src/presentation/occ / app/src/application/commands 不直接引用 generator / Detail writer。
app/src/rebarsmart / app/src/drawing 不出现 OCCT/AIS 类型。
```

## 验证记录

目标测试：

```powershell
cmake --build build/default --target tsrs_occ_viewer_selection_tests tsrs_main_window_skeleton_tests --config Debug
ctest --test-dir build/default -C Debug -R tsrs_occ_viewer_selection_tests --output-on-failure
ctest --test-dir build/default -C Debug -R tsrs_main_window_skeleton_tests --output-on-failure
```

结果：

```text
tsrs_occ_viewer_selection_tests = passed
tsrs_main_window_skeleton_tests = passed
```

默认 CTest：

```text
待最终提交前记录。
```

## 关闭的 GAP

```text
TODO020E-GAP-002:
  Viewer selection 仍未从 session 输出 stable binding id。
  -> TODO-021 已通过 MainWindow smoke selection + SelectionCommandService 关闭 P0 出口。

TODO020B-GAP-005:
  真实 Viewer selection -> binding id 尚未接通。
  -> TODO-021 已完成 P0 stable binding 出口；完整鼠标 picking 仍作为残余风险。
```

## 剩余风险

| ID | 风险 | 处理 |
| --- | --- | --- |
| TODO021-GAP-001 | 当前 automated test 使用 smoke 方式解析第一条 binding，不等于完整人工鼠标 picking 验收。 | 后续 Viewer 交互增强或人工 UI 记录补证据。 |
| TODO021-GAP-002 | 选中/悬停颜色语义已锁定，但真实 OCCT 高亮材质还未做视觉截图验收。 | 后续 UI 视觉验证补。 |
| TODO021-GAP-003 | Shape 选择模式当前使用 AIS default mode，子对象选择只覆盖 vertex/edge/face。 | 后续选择集管理补。 |

## 下一步

下一节点：

```text
TODO-020F STEP import unit and scale policy
```

原因：

```text
TODO-021 已让 Viewer / MainWindow 具备 stable binding 出口。
进入 RebarModel / Schedule / generator->Detail 链路前，
必须先明确 STEP source unit -> TS_RS internal meter 合同，
避免钢筋参数、几何长度和 Detail 输出出现 1000 倍错误。
```
