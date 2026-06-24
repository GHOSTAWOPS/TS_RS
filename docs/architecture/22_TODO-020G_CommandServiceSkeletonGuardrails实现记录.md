# TODO-020G CommandService skeleton guardrails 实现记录

## 结论

TODO-020G 已补齐 Viewer 选择系统前的 Application CommandService 护栏：

```text
SelectionCommandService
DetailPackageCommandService
RebarCreationCommandService
```

本节点只建立边界和结构测试，不接真实 Viewer selection、不接 Detail exporter、
不接 RebarSmart generator、不创建 RebarModel。

## 本节点不表示

```text
Viewer 点 / 边 / 面选择已完成。
Detail 导出命令已能导出真实工程图。
RebarCreationCommandService 已能创建钢筋。
RebarModel transaction 已完成。
UI 可以直接调用 generator / Detail writer。
```

## 修改范围

代码：

```text
app/src/application/commands/SelectionCommandService.h
app/src/application/commands/SelectionCommandService.cpp
app/src/application/commands/DetailPackageCommandService.h
app/src/application/commands/DetailPackageCommandService.cpp
app/src/application/commands/RebarCreationCommandService.h
app/src/application/commands/RebarCreationCommandService.cpp
app/CMakeLists.txt
```

测试：

```text
app/tests/application/command_service_guardrails_tests.cpp
app/tests/application/command_service_boundary_scan_tests.cpp
app/tests/CMakeLists.txt
```

没有修改：

```text
app/src/ui/mainwindow
app/src/presentation/occ selection
app/src/rebarsmart/generators
app/src/drawing/detail
app/src/domain/rebar
```

## 实现内容

### 1. SelectionCommandService

新增 `SelectionCommandService`，当前职责是把 UI / Viewer 后续传入的
`TopologyBindingReference` 交给 `TopologyBindingRegistry::restore()` 解析，并输出：

```text
role
kind
bindingStableId
usedFallback
diagnosticCode
diagnostic
```

这为 TODO-021 Viewer 选择系统提供 stable binding 出口。

如果没有 registry：

```text
SELECTION_REGISTRY_MISSING
```

### 2. DetailPackageCommandService

新增 `DetailPackageCommandService` 骨架。

当前只返回：

```text
DETAIL_PACKAGE_COMMAND_NOT_IMPLEMENTED
```

它的意义是先冻结 UI -> CommandService -> Detail 导出的调用边界，
不允许 UI 直接调用 `DetailPackageWriter`。

### 3. RebarCreationCommandService

新增 `RebarCreationCommandService` 骨架。

当前只返回：

```text
REBAR_CREATION_COMMAND_NOT_IMPLEMENTED
```

它的意义是先冻结 UI -> CommandService -> generator / RebarModel transaction
的调用边界，不允许 UI 直接调用 RebarSmart generator。

## TDD 记录

先新增红灯测试：

```text
app/tests/application/command_service_guardrails_tests.cpp
```

红灯失败：

```text
fatal error C1083:
无法打开包括文件:
application/commands/DetailPackageCommandService.h
```

然后补最小 CommandService 骨架和 CMake 接线，目标测试转绿。

## 测试覆盖

`tsrs_command_service_guardrails_tests` 覆盖：

```text
1. SelectionCommandService 能通过 registry 恢复 stable binding id。
2. SelectionCommandService 无 registry 时返回 SELECTION_REGISTRY_MISSING。
3. DetailPackageCommandService 当前只返回 not implemented 诊断。
4. RebarCreationCommandService 当前只返回 not implemented 诊断。
```

`tsrs_command_service_boundary_scan_tests` 覆盖：

```text
1. app/src/ui 不直接 include / 引用 RebarSmart generator。
2. app/src/ui 不直接 include / 引用 DetailPackageReader / DetailPackageWriter。
3. app/src/application/commands 骨架不直接 include 具体 generator / Detail writer。
4. app/src/rebarsmart 与 app/src/drawing 不出现 TopoDS_ / AIS_ / BRep / TopAbs / gp_ / Geom_。
```

## 验证记录

目标测试：

```powershell
cmake --build build/default --target tsrs_command_service_guardrails_tests --config Debug
ctest --test-dir build/default -C Debug -R tsrs_command_service_guardrails_tests --output-on-failure

cmake --build build/default --target tsrs_command_service_boundary_scan_tests --config Debug
ctest --test-dir build/default -C Debug -R tsrs_command_service_boundary_scan_tests --output-on-failure
```

结果：

```text
tsrs_command_service_guardrails_tests = passed
tsrs_command_service_boundary_scan_tests = passed
```

默认 CTest：

```text
待最终提交前记录。
```

## 关闭的 GAP

```text
TODO020E-GAP-003:
  ShapeStore / TopologyBindingRegistry 当前仍持有 OCCT 类型，
  不能进入 generator / RebarModel。
  -> TODO-020G 已用 CommandService skeleton 和结构扫描测试建立护栏。
```

## 剩余风险

| ID | 风险 | 处理 |
| --- | --- | --- |
| TODO020G-GAP-001 | SelectionCommandService 只解析 binding reference，还没有接真实 Viewer selection event。 | TODO-021 处理。 |
| TODO020G-GAP-002 | DetailPackageCommandService 只是 not implemented 骨架。 | 后续 Detail exporter / DrawingModel 节点处理。 |
| TODO020G-GAP-003 | RebarCreationCommandService 只是 not implemented 骨架。 | TODO-025 / TODO-028 处理 RebarModel transaction 和生成命令。 |
| TODO020G-GAP-004 | 结构扫描是字符串级 guardrail，不等于完整架构证明。 | xhigh review 和后续代码审查继续检查。 |

## 下一步

下一节点：

```text
TODO-021 Viewer 选择系统
```

原因：

```text
TODO-020E 已有 StepSession / ImportedModelStore。
TODO-020G 已有 CommandService 护栏。
TODO-024 minimal sheet autoin 已人工通过。
因此可以进入 Viewer 选择系统，但必须输出 stable binding id，
不能把裸 AIS / TopoDS 交给 generator。
```
