# TODO-020 STEP 导入与水泥灰显示实现记录

## 结论

TODO-020 当前按外部严格评审意见收窄为：

```text
真实 STEP/STP 导入
  -> 结构统计 diagnostic
  -> OCCT Viewer 统一水泥灰显示
  -> UI 通过 StepImportCommandService 调用导入链路
```

本节点不等于：

```text
TopologyBinding 已完成。
Viewer 点 / 边 / 面选择已完成。
钢筋生成器已接真实选择。
Detail 包兼容已完成。
P0A 最小闭环已完成。
```

当前状态：

```text
done。
```

## 本轮修正的外部评审问题

外部评审指出当前代码存在两处边界泄漏：

```text
MainWindow 直接创建 StepImportService。
OccViewerWidget 直接依赖 StepImportResult。
```

本轮已修正为：

```text
MainWindow
  -> StepImportCommandService
  -> StepImportService
  -> StepDisplayModel
  -> OccViewerWidget.displayStepModel(...)
```

当前 `StepDisplayModel` 仍可在 presentation/occ 边界内持有 `TopoDS_Shape`。
这是 TODO-020 P0 显示链路允许的近似，不是长期 ShapeStore / TopologyBinding 设计。

## 已实现内容

### Step import

代码：

```text
app/src/step/StepImportService.h
app/src/step/StepImportService.cpp
```

能力：

```text
支持 .stp / .step。
缺文件、扩展名错误、读取失败、TransferRoots 失败、空 shape、异常均返回 diagnostic。
统计 root / solid / face / edge / vertex 数量。
当前使用 STEPControl_Reader::OneShape() 作为 P0 显示 shape。
```

边界：

```text
OneShape() 只作为 TODO-020 显示路径。
不能把 OneShape() 当成最终 ShapeStore / TopologyBinding 来源。
```

### Application command service

代码：

```text
app/src/application/commands/StepImportCommandService.h
app/src/application/commands/StepImportCommandService.cpp
```

能力：

```text
UI 不再直接调用 StepImportService。
导入结果被转换为 command-level result。
成功时产生 presentation 层 StepDisplayModel。
失败时保留 diagnosticCode / diagnostic。
```

### Viewer display model

代码：

```text
app/src/presentation/occ/StepDisplayModel.h
app/src/presentation/occ/OccViewerWidget.h
app/src/presentation/occ/OccViewerWidget.cpp
```

能力：

```text
OccViewerWidget 不再 include step/StepImportService.h。
OccViewerWidget 不再消费 StepImportResult。
displayStepModel(...) 按固定 cement-gray 样式显示结构模型。
```

### UI chain

代码：

```text
app/src/ui/mainwindow/MainWindow.cpp
```

能力：

```text
MainWindow::importStepFile(...) 只调用 StepImportCommandService。
导入成功后记录 face / edge / vertex 数量。
导入失败后显示 command diagnostic。
```

## 测试

新增测试：

```text
app/tests/application/step_import_command_service_tests.cpp
app/tests/application/step_import_local_sample_probe_tests.cpp
```

覆盖：

```text
缺失 STEP 文件时返回 STEP_IMPORT_MISSING_FILE。
成功导入合成 STEP fixture 后产生 StepDisplayModel。
StepDisplayModel 包含非空 shape 和 face / edge / vertex 统计。
本地真实样本探针在未设置 TSRS_LOCAL_STEP_SAMPLE 时跳过；
设置后可导入本机真实 STEP 并打印拓扑统计。
```

已运行：

```powershell
cmd.exe /d /s /c """D:\Visual Studio 2026\Community\VC\Auxiliary\Build\vcvars64.bat"" >nul && cmake --build build/default --target tsrs_step_import_command_service_tests --config Debug"

cmd.exe /d /s /c """D:\Visual Studio 2026\Community\VC\Auxiliary\Build\vcvars64.bat"" >nul && ctest --test-dir build/default -R tsrs_step_import_command_service_tests --output-on-failure -C Debug"
```

结果：

```text
tsrs_step_import_command_service_tests passed, 1/1。
```

已运行：

```powershell
cmd.exe /d /s /c """D:\Visual Studio 2026\Community\VC\Auxiliary\Build\vcvars64.bat"" >nul && cmake --build build/default --target tsrs_ui tsrs_app --config Debug"
```

结果：

```text
tsrs_ui / tsrs_app build passed。
```

已运行默认 CTest：

```powershell
ctest --test-dir build/default --output-on-failure -C Debug
```

结果：

```text
17/17 passed。
```

已运行真实样本探针：

