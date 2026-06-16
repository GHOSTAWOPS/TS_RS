# TODO-010 GuideSurfaceOffset / GuideCurveZone 实现记录

## 结论

```text
TODO-010 已完成。

TS_RS 纯算法层现在具备 RebarSmart-style 布筋面偏移距离和引导线端距调整能力：
  GuideSurfaceOffsetCalculator
  GuideCurveZoneCalculator
```

本轮只实现纯算法 zone calculator，不接 Qt / OCCT / Detail，不实现 IGeometryEngine，也不提前实现定距 / 定数钢筋生成器。

## 控制合同

Primary Setpoint：

```text
实现 RebarSmart 定距 / 定数生成前置所需的 guide surface offset 与 guide curve zone 计算。
```

Acceptance：

```text
1. 先补 GO / GZ 测试，再实现。
2. GO-001..GO-012 通过。
3. GZ-001..GZ-011 通过。
4. targeted CTest 通过。
5. 默认 CTest 通过。
6. xhigh 只读 review 的 Critical / Important 均已关闭。
```

Guardrail Metrics：

```text
不接 Qt。
不接 OCCT。
不接 Detail。
不调用 RebarSmart DLL。
不调用 VisualTS EXE / DLL。
不引入 3DE / CAA / CATIA / ACIS / HOOPS / Codejock 运行时依赖。
不实现 IGeometryEngine。
不实现 FixDistanceGenerator / FixNumberGenerator。
```

Boundary：

```text
代码只改：
  app/src/rebarsmart/zone/
  app/tests/rebarsmart/
  app/CMakeLists.txt
  app/tests/CMakeLists.txt

文档只改：
  docs/architecture/
  docs/roadmap/
  todo.csv
```

Approximation Validity：

```text
GuideSurfaceOffsetCalculator / GuideCurveZoneCalculator 是 current-understanding 版本。

已实现：
  GetGuideSurfaceOffestDistance 的 mode 0..5 公式。
  UpdateGuideCurveZone 的 mode 0..4 公式。
  activeLayerIndex * layerSpacing 层偏移。
  unknown mode 返回诊断码，不静默当正常 0 处理。
  NaN / infinity / 负输入的防御性诊断。

未确认：
  unknown offsetMode 在 RebarSmart 最终行为中是否应继续返回 0、保持当前值或报错。
  guide surface offset 为负时 RebarSmart 是否允许；当前按公式直出，仅要求结果有限。
  spacing / layerSpacing / firstLayerProtectThicknessNet 等字段的 UI 单位换算仍由上游负责。
```

## 新增文件

```text
app/src/rebarsmart/zone/GuideSurfaceOffsetCalculator.h
app/src/rebarsmart/zone/GuideSurfaceOffsetCalculator.cpp
app/src/rebarsmart/zone/GuideCurveZoneCalculator.h
app/src/rebarsmart/zone/GuideCurveZoneCalculator.cpp
app/tests/rebarsmart/guide_surface_offset_calculator_tests.cpp
app/tests/rebarsmart/guide_curve_zone_calculator_tests.cpp
```

## 接口

```cpp
struct GuideSurfaceOffsetInput {
    int offsetMode;
    int activeLayerIndex;
    double diameterM;
    double coverThicknessM;
    double layerSpacingM;
    double firstLayerProtectThicknessNetM;
};

struct GuideSurfaceOffsetResult {
    bool ok;
    double offsetM;
    std::string diagnosticCode;
};

GuideSurfaceOffsetResult calculateGuideSurfaceOffset(GuideSurfaceOffsetInput input);

enum class ZoneAdjustMode {
    KeepCurrent = 0,
    Zero = 1,
    HalfDiameter = 2,
    HalfDiameterPlusSpacing = 3,
    GuideSurfaceOffset = 4,
};

struct GuideCurveZoneInput {
    ZoneLengths current;
    ZoneAdjustMode headMode;
    ZoneAdjustMode tailMode;
    double diameterM;
    double spacingM;
    double guideSurfaceOffsetM;
};

struct GuideCurveZoneResult {
    bool ok;
    ZoneLengths zone;
    std::string diagnosticCode;
};

GuideCurveZoneResult calculateGuideCurveZone(GuideCurveZoneInput input);
```

