# TODO-020F STEP Import Unit And Scale Policy 实现记录

## 结论

TODO-020F 已完成 STEP source unit -> TS_RS internal meter 的 P0 合同：

```text
STEP file unit
  -> StepLengthUnitPolicy
  -> StepImportResult
  -> StepImportCommandResult
  -> StepSession
```

TS_RS 内部长度单位继续固定为 `m`。

本轮没有缩放 OCCT `TopoDS_Shape` 坐标。

原因：

```text
Viewer / ShapeStore / TopologyBindingRegistry 当前都基于 OCCT 导入后的 shape 坐标工作。
如果在 TODO-020F 里直接缩放 shape，会改变显示、bbox、stable binding fingerprint，
这属于更大的几何行为变更。
```

所以本轮只把单位事实显式带进导入会话，供后续 RebarModel / Schedule /
generator->Detail 链路消费。

## 新增代码

新增：

```text
app/src/step/StepUnitPolicy.h
app/src/step/StepUnitPolicy.cpp
```

核心 DTO：

```text
StepLengthUnitPolicy
  rawSourceLengthUnit
  normalizedSourceLengthUnit
  internalLengthUnit = "m"
  sourceToMeterScale
  sourceLengthUnitDetected
  sourceLengthUnitKnown
  sourceToMeterScaleAssumed
  shapeCoordinatesScaledToInternalMeters = false
```

当前 P0 已识别：

```text
mm / millimeter / millimetre -> 0.001
cm / centimeter / centimetre -> 0.01
m / meter / metre -> 1.0
in / inch -> 0.0254
ft / foot / feet -> 0.3048
```

空单位或未知单位：

```text
normalizedSourceLengthUnit = "mm" 或原始归一化字符串
sourceToMeterScale = 0.001
sourceToMeterScaleAssumed = true
sourceLengthUnitKnown = false
```

这是保守策略，不表示真实 STEP 单位已经被证明。

## STEP 导入接入

`StepImportService` 现在在 `ReadFile` 后读取 OCCT 文件单位：

```text
STEPControl_Reader::FileUnits(lengthUnits, angleUnits, solidAngleUnits)
```

并写入：

```text
StepImportResult.unitPolicy
```

参考来源：

```text
旧实现项目 OcctStepImportService.cpp 只作为 STEP/OCCT 工程零件参考：
reader.ChangeReader().FileUnits(...)
```

没有复用旧实现项目 rebar 业务逻辑。

## Application 主链路接入

`StepImportCommandService` 现在把 policy 写入：

```text
StepImportCommandResult.unitPolicy
StepSession.unitPolicy
```

后续 `.tsrebar` 保存、RebarModel transaction、ScheduleModel、Detail exporter
可以通过 StepSession 获取导入单位事实，不需要重新解析 STEP，也不需要猜单位。

## 测试

新增 / 更新测试：

```text
app/tests/step/step_import_service_tests.cpp
app/tests/application/step_import_command_service_tests.cpp
```

覆盖：

```text
1. MM -> normalized mm -> sourceToMeterScale 0.001。
2. 空单位 -> assumed mm -> sourceToMeterScale 0.001。
3. OCCT 写出的 box STEP fixture 导入后携带 unitPolicy。
4. StepImportCommandResult 携带 unitPolicy。
5. StepSession 保存同一份 unitPolicy。
6. shapeCoordinatesScaledToInternalMeters=false，明确本轮未缩放 shape。
```

## 验证

专项验证：

```text
cmake --build build/default --target tsrs_step_import_service_tests tsrs_step_import_command_service_tests --config Debug
ctest --test-dir build/default -C Debug -R "tsrs_step_import_service_tests|tsrs_step_import_command_service_tests" --output-on-failure
```

结果：

```text
2/2 passed
```

默认 CTest 和 xhigh review 见本节点提交前记录。

## 边界

本轮没有做：

```text
不缩放 TopoDS_Shape。
不改变 Viewer 显示坐标。
不改变 TopologyBindingRegistry fingerprint / bbox 逻辑。
不接 RebarModel。
不接 ScheduleModel。
不接 generator->Detail。
不实现 .tsrebar 保存。
```

## 风险与后续

| ID | 风险 | 后续 |
|---|---|---|
| TODO020F-GAP-001 | 未知 STEP 单位当前默认按 mm 假定。 | 后续真实工程样本记录 rawSourceLengthUnit 和 assumed 标志。 |
| TODO020F-GAP-002 | OCCT shape 坐标尚未统一缩放到 meter。 | 如果后续需要米坐标几何计算，必须在 GeometryEngine / RebarModel 边界显式转换。 |
| TODO020F-GAP-003 | `sourceToMeterScale` 已进入 StepSession，但尚未被 RebarModel / Schedule 消费。 | TODO-025 及后续节点处理。 |

## 对下一节点的影响

TODO-020F 完成后，可以进入：

```text
TODO-025 RebarModel minimal transaction
```

但 TODO-025 必须继续遵守：

```text
domain/rebar 不依赖 TopoDS / AIS / BRep / TopAbs / gp_ / Geom_。
RebarModel 内部长度使用 meter。
从 StepSession 消费 unitPolicy，而不是直接解释 STEP 文件单位。
```