```powershell
$env:PATH='D:\Work\vcpkg\installed\x64-windows\debug\bin;D:\Work\vcpkg\installed\x64-windows\bin;' + $env:PATH
$env:TSRS_LOCAL_STEP_SAMPLE='..\123.stp'
build\default\tsrs_step_import_local_sample_probe_tests.exe
```

结果：

```text
source=..\123.stp
rootCount=1
solidCount=754
faceCount=3016
edgeCount=9048
vertexCount=18096
```

已运行真实样本 app smoke：

```powershell
$env:QT_QPA_PLATFORM='minimal'
$env:QT_QPA_PLATFORM_PLUGIN_PATH='D:\Work\vcpkg\installed\x64-windows\debug\Qt6\plugins\platforms'
$env:PATH='D:\Work\vcpkg\installed\x64-windows\debug\bin;D:\Work\vcpkg\installed\x64-windows\bin;' + $env:PATH
build\default\tsrs_app.exe --smoke --step ..\123.stp
```

结果：

```text
exit code = 0
```

GC-001 记录：

```text
docs/validation/golden_cases/GC-001/README.md
```

已补可视化截图：

```text
docs/validation/golden_cases/GC-001/tsrs_app_123stp_window_20260617.png
```

观察结果：

```text
TS_RS 主窗口成功打开。
中央 OCCT Viewer 显示 123.stp 模型。
消息栏显示 faces=3016 / edges=9048 / vertices=18096。
模型按统一浅灰 / 水泥灰风格显示，未使用 STEP 源文件颜色。
```

## 边界检查

已运行：

```powershell
rg -n "StepImportService|StepImportResult|displayImportedStep" app/src/ui app/src/presentation
```

结果：

```text
无匹配。
```

已运行：

```powershell
rg -n "TopoDS_|AIS_|BRep|TopAbs_|gp_|Geom_" app/src/rebarsmart
```

结果：

```text
无匹配。
```

说明：

```text
app/src/domain 当前尚不存在，所以 domain/rebar 泄漏检查暂按无对象记录。
```

已运行：

```powershell
rg -n "EdgeToRebarFactory|FaceRebarGenerator|LegacyUiCommandMap|OccLegacyGeometryAdapter|VisualTS" app/src
rg -n "FixDistanceGenerator|FixNumberGenerator" app/src/ui
rg -n "TopoDS_|AIS_|BRep|TopAbs_|gp_|Geom_" app/src/application
```

结果：

```text
均无匹配。
```

## xhigh 只读 review

本轮涉及代码、测试、CMake 和 UI / presentation / step 边界，
按项目规则在 commit 前执行 xhigh 只读 review。

review 结论：

```text
Critical: 无。
Important: 无。
Verdict: 允许将 TODO-020 标记为 done，并进入 commit / tag / push。
```

子代理状态：

```text
xhigh 只读 review 完成后，已关闭不再需要的子代理。
```

## Headless smoke 口径

本轮发现 Qt `minimal` 平台下，真实 AIS Viewer 窗口显示可能返回失败。
因此 `--smoke --step` 现在只验证 STEP 导入 command 链路，
不要求 Viewer 在 headless/minimal 环境完成 AIS 显示。

正常交互模式仍使用：

```text
MainWindow::importStepFile(...)
  -> displayStepModel(...)
```

headless smoke 使用：

```text
MainWindow::importStepFileForSmoke(...)
  -> 只导入和统计，不显示 Viewer
```

这不是宣布水泥灰可视化截图已完成。
可视化显示仍需后续手工截图或专门的 viewer 截图 gate 验证。

## 当前 GAP

| ID | 缺口 | 处理 |
| --- | --- | --- |
| TODO020-GAP-001 | 合成 box 测试不能代表真实工程 STEP。 | 已补 GC-001：`123.stp` 导入统计和 app smoke。后续继续补更多样本。 |
| TODO020-GAP-002 | OneShape() 只适合显示，不适合长期 ShapeStore / TopologyBinding。 | 新增 TODO-020B。 |
| TODO020-GAP-003 | 当前没有自动像素检查证明水泥灰显示真实可见。 | 已补 GC-001 截图；后续可加 viewer 截图/像素 gate。 |
| TODO020-GAP-004 | StepDisplayModel 暂时持有 TopoDS_Shape。 | 限定在 presentation/occ 显示边界；不得进入 domain/rebar 或 rebarsmart。 |

## 下一步建议

进入下一个节点：

```text
TODO-020B ShapeStore + TopologyBindingRegistry P0。
```

不要在 TODO-020 内继续实现：

```text
选择系统。
TopologyBindingRegistry。
生成器接入。
Detail Reader / Writer。
```
