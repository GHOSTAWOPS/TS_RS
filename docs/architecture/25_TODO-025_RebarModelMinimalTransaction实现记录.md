# TODO-025 RebarModel Minimal Transaction 实现记录

## 结论

TODO-025 已完成 RebarModel 最小事务壳：

```text
RebarGroupDraft
  -> RebarModel.replacePreview(...)
  -> RebarModel.commitPreview()
  -> RebarModel.cancelPreview()
```

本轮只建立 TS_RS 自己的钢筋模型事务边界。

本轮不实现真实钢筋生成业务，不接 UI，不接 Detail，不接 `.tsrebar` 保存。

## 新增代码

新增：

```text
app/src/domain/rebar/RebarModel.h
app/src/domain/rebar/RebarModel.cpp
```

最小 DTO：

```text
RebarGroupDraft
  groupId
  commandId
  styleName
  grade
  diameterM
  bundleCount
  centerlineStableIds
```

当前模型状态：

```text
preview draft
committed groups
dirty flag
```

诊断：

```text
REBAR_MODEL_OK
REBAR_MODEL_INVALID_DRAFT
REBAR_MODEL_NO_PREVIEW
```

## Application Service 接入

`RebarCreationCommandService` 新增事务出口：

```text
previewDraft(...)
commitPreview()
cancelPreview()
```

服务必须持有 `RebarModel*` 才能执行事务。

没有模型时返回：

```text
REBAR_CREATION_MODEL_MISSING
```

原有 `previewFixDistance(...)` 仍保持：

```text
REBAR_CREATION_COMMAND_NOT_IMPLEMENTED
```

这表示 TODO-025 只完成事务壳，不代表 FixDistance / FixNumber 生成器已经接入命令服务。

## 测试

新增：

```text
app/tests/domain/rebar_model_transaction_tests.cpp
app/tests/application/rebar_creation_command_service_tests.cpp
```

覆盖：

```text
1. replacePreview 接收有效 draft。
2. preview 不创建 committed group。
3. preview 不标记 dirty。
4. commitPreview 创建 committed group。
5. commitPreview 清空 preview。
6. commitPreview 标记 dirty。
7. cancelPreview 清空 preview。
8. cancelPreview 不创建 group、不标记 dirty。
9. 无 preview commit 返回 REBAR_MODEL_NO_PREVIEW。
10. 无效 draft 返回 REBAR_MODEL_INVALID_DRAFT。
11. RebarCreationCommandService 无模型返回 REBAR_CREATION_MODEL_MISSING。
12. RebarCreationCommandService 通过 RebarModel 完成 preview / commit / cancel。
```

## TDD 记录

RED：

```text
cmake --build build/default --config Debug --target tsrs_rebar_model_transaction_tests tsrs_rebar_creation_command_service_tests
```

结果：

```text
fatal error C1083: cannot open include file: domain/rebar/RebarModel.h
```

GREEN：

```text
cmake --build build/default --config Debug --target tsrs_rebar_model_transaction_tests tsrs_rebar_creation_command_service_tests
ctest --test-dir build/default -C Debug -R "tsrs_rebar_model_transaction_tests|tsrs_rebar_creation_command_service_tests" --output-on-failure
```

结果：

```text
2/2 passed
```

## 默认验证

默认 CTest 使用串行构建验证：

```text
cmake --build build/default --config Debug -- -j1
ctest --test-dir build/default -C Debug --output-on-failure
```

结果：

```text
27/27 passed
```

备注：

```text
并行构建时，多个 OCCT/STEP 测试目标的 post-build copy_directory
会同时复制 vcpkg bin 到同一个 build/default 目录，
Windows 下可能出现 Permission denied。
串行构建可稳定通过。
```

该问题属于构建脚本并发门禁问题，不属于 TODO-025 业务逻辑失败。

## 边界

本轮没有做：

```text
不接 FixDistanceGenerator / FixNumberGenerator。
不从 Viewer 选择直接写 RebarModel。
不接 DetailPackageWriter。
不实现 ScheduleModel。
不实现 .tsrebar save/open。
不实现 undo / redo。
不实现完整钢筋字段。
```

`domain/rebar` 必须继续保持：

```text
不依赖 TopoDS_ / AIS_ / BRep / TopAbs / gp_ / Geom_。
```

## 对下一节点的影响

TODO-025 完成后，可以进入：

```text
TODO-026 .tsrebar minimal save/open
```

但 TODO-026 只能保存 P0 最小工程状态和引用摘要。

不要把 Detail 字段、OCCT shape、AIS 对象或 RebarSmart 运行时对象写入 `.tsrebar`。
