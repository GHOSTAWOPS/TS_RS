# TODO-028 RebarCreationCommandService P0 实现记录

## 结论

TODO-028 已完成 `RebarCreationCommandService P0`。

本轮把 `FixDistanceCenterlineGenerator P0` 的生成结果接入
`RebarModel` preview / commit 事务：

```text
RebarCreationCommandService.previewFixDistance(...)
  -> IFixDistanceCenterlineGenerator
  -> RebarSmartFixDistanceCenterlineGeneratorAdapter
  -> generateFixDistanceCenterlines(...)
  -> RebarModel.replacePreview(...)
  -> RebarModel.commitPreview()
```

这只是 P0 命令服务主链路，不表示完整钢筋 UI、完整 RebarSmart
定距钢筋复刻或 Detail 导出闭环已经完成。

## 新增代码

新增端口：

```text
app/src/application/commands/RebarCreationPorts.h
```

新增 RebarSmart 适配器：

```text
app/src/application/rebarsmart/RebarCreationCommandServiceFactory.h
app/src/application/rebarsmart/RebarCreationCommandServiceFactory.cpp
app/src/application/rebarsmart/RebarSmartFixDistanceCenterlineGeneratorAdapter.h
app/src/application/rebarsmart/RebarSmartFixDistanceCenterlineGeneratorAdapter.cpp
```

修改：

```text
app/src/application/commands/RebarCreationCommandService.h
app/src/application/commands/RebarCreationCommandService.cpp
app/tests/application/rebar_creation_command_service_tests.cpp
app/tests/application/command_service_guardrails_tests.cpp
app/CMakeLists.txt
```

## 关键边界

`RebarCreationCommandService` 不直接 include 具体 generator：

```text
rebarsmart/generators/FixDistanceGenerator.h
```

`RebarCreationCommandService` 也不直接 include 具体 RebarSmart adapter：

```text
application/rebarsmart/RebarSmartFixDistanceCenterlineGeneratorAdapter.h
```

具体 generator 只在适配器中使用：

```text
app/src/application/rebarsmart/RebarSmartFixDistanceCenterlineGeneratorAdapter.cpp
```

真实装配只在 factory 中发生：

```text
app/src/application/rebarsmart/RebarCreationCommandServiceFactory.cpp
```

这样保持：

```text
UI
  -> RebarCreationCommandService
  -> IFixDistanceCenterlineGenerator port
  -> RebarCreationCommandServiceFactory / RebarSmart adapter
  -> RebarModel transaction
```

## P0 行为

`previewFixDistance` 当前支持：

```text
1. 校验 RebarModel 是否存在。
2. 校验 geometry engine 是否存在。
3. 校验 generator port 是否存在。
4. 调用 FixDistance centerline generator port。
5. 将生成的 centerline GeometryRef.stableId 写入 RebarGroupDraft。
6. 通过 RebarModel.replacePreview(...) 建立 preview。
7. preview 不 dirty，不进入 committed groups。
8. commitPreview 后写入 committed groups 并 dirty。
9. generator 失败时透传 diagnosticCode，不创建 preview。
```

新增 diagnostic：

```text
REBAR_CREATION_GEOMETRY_MISSING
REBAR_CREATION_GENERATOR_MISSING
```

## 测试覆盖

`tsrs_rebar_creation_command_service_tests` 覆盖：

```text
service 缺少 RebarModel 时返回 REBAR_CREATION_MODEL_MISSING。
previewDraft / commit / cancel 仍走 RebarModel transaction。
previewFixDistance 生成 preview，不 commit，不 dirty。
commitPreview 后 committed group 数量增加，并设置 dirty。
生成结果保留 groupId / commandId / styleName / grade / diameterM / bundleCount。
生成结果保留 centerline stable id。
Fake generator 覆盖 CommandService 只通过 IFixDistanceCenterlineGenerator port 消费结果。
缺少 geometry engine 返回 REBAR_CREATION_GEOMETRY_MISSING。
缺少 generator port 返回 REBAR_CREATION_GENERATOR_MISSING。
generator 失败时透传 RS_FIX_DISTANCE_AUXILIARY_CURVE_INVALID。
generator 失败不创建 preview / commit state。
```

`tsrs_command_service_boundary_scan_tests` 覆盖：

```text
ui / presentation 不直接依赖 generator / Detail writer。
application/commands 不直接 include 具体 FixDistanceGenerator / FixNumberGenerator。
application/commands 不直接 include application/rebarsmart adapter / factory。
rebarsmart / drawing 不泄漏 OCCT / AIS 类型。
```

## 验证记录

专项验证：

```powershell
cmake --build build/default --config Debug --target `
  tsrs_rebar_creation_command_service_tests `
  tsrs_command_service_guardrails_tests `
  tsrs_command_service_boundary_scan_tests

ctest --test-dir build/default -C Debug -R `
  "tsrs_rebar_creation_command_service_tests|tsrs_command_service_guardrails_tests|tsrs_command_service_boundary_scan_tests" `
  --output-on-failure
```

结果：

```text
3/3 passed
```

默认验证：

```powershell
cmake --build build/default --config Debug -- -j1
ctest --test-dir build/default -C Debug --output-on-failure
```

结果：

```text
29/29 passed
```

边界扫描：

```powershell
rg -n "TopoDS_|AIS_|BRep|TopAbs|gp_|Geom_|Qt" app/src/domain app/src/rebarsmart
rg -n "FixDistanceGenerator|FixNumberGenerator|RebarModel" app/src/ui app/src/presentation/occ
rg -n "DetailPackage|StbGroup|StbGeo|StbTable|MaterialTable|drawing/detail" app/src/application app/src/domain app/src/rebarsmart
rg -n "application/rebarsmart|RebarSmartFixDistanceCenterlineGeneratorAdapter|rebarsmart/generators/FixDistanceGenerator|rebarsmart/generators/FixNumberGenerator" app/src/application/commands app/src/ui app/src/presentation/occ
```

结果：

```text
domain/rebar 与 rebarsmart 未命中 OCCT / Qt 泄漏。
ui / presentation 未命中 generator / RebarModel 直接依赖。
application/commands 未直接 include 具体 RebarSmart generator。
application/commands 未直接 include application/rebarsmart adapter / factory。
DetailPackageCommandService 仍是既有 guardrail 占位，不是本轮新增耦合。
```

## 本轮未做

```text
不实现 FixNumber 命令服务。
不实现 UI 参数面板。
不接 Viewer 真实选择。
不实现 Detail exporter。
不实现 StbGroupWriter / StbTable / MaterialTable。
不计算钢筋真实长度 / 重量。
不把 Detail 字段写入 RebarModel。
不声称完整 RebarSmart 定距钢筋复刻完成。
```

## 对下一节点的影响

TODO-028 完成后，可以进入：

```text
TODO-029 Generator result -> RebarModel -> Detail minimal exporter
```

TODO-029 必须继续保持：

```text
Detail 字段不能反向污染 RebarModel。
只能做最小映射和极简 Detail 包导出。
不能把 TODO-024 v2 minimal sheet pass 扩大解释为完整 Detail 兼容。
```
