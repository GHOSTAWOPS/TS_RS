# TODO-008 PriorityCount / PrioritySpace 实现记录

## 结论

```text
TODO-008 已完成。

TS_RS 纯算法层现在具备 RebarSmart-style 根数优先和间距优先分布计算能力：
  PriorityCountDistributor
  PrioritySpaceDistributor
```

本轮只实现纯算法分布器，不接 Qt / OCCT / Detail，不实现定距 / 定数钢筋生成器，也不提前实现 TODO-009 的间距列表分布。

## 控制合同

Primary Setpoint：

```text
实现 RebarSmart 定距 / 定数前置所需的根数优先与间距优先分布计算。
```

Acceptance：

```text
1. 先补 PC / PS 测试，再实现。
2. PC-001..PC-009 通过。
3. PS-001..PS-010 通过。
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
不实现 PriorityListDistributor。
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
PrioritySpaceDistributor 是 current-understanding 版本。

已实现：
  usable = max(0, curveLength - headZone - tailZone)
  sectionCount = floor(usable / spacing)
  count = sectionCount + 1
  margin = usable - sectionCount * spacing

工程保护：
  近整数浮点误差用 epsilon 修正。
  sectionCount 超过工程上限时返回 RS_SECTION_COUNT_TOO_LARGE。

未确认：
  ratioSpace 的真实用途仍是 ALG-GAP-004。
  CalPointCountOnGuideCurve / CalSectionCountOnGuideCurve 的完整 RebarSmart 公式仍需后续 IDA / 运行确认。
```

## 新增文件

```text
app/src/rebarsmart/distribute/DistributionTypes.h
app/src/rebarsmart/distribute/PriorityCountDistributor.h
app/src/rebarsmart/distribute/PriorityCountDistributor.cpp
app/src/rebarsmart/distribute/PrioritySpaceDistributor.h
app/src/rebarsmart/distribute/PrioritySpaceDistributor.cpp
app/tests/rebarsmart/priority_count_distributor_tests.cpp
app/tests/rebarsmart/priority_space_distributor_tests.cpp
```

## 接口

```cpp
struct ZoneLengths {
    double headM{0.0};
    double tailM{0.0};
};

struct DistributionResult {
    bool ok{false};
    int count{0};
    double spacingM{0.0};
    double marginM{0.0};
    std::vector<double> pointLengthsM;
    std::string diagnosticCode;
    int errorOffset{-1};
};

DistributionResult distributeByCount(double curveLengthM, ZoneLengths zone, int count);

DistributionResult distributeBySpacing(double curveLengthM,
                                      ZoneLengths zone,
                                      double spacingM,
                                      double spacingRatio);
```

说明：

```text
TODO-008 不输出 pointLengthsM。
点位列表留给 TODO-009 PriorityListDistributor 和后续生成器。
```

## 测试覆盖

PriorityCount：

| Case | 内容 | 期望 |
|---|---|---|
| PC-001 | curve=10, zone=1/1, count=5 | spacing=2, margin=0 |
| PC-002 | count=1 | spacing=16, margin=0 |
| PC-003 | head+tail 超过 curve | usable clamp 0, spacing=0 |
| PC-004 | curve=0 | RS_CURVE_LENGTH_NON_POSITIVE |
| PC-005 | count=0 | RS_COUNT_NON_POSITIVE |
| PC-006 | head zone 负数 | RS_ZONE_LENGTH_INVALID |
| PC-007 | head zone NaN | RS_ZONE_LENGTH_INVALID |
| PC-008 | tail zone 负数 | RS_ZONE_LENGTH_INVALID |
| PC-009 | curve inf | RS_CURVE_LENGTH_NON_POSITIVE |

PrioritySpace：

| Case | 内容 | 期望 |
|---|---|---|
| PS-001 | curve=10, zone=1/1, spacing=2 | count=5, margin=0 |
| PS-002 | spacing=3 | count=3, margin=2 |
| PS-003 | head+tail 超过 curve | count=1, margin=0 |
| PS-004 | spacing=0 | RS_SPACING_NON_POSITIVE |
| PS-005 | 0.6 / 0.2 近整数浮点边界 | count=4, margin=0 |
| PS-006 | head zone 负数 | RS_ZONE_LENGTH_INVALID |
| PS-007 | head zone inf | RS_ZONE_LENGTH_INVALID |
| PS-008 | tail zone 负数 | RS_ZONE_LENGTH_INVALID |
| PS-009 | spacing NaN | RS_SPACING_NON_POSITIVE |
| PS-010 | sectionCount 过大 | RS_SECTION_COUNT_TOO_LARGE |

## RED 记录

先写测试后构建，确认失败：

```text
PC-006: expected failure
PS-006: expected failure
```

