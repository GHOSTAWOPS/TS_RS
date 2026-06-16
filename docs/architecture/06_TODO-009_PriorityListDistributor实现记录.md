# TODO-009 PriorityListDistributor 实现记录

## 结论

```text
TODO-009 已完成。

TS_RS 纯算法层现在具备 RebarSmart-style 间距列表优先分布能力：
  PriorityListDistributor
```

本轮只实现纯算法分布器，不接 Qt / OCCT / Detail，不实现定距 / 定数钢筋生成器，也不提前实现 TODO-010 的 zone calculator。

## 控制合同

Primary Setpoint：

```text
实现 RebarSmart mode==2 间距列表优先分布计算。
```

Acceptance：

```text
1. 先补 PL 测试，再实现。
2. PL-001..PL-010 通过。
3. targeted CTest 通过。
4. 默认 CTest 通过。
5. xhigh 只读 review 的 Critical / Important 均已关闭。
```

Guardrail Metrics：

```text
不接 Qt。
不接 OCCT。
不接 Detail。
不调用 RebarSmart DLL。
不调用 VisualTS EXE / DLL。
不引入 3DE / CAA / CATIA / ACIS / HOOPS / Codejock 运行时依赖。
不实现 GuideCurveZoneCalculator / GuideSurfaceOffsetCalculator。
```

Boundary：

```text
代码只改：
  app/src/rebarsmart/distribute/
  app/tests/rebarsmart/
  app/CMakeLists.txt
  app/tests/CMakeLists.txt
```

Approximation Validity：

```text
PriorityListDistributor 是 current-understanding 版本。

已实现：
  第一个点位 = headZone。
  后续点位 = 前一点位 + spacingList[i]。
  只保留不超过 curveLength - tailZone 的点位。
  count = pointLengthsM.size()。
  margin = curveLength - tailZone - lastPointLength。
  点位数量 <= 1 返回 RS_POINT_COUNT_TOO_SMALL。

未确认：
  GetPointLength_ProiriyList 是否包含首点。
  最后一点等于尾端时的 RebarSmart 精确容差。
  列表超过可用长度时 RebarSmart 是截断还是报错。
```

## 新增文件

```text
app/src/rebarsmart/distribute/PriorityListDistributor.h
app/src/rebarsmart/distribute/PriorityListDistributor.cpp
app/tests/rebarsmart/priority_list_distributor_tests.cpp
```

## 接口

```cpp
DistributionResult distributeBySpacingList(double curveLengthM,
                                           ZoneLengths zone,
                                           std::vector<double> spacingListM);
```

说明：

```text
输入 spacingListM 应来自 SpaceListParser 或等价上游转换。
本函数不解析文本，只处理已转换成米制的间距数组。
```

## 测试覆盖

| Case | 内容 | 期望 |
|---|---|---|
| PL-001 | curve=10, zone=1/1, list=2,2,2,2 | points=1,3,5,7,9; count=5; margin=0 |
| PL-002 | list=3,3 | points=1,4,7; count=3; margin=2 |
| PL-003 | list empty | RS_SPACE_LIST_EMPTY |
| PL-004 | list=20 | RS_POINT_COUNT_TOO_SMALL |
| PL-005 | curve=0 | RS_CURVE_LENGTH_NON_POSITIVE |
| PL-006 | head zone 负数 | RS_ZONE_LENGTH_INVALID |
| PL-007 | list 含 0 | RS_SPACING_NON_POSITIVE |
| PL-008 | list 含 NaN | RS_SPACING_NON_POSITIVE |
| PL-009 | head 已到 tailLimit 且 tiny spacing | RS_POINT_COUNT_TOO_SMALL |
| PL-010 | tailLimit < head | RS_POINT_COUNT_TOO_SMALL |

## RED 记录

先写测试后构建，确认失败：

```text
fatal error C1083:
无法打开包括文件: “rebarsmart/distribute/PriorityListDistributor.h”: No such file or directory
```

xhigh review 后新增边界测试，再次确认失败：

```text
PL-009: expected failure
```

说明：

```text
旧实现会在近尾端 epsilon 场景下重复 push 同一个点。
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

& $cmake -S "$PWD\app" -B "$PWD\build\todo-009" -G Ninja "-DCMAKE_MAKE_PROGRAM=$ninja"
& $cmake --build "$PWD\build\todo-009"
& $cmake -E chdir "$PWD\build\todo-009" ctest -R "tsrs_priority_list_distributor_tests" --output-on-failure
& $cmake -E chdir "$PWD\build\todo-009" ctest --output-on-failure
```

结果：

```text
targeted test:
  1/1 Test #5: tsrs_priority_list_distributor_tests ... Passed

default CTest:
  1/5 Test #1: tsrs_app_smoke_tests .................... Passed
  2/5 Test #2: tsrs_space_list_parser_tests ............ Passed
  3/5 Test #3: tsrs_priority_count_distributor_tests ... Passed
  4/5 Test #4: tsrs_priority_space_distributor_tests ... Passed
  5/5 Test #5: tsrs_priority_list_distributor_tests .... Passed
  100% tests passed, 0 tests failed out of 5
```

## 护栏检查

已执行：

```powershell
git diff --check
rg -n "TopoDS_|AIS_|BRep|TopAbs|gp_|Geom_" .\app\src\rebarsmart .\app\tests\rebarsmart
```

结果：

```text
git diff --check:
  exit 0，仅 CRLF warning。

rebarsmart OCCT leak check:
  no matches。
```

## xhigh 只读 review

第一次 xhigh review：

```text
subagent:
  019ece14-dacf-7fb0-9331-6bdd85428e77

完成后已关闭。
```

结论：

```text
Critical:
  无。

Important:
  PriorityList 近尾端 epsilon 会生成重复点。
```

主流程修复：

```text
1. 增加 PL-009：head 已到 tailLimit 且 tiny spacing，期望 RS_POINT_COUNT_TOO_SMALL。
2. 增加 PL-010：tailLimit < head，期望 RS_POINT_COUNT_TOO_SMALL。
3. 实现中在 push 前检查 candidatePointM 是否严格前进。
4. tailLimit <= head + epsilon 时直接返回 RS_POINT_COUNT_TOO_SMALL。
```

窄复审：

```text
subagent:
  019ece2b-c7ab-7103-a770-597620dfdc32

完成后已关闭。
```

复审结论：

```text
Verdict:
  Approved

Critical:
  无。

Important:
  无。
```

说明：

```text
中间一个复审代理 019ece1f-5ca5-7261-a6ab-ba6591e8bf99 超时卡住，
已关闭后重新开窄复审代理。
```

## 未闭合 GAP

| GAP | 内容 | 当前处理 |
|---|---|---|
| ALG-GAP-003 | GetPointLength_ProiriyList 首点 / 末点包含规则 | 当前按首点=headZone 实现 |
| ALG-GAP-014 | 列表超过可用长度时截断还是报错 | 当前按截断实现 |
| ALG-GAP-015 | PriorityList 真实容差 | 当前使用 1.0e-9 工程容差 |
| ALG-GAP-016 | 直接调用 API 时超大 spacingList 上限 | 上游 SpaceListParser 有 100000 上限；本层暂未复制上限 |

## 对 todo.csv 的影响

本轮完成后：

```text
TODO-009 -> done
TODO-010 -> next
```

TODO-010 进入：

```text
GuideCurveZoneCalculator
```

边界仍然是纯算法层，不接 Qt / OCCT / Detail。