说明：

```text
本轮没有使用计划草案中的裸 double / ZoneLengths 返回值。
根据 xhigh 预审建议，实际接口返回 ok + diagnosticCode，
避免 unknown mode / invalid input 和真实 0 值混淆。
```

## 测试覆盖

GuideSurfaceOffset：

| Case | 内容 | 期望 |
|---|---|---|
| GO-001 | mode 0 | cover + 0.5D = 0.066 |
| GO-002 | mode 1 | cover - 0.5D = 0.034 |
| GO-003 | mode 2 + layerIndex 1 | 0.266 |
| GO-004 | mode 3 | firstNet + 0.5D = 0.096 |
| GO-004A | mode 4 + layerIndex 1 | 0.296 |
| GO-005 | mode 5 + layerIndex 2 | cover + 2 * layerSpacing = 0.45 |
| GO-006 | unknown mode 99 | RS_GUIDE_SURFACE_OFFSET_MODE_UNKNOWN |
| GO-007 | diameter 负数 | RS_GUIDE_SURFACE_OFFSET_INVALID |
| GO-008 | cover NaN | RS_GUIDE_SURFACE_OFFSET_INVALID |
| GO-009 | layerSpacing 负数 | RS_GUIDE_SURFACE_OFFSET_INVALID |
| GO-010 | activeLayerIndex 负数 | RS_GUIDE_SURFACE_OFFSET_INVALID |
| GO-011 | firstLayerProtectThicknessNet 负数 | RS_GUIDE_SURFACE_OFFSET_INVALID |
| GO-012 | mode 1 公式得到负偏移 | ok, offset=-0.006 |

GuideCurveZone：

| Case | 内容 | 期望 |
|---|---|---|
| GZ-001 | mode 0 / 0 | keep current 1.0 / 2.0 |
| GZ-002 | mode 1 / 1 | 0 / 0 |
| GZ-003 | mode 2 / 2 | 0.016 / 0.016 |
| GZ-004 | mode 3 / 3 | 0.216 / 0.216 |
| GZ-005 | mode 4 / 4 | 0.066 / 0.066 |
| GZ-006 | mode 2 / 4 | 0.016 / 0.066 |
| GZ-007 | current head 负数 | RS_GUIDE_CURVE_ZONE_INVALID |
| GZ-008 | diameter 负数 | RS_GUIDE_CURVE_ZONE_INVALID |
| GZ-009 | spacing NaN | RS_GUIDE_CURVE_ZONE_INVALID |
| GZ-010 | guideSurfaceOffset 负数 | RS_GUIDE_CURVE_ZONE_INVALID |
| GZ-011 | unknown zone mode 99 | RS_GUIDE_CURVE_ZONE_MODE_UNKNOWN |

## RED 记录

先写测试后构建，确认失败：

```text
fatal error C1083:
无法打开包括文件:
  rebarsmart/zone/GuideSurfaceOffsetCalculator.h
  rebarsmart/zone/GuideCurveZoneCalculator.h
```

说明：

```text
测试已接入 CMake，失败原因是生产头文件不存在，符合 TDD RED 预期。
```

## GREEN 验证

实际通过命令：

```powershell
$vcvars = 'D:\Visual Studio 2026\Community\VC\Auxiliary\Build\vcvars64.bat'
$envLines = cmd.exe /d /c "call `"$vcvars`" >nul && set"
foreach ($line in $envLines) {
    if ($line -match '^([^=]+)=(.*)$') {
        [Environment]::SetEnvironmentVariable($matches[1], $matches[2], 'Process')
    }
}

$cmake = 'D:\Work\vcpkg\downloads\tools\cmake-4.3.2-windows\cmake-4.3.2-windows-x86_64\bin\cmake.exe'
$ninja = 'D:\Visual Studio 2026\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe'

& $cmake -S app -B build\todo-010 -G Ninja -DCMAKE_MAKE_PROGRAM="$ninja"
& $cmake --build build\todo-010
& $cmake --build build\todo-010 --target test