说明：

```text
旧实现没有校验 ZoneLengths，测试确实抓到了待修行为。
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

& $cmake -S "$PWD\app" -B "$PWD\build\todo-008-green-004" -G Ninja "-DCMAKE_MAKE_PROGRAM=$ninja"
& $cmake --build "$PWD\build\todo-008-green-004"
& $cmake -E chdir "$PWD\build\todo-008-green-004" ctest -R "tsrs_priority_(count|space)_distributor_tests" --output-on-failure
& $cmake -E chdir "$PWD\build\todo-008-green-004" ctest --output-on-failure
```

结果：

```text
targeted test:
  1/2 Test #3: tsrs_priority_count_distributor_tests ... Passed
  2/2 Test #4: tsrs_priority_space_distributor_tests ... Passed

default CTest:
  1/4 Test #1: tsrs_app_smoke_tests .................... Passed
  2/4 Test #2: tsrs_space_list_parser_tests ............ Passed
  3/4 Test #3: tsrs_priority_count_distributor_tests ... Passed
  4/4 Test #4: tsrs_priority_space_distributor_tests ... Passed
  100% tests passed, 0 tests failed out of 4
```

## 工具链记录

```text
VS bundled CMake:
  D:\Visual Studio 2026\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe
  version 4.2.3-msvc3
  本环境 configure 阶段出现 -1073740791 崩溃。

vcpkg CMake:
  D:\Work\vcpkg\downloads\tools\cmake-4.3.2-windows\cmake-4.3.2-windows-x86_64\bin\cmake.exe
  version 4.3.2
  configure / build / ctest 正常。
```

结论：

```text
TODO-008 验证以 vcpkg CMake 4.3.2 为准。
VS bundled CMake 4.2.3 崩溃记录为环境问题，不作为代码失败。
```

## 护栏检查

已执行：

```powershell
git diff --check

if (Test-Path .\app\src\domain\rebar) {
    rg -n "TopoDS_|AIS_|BRep|TopAbs|gp_|Geom_" .\app\src\domain\rebar
} else {
    Write-Output 'domain/rebar not present; leak check not applicable'
}

rg -n "TopoDS_|AIS_|BRep|TopAbs|gp_|Geom_" .\app\src\rebarsmart .\app\tests\rebarsmart
```

结果：

```text
git diff --check:
  exit 0，仅 CRLF warning。

domain/rebar leak check:
  domain/rebar not present; leak check not applicable。

rebarsmart OCCT leak check:
  no matches。
```

## xhigh 只读 review

第一次 xhigh review：

```text
subagent:
  019ecdfd-38aa-7313-bc9c-c5c128f0bf6d

完成后已关闭。
```

结论：

```text
Critical:
  无。

Important:
  1. PrioritySpace 的 0.6 / 0.2 浮点整除会被 floor 少算一段。
  2. ZoneLengths 未校验有限且非负。
  3. floor 转 int 前未限制 sectionCount 范围。
  4. 测试缺少上述边界。
```

主流程修复：

```text
1. 增加 RS_ZONE_LENGTH_INVALID。
2. 增加 RS_SECTION_COUNT_TOO_LARGE。
3. ZoneLengths 要求 head/tail 均有限且非负。
4. PrioritySpace 近整数 rawSectionCount 用 round + epsilon 修正。
5. sectionCount 转 int 前做上限保护。
6. 补充 PC-005..PC-009、PS-005..PS-010。
```

复审：

```text
subagent:
  019ece07-ecb1-7fa0-8a26-bc72b90da3b6

完成后已关闭。
```

复审结论：

```text
Verdict:
  Approved with Minor

Critical:
  无。

Important:
  无。

Minor:
  测试只覆盖 invalid headM，建议补 invalid tailM。
```

主流程已补：

```text
PC-008: tail zone 负数。
PS-008: tail zone 负数。
```

## 未闭合 GAP

| GAP | 内容 | 当前处理 |
|---|---|---|
| ALG-GAP-001 | CalSectionCountOnGuideCurve 完整公式 | TODO-008 只实现 current-understanding 版本 |
| ALG-GAP-002 | CalPointCountOnGuideCurve 是否总是 section+1 | 当前按 section+1 测试锁定 |
| ALG-GAP-004 | ratioSpace 真实用途 | 接口保留参数，但不参与计算 |
| ALG-GAP-013 | sectionCount 工程上限是否符合 RebarSmart | 当前仅作为 TS_RS 防溢出护栏 |

## 对 todo.csv 的影响

本轮完成后：

```text
TODO-008 -> done
TODO-009 -> next
```

TODO-009 进入：

```text
PriorityListDistributor
```

边界仍然是纯算法层，不接 Qt / OCCT / Detail。
