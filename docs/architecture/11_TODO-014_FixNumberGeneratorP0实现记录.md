# TODO-014 FixNumberCenterlineGenerator P0 实现记录

## 结论

```text
TODO-014 已完成。

TS_RS 现在具备 RebarSmart-style 定数钢筋的 P0 主路径生成器：
  FixNumberCenterlineGenerator P0
```

本轮只实现定数中心线生成主路径，不接 Qt，不接 Detail，不扩展到真实辅助配对、孔洞、弯钩、式样表和实体钢筋对象。

说明：

```text
代码文件仍叫 FixNumberGenerator.*，
公开函数是 generateFixNumberCenterlines(...）。
后续文档和评审统一称为 FixNumberCenterlineGenerator P0，
避免误解为完整 RebarSmart FixNumber 复刻。
```

## 控制合同

Primary Setpoint：

```text
让 RebarSmart-style 定数钢筋可以基于 IGeometryEngine 生成一组可测试的中心线引用。
```

Acceptance：

```text
1. 先补 FixNumberCenterlineGenerator 测试，再实现。
2. 测试覆盖至少两条引导线、根数等分、相邻配对生成中心线。
3. 测试覆盖 guide curve count invalid / guide curve invalid / diameter invalid / count non-positive。
4. targeted CTest 通过。
5. 默认 CTest 通过。
6. xhigh 只读 review 的 Critical / Important 均已关闭。
```

Guardrail Metrics：

```text
不接 Qt。
不接 Detail。
不实现下一阶段生成器。
不调用 RebarSmart DLL。
不调用 VisualTS EXE / DLL。
不引入 3DE / CAA / CATIA / ACIS / HOOPS / Codejock 运行时依赖。
不让 rebarsmart 目录泄漏 OCCT 类型。
不把未闭合的引导线配对 / 排序 / 孔洞 / 弯钩行为写成最终事实。
```

## 新增文件

```text
app/src/rebarsmart/generators/FixNumberGenerator.h
app/src/rebarsmart/generators/FixNumberGenerator.cpp
app/tests/rebarsmart/fix_number_generator_tests.cpp
```

## 修改文件

```text
app/CMakeLists.txt
app/tests/CMakeLists.txt
```

## 实现摘要

```text
FixNumberGeneratorInput：
  parameters
  selection

FixNumberParameters：
  styleName
  grade
  diameterM
  spacingM
  count
  bundleCount
  coverThicknessM
  marginMode
  priorityMode
  headDistanceM
  tailDistanceM

FixNumberSelectionContext：
  mainGuideCurves

generateFixNumberCenterlines：
  1. 读取引导线集合。
  2. 按当前理解将相邻引导线作为配对对象。
  3. 每条引导线按 count 和 head/tail 距离等分。
  4. 连接同序等分点生成两点 polyline 中心线。
```

## P0 当前理解

```text
1. 至少两条引导线是必须条件。
2. 先按输入顺序做相邻配对。
3. 每一对引导线按根数等分，连接同序点。
4. 多于两条引导线时，真实排序 / 网格 / 分组规则仍是 GAP。
```

这条理解仅用于 P0：

```text
1. 让定数生成器先闭合主路径。
2. 让选择对象、根数和几何引擎先串起来。
3. 后续如果要闭合真实 RebarSmart 配对与排序，再另开 TODO。
```

## RED 记录

先补测试后构建，确认失败：

```text
fatal error C1083:
无法打开包括文件:
  rebarsmart/generators/FixNumberGenerator.h
```

## 调试记录

1. 首轮测试修正了 DTO 使用方式，确保不靠错误聚合初始化混过去。
2. 再次失败后，分离了“选择数量不足”和“引用存在但几何缺失”两类诊断。
3. 之后补了坐标断言和多引导线相邻配对断言，避免测试只看数量。

## 验证

targeted 验证：

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

& $cmake -S app -B build\todo-014-green -G Ninja -DCMAKE_MAKE_PROGRAM="$ninja"
& $cmake --build build\todo-014-green --target tsrs_fix_number_generator_tests
& "$PWD\build\todo-014-green\tests\tsrs_fix_number_generator_tests.exe"
```

结果：

```text
targeted_exit=0
```

默认验证：

```powershell
& $cmake -S app -B build\default -G Ninja -DCMAKE_MAKE_PROGRAM="$ninja"
& $cmake --build build\default
Push-Location build\default
try {
    & $cmake -E env CTEST_OUTPUT_ON_FAILURE=1 ctest --timeout 60
} finally {
    Pop-Location
}
```

结果：

```text
11/11 tests passed, 0 tests failed.
```

护栏检查：

```powershell
rg -n "TopoDS_|AIS_|BRep|TopAbs_|gp_|Geom_" app/src/rebarsmart app/tests/rebarsmart app/src/geometry/kernel app/tests/geometry/geometry_engine_contract_tests.cpp
git diff --check
```

结果：

```text
OCCT leak check:
  no matches。

git diff --check:
  exit 0，仅 CRLF warning。
```

## xhigh 只读 review

```text
subagent:
  019ecf8a-18e5-71f1-bccf-806a676d3ca2
```

结论：

```text
Verdict:
  Ready。

Critical:
  无。

Important:
  无。

Required fixes before commit:
  无。
```

Minor：

```text
三引导线用例覆盖了当前相邻配对理解，但测试名 FN-001A 不够自解释。
```

处理：

```text
已把三引导线用例失败信息改为 current-understanding adjacent pairs，
明确这是 P0 当前理解，不是 RebarSmart 多引导线最终配对规则。
```

## 未闭合 GAP

| GAP | 内容 | 当前处理 |
|---|---|---|
| RS-FNG-GAP-001 | 多于两条引导线的真实排序 / 分组 / 网格配对规则 | 已记录，后续闭合 |
| RS-FNG-GAP-002 | 轴线经孔洞处理 / 弯钩 / 辅助延长的真实顺序 | 保持为后续 TODO |
| RS-FNG-GAP-003 | `basic.spacingM` 在定数模式中的真实用途 | 保持为证据待确认项 |
| RS-FNG-GAP-004 | 生成结果还只是中心线引用，不是完整钢筋实体 | 保持为后续 TODO |
| RS-FNG-GAP-005 | 投影 / 偏移 / 相交 / 扫掠未进入 IGeometryEngine P0 | 见 `docs/geometry/01_IGeometryEngine_P1扩展接口.md` |

## 对 todo.csv 的影响

本轮完成后：

```text
TODO-014 -> done
TODO-015 -> next
```