& $cmake -S app -B build\default -G Ninja -DCMAKE_MAKE_PROGRAM="$ninja"
& $cmake --build build\default
& $cmake --build build\default --target test
```

结果：

```text
build/todo-010 CTest:
  1/7 tsrs_app_smoke_tests ......................... Passed
  2/7 tsrs_space_list_parser_tests ................. Passed
  3/7 tsrs_priority_count_distributor_tests ........ Passed
  4/7 tsrs_priority_space_distributor_tests ........ Passed
  5/7 tsrs_priority_list_distributor_tests ......... Passed
  6/7 tsrs_guide_surface_offset_calculator_tests ... Passed
  7/7 tsrs_guide_curve_zone_calculator_tests ....... Passed
  100% tests passed, 0 tests failed out of 7

build/default CTest:
  1/7 tsrs_app_smoke_tests ......................... Passed
  2/7 tsrs_space_list_parser_tests ................. Passed
  3/7 tsrs_priority_count_distributor_tests ........ Passed
  4/7 tsrs_priority_space_distributor_tests ........ Passed
  5/7 tsrs_priority_list_distributor_tests ......... Passed
  6/7 tsrs_guide_surface_offset_calculator_tests ... Passed
  7/7 tsrs_guide_curve_zone_calculator_tests ....... Passed
  100% tests passed, 0 tests failed out of 7
```

## 护栏检查

已执行：

```powershell
git diff --check
rg -n "TopoDS_|AIS_|BRep|TopAbs|gp_|Geom_" app/src/rebarsmart app/tests/rebarsmart
```

结果：

```text
git diff --check:
  exit 0，仅 CRLF warning。

rebarsmart OCCT leak check:
  no matches。
```

## xhigh 只读 review

开发前 xhigh review：

```text
subagent:
  019ece32-29cb-7e80-9b11-0e5549d0e4f8

完成后已关闭。
```

结论：

```text
Critical:
  无。

Important:
  1. offset 不应裸返回 double，应带 ok / diagnosticCode。
  2. 需要补非法输入测试。
  3. 需要补 unknown mode 测试。
```

主流程处理：

```text
1. GuideSurfaceOffsetResult / GuideCurveZoneResult 均带 ok + diagnosticCode。
2. 补 GO-007..GO-012。
3. 补 GZ-007..GZ-011。
```

代码 review：

```text
subagent:
  019ece3c-481d-7af0-a15a-3e3adb5179cd

完成后已关闭。
```

结论：

```text
Critical:
  无。

Important:
  1. mode 1 的 cover - 0.5D 可能得到负偏移，不能无证据判非法。
  2. offsetMode 4 缺少单独测试。
```

主流程修复：

```text
1. 删除 offsetM < 0.0 失败条件，只要求结果有限。
2. 新增 GO-004A 覆盖 offsetMode 4。
3. 新增 GO-012 锁定 mode 1 负偏移公式直出。
```

窄复审：

```text
subagent:
  019ece48-a411-7cc2-a561-2ed9c60a0a55

完成后已关闭。
```

复审结论：

```text
Ready to commit:
  Yes

Critical:
  无。

Important:
  无。

Minor:
  无。
```

## 未闭合 GAP

| GAP | 内容 | 当前处理 |
|---|---|---|
| ALG-GAP-005 | 辅助布筋区间 Ratio=1.0 单位 | TODO-010 不做 UI Ratio 换算，只接收内部米制值 |
| ALG-GAP-017 | unknown offsetMode 的最终 RebarSmart 行为 | 当前返回 RS_GUIDE_SURFACE_OFFSET_MODE_UNKNOWN，fallback offset=0 |
| ALG-GAP-018 | mode 1 负偏移是否允许 | 当前按公式直出，仅要求结果有限 |
| ALG-GAP-019 | FormGuideCurveZoneArray 的引导线反向 / 孔洞分割 / 端点保护修正 | 留到 FixDistanceGenerator / IGeometryEngine 后续阶段 |

## 对 todo.csv 的影响

本轮完成后：

```text
TODO-010 -> done
TODO-011 -> next
```

TODO-011 进入：

```text
IGeometryEngine 接口设计
```

边界仍然是：

```text
纯算法 / 生成器不直接依赖 OCCT。
TODO-011 只设计生成器可依赖的几何抽象接口和 mock。
```
