# TODO-027 ScheduleModel P0 实现记录

## 结论

TODO-027 已完成 `ScheduleModel` 基础下料表 P0。

本轮只从 `RebarModel` 已提交钢筋组生成基础数量摘要：

```text
RebarModel.committedGroups()
  -> buildBasicSchedule(...)
  -> BasicSchedule rows / totals
```

本轮不接 Detail，不接 UI，不接 Viewer，不接生成器主链路。

## 新增代码

新增：

```text
app/src/drawing/schedule/ScheduleModel.h
app/src/drawing/schedule/ScheduleModel.cpp
app/tests/drawing/schedule_model_tests.cpp
```

新增测试 target：

```text
tsrs_schedule_model_tests
```

## P0 字段

`ScheduleRow` 当前字段：

```text
groupId
commandId
styleName
grade
diameterM
bundleCount
centerlineCount
barCount
lengthKnown
totalLengthM
```

`BasicSchedule` 当前字段：

```text
rows
totalGroupCount
totalBarCount
hasKnownLengths
totalKnownLengthM
```

## 关键边界

当前 `RebarModel` 只有：

```text
RebarGroupDraft.centerlineStableIds
```

它没有保存真实中心线几何长度。

因此 TODO-027 不伪造长度：

```text
lengthKnown = false
totalLengthM = 0.0
hasKnownLengths = false
totalKnownLengthM = 0.0
```

真实长度后续必须来自正式生成器 / 几何链路写入的业务事实，
不能由 ScheduleModel 根据 stable id 猜测。

## 计数规则

P0 下料表数量规则：

```text
centerlineCount = centerlineStableIds.size()
barCount = centerlineCount * bundleCount
totalGroupCount = committed groups count
totalBarCount = sum(row.barCount)
```

`preview` 不进入下料表。

## TDD 记录

RED：

```text
cmake --build build/default --config Debug --target tsrs_schedule_model_tests
```

结果：

```text
fatal error C1083: cannot open include file:
drawing/schedule/ScheduleModel.h
```

GREEN：

```text
ctest --test-dir build/default -C Debug -R "tsrs_schedule_model_tests" --output-on-failure
```

结果：

```text
1/1 passed
```

## 默认验证

默认串行构建和 CTest：

```text
cmake --build build/default --config Debug -- -j1
ctest --test-dir build/default -C Debug --output-on-failure
```

结果：

```text
29/29 passed
```

边界扫描：

```text
rg -n "TopoDS_|AIS_|BRep|TopAbs|gp_|Geom_" app/src/domain app/src/project app/src/rebarsmart app/src/drawing
rg -n "StbGroup|StbGeo|StbTable|MaterialTable|DetailPackage|Detail\\.xml|DetailNN|drawing/detail" app/src/drawing/schedule app/src/domain app/tests/drawing/schedule_model_tests.cpp
```

结果：

```text
无命中。
```

## 本轮未做

```text
不计算真实长度。
不计算重量。
不输出 Excel / CSV。
不接 DetailPackageWriter。
不接 DrawingModel。
不接 RebarCreationCommandService。
不把 Detail 字段写入 ScheduleModel。
```

## 对下一节点的影响

TODO-027 完成后，可以进入：

```text
TODO-028 RebarCreationCommandService P0
```

TODO-028 应把生成器结果通过 Application Service 写入 `RebarModel`
transaction。

仍需保持：

```text
UI -> CommandService -> Generator -> RebarModel Transaction
```

不能让 UI 直接调用 generator 或直接改 RebarModel。
