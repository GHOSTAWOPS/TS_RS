# TODO-007 SpaceListParser 实现记录

## 结论

```text
TODO-007 已完成。

TS_RS 现在具备 RebarSmart-style 不等距间距列表解析能力：
  20*12,25*8 -> 12 个 0.20m + 8 个 0.25m
  20 -> 1 个 0.20m
  20*1 -> 1 个 0.20m
```

本轮只实现纯算法解析器，不接 Qt / OCCT / Detail，不实现定距或定数钢筋生成器。

## 控制合同

Primary Setpoint：

```text
实现 SpaceListParser，使 RebarSmart 间距列表文本可以稳定转成内部米制距离数组。
```

Acceptance：

```text
1. 先补 SL-001..SL-015 测试。
2. RED 构建失败原因来自 SpaceListParser.h 尚不存在。
3. GREEN 后 targeted test 通过。
4. 默认 CTest 通过。
5. 诊断码覆盖空串、非法 token、重复次数 0、重复次数非法、全角逗号不支持、repeat 过大。
6. xhigh review 的 Critical / Important 均已关闭。
```

Guardrail Metrics：

```text
不接 Qt。
不接 OCCT。
不接 Detail。
不调用 RebarSmart DLL。
不调用 VisualTS DLL / EXE。
不引入 3DE / CAA / CATIA / ACIS / HOOPS / Codejock 运行时依赖。
```

Boundary：

```text
代码只改：
  app/src/rebarsmart/space_list/
  app/tests/rebarsmart/
  app/CMakeLists.txt
  app/tests/CMakeLists.txt
```

Approximation Validity：

```text
本轮 parser 只锁定当前证据明确的语法：
  N*M
  N
  半角逗号分隔
  unitScaleToM 由调用方传入

全角逗号默认返回 RS_SPACE_LIST_SEPARATOR_UNSUPPORTED。
allowChineseComma=true 是内部兼容开关，不代表 RebarSmart 已确认支持全角逗号。
RebarSmart 是否真实支持全角逗号、空格、小数等，仍保持 GAP。

实现已按 xhigh review 修正为长度受控的 ASCII parser：
  不使用 strtod / strtol。
  不依赖 C locale。
  不把 string_view 截断成 C 字符串语义。
  repeatCount 和总输出数量上限均为 100000。
```

## 新增文件

```text
app/src/rebarsmart/space_list/SpaceListParser.h
app/src/rebarsmart/space_list/SpaceListParser.cpp
app/tests/rebarsmart/space_list_parser_tests.cpp
```

## 接口

```cpp
struct SpaceListParseOptions {
    double unitScaleToM{1.0};
    bool allowChineseComma{false};
};

struct SpaceListParseResult {
    bool ok{false};
    std::vector<double> valuesM;
    std::string diagnosticCode;
    int errorOffset{-1};
};

SpaceListParseResult parseSpaceList(std::string_view text, SpaceListParseOptions options);
```

设计说明：

```text
SpaceListParser 不决定 UI 单位。
调用方必须传入 unitScaleToM。
例如定距文档中的 20*12,25*8 使用 unitScaleToM=0.01 时，
解析成 0.20m / 0.25m。
```

## 测试覆盖

| Case | 输入 | unitScaleToM | 期望 |
|---|---|---:|---|
| SL-001 | `20*12,25*8` | 0.01 | 20 个值，前 12 个 0.20，后 8 个 0.25 |
| SL-002 | `20` | 0.01 | 1 个值 0.20 |
| SL-003 | `20*1` | 0.01 | 1 个值 0.20 |
| SL-004 | 空字符串 | 0.01 | `RS_SPACE_LIST_EMPTY` |
| SL-005 | `20*0` | 0.01 | `RS_SPACE_LIST_REPEAT_ZERO` |
| SL-006 | `20*-1` | 0.01 | `RS_SPACE_LIST_REPEAT_INVALID` |
| SL-007 | `20*,25` | 0.01 | `RS_SPACE_LIST_TOKEN_INVALID` |
| SL-008 | `20*12，25*8` | 0.01 | `RS_SPACE_LIST_SEPARATOR_UNSUPPORTED` |
| SL-009 | `+20` | 0.01 | `RS_SPACE_LIST_TOKEN_INVALID` |
| SL-010 | `0x10` | 0.01 | `RS_SPACE_LIST_TOKEN_INVALID` |
| SL-011 | `20,` | 0.01 | `RS_SPACE_LIST_TOKEN_INVALID` |
| SL-012 | `20*100001` | 0.01 | `RS_SPACE_LIST_REPEAT_TOO_LARGE` |
| SL-013 | `20*60000,25*60000` | 0.01 | `RS_SPACE_LIST_REPEAT_TOO_LARGE` |
| SL-014 | `20，25` 且 allowChineseComma=true | 0.01 | 2 个值 0.20、0.25 |
| SL-015 | `20\\0,25` | 0.01 | `RS_SPACE_LIST_TOKEN_INVALID` |

## RED 记录

先写测试后构建，确认失败：

```text
fatal error C1083:
无法打开包括文件: “rebarsmart/space_list/SpaceListParser.h”: No such file or directory
```

这说明测试确实在检查尚未实现的新接口。

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

$cmake = 'D:\Visual Studio 2026\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
$ninja = 'D:\Visual Studio 2026\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe'
$cl = 'D:\Visual Studio 2026\Community\VC\Tools\MSVC\14.50.35717\bin\Hostx64\x64\cl.exe'

& $cmake -S "$PWD\app" -B "$PWD\build\todo-007" -G Ninja "-DCMAKE_MAKE_PROGRAM=$ninja" "-DCMAKE_CXX_COMPILER=$cl"
& $cmake --build "$PWD\build\todo-007"
& $cmake -E chdir "$PWD\build\todo-007" ctest -R tsrs_space_list_parser_tests --output-on-failure
& $cmake -E chdir "$PWD\build\todo-007" ctest --output-on-failure
```

结果：

```text
targeted test:
  1/1 Test #2: tsrs_space_list_parser_tests ..... Passed

default CTest:
  1/2 Test #1: tsrs_app_smoke_tests ............. Passed
  2/2 Test #2: tsrs_space_list_parser_tests ..... Passed
  100% tests passed, 0 tests failed out of 2
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
```

结果：

```text
git diff --check:
  仅有 LF/CRLF 提示，无 whitespace error。

domain/rebar leak check:
  domain/rebar not present; leak check not applicable
```

## xhigh 只读 review

本轮涉及代码、测试和 CMake，按项目规则必须在 commit 前执行 xhigh 只读 review。

执行记录：

```text
xhigh subagent:
  019ecb2c-3c90-7d60-b261-3010fecda5e6

完成后已关闭该 subagent。
```

review 结论：

```text
Critical:
  无。

Important:
  1. strtod / strtol 受 C locale 影响，且 string_view -> C 字符串会吞嵌入 NUL。
  2. repeatCount 没有上限，可能导致超大分配。
  3. 尾随分隔符会被静默吞掉。

Minor:
  1. errorOffset 没有精确断言。
  2. allowChineseComma=true 没有正向测试。
```

主流程修复：

```text
1. 改为长度受控 ASCII parser，不再使用 strtod / strtol。
2. 增加 repeatCount / 总输出数量上限 100000。
3. 尾随半角逗号返回 RS_SPACE_LIST_TOKEN_INVALID；全角逗号默认仍返回 RS_SPACE_LIST_SEPARATOR_UNSUPPORTED，仅在 allowChineseComma=true 时按分隔符处理。
4. 精确断言 errorOffset。
5. 增加 SL-009..SL-015。
6. 补充 <utility> include，保证 std::move 声明稳定。
```

## 未闭合 GAP

| GAP | 内容 | 当前处理 |
|---|---|---|
| ALG-GAP-009 | RebarSmart 是否接受全角逗号 | 默认返回 `RS_SPACE_LIST_SEPARATOR_UNSUPPORTED` |
| ALG-GAP-010 | RebarSmart 是否接受空格 / 小数 / 正号 / 负数 / 十六进制写法 | 只实现严格 ASCII 正数；后续用 IDA / 运行确认闭合 |
| ALG-GAP-011 | `ParseSpaceList` 原始函数是否有更复杂分隔符 | 当前不扩大语法，避免猜测成事实 |
| ALG-GAP-012 | repeatCount / 总输出上限的真实 RebarSmart 口径 | 当前工程保护上限为 100000，避免资源耗尽 |

## 对 todo.csv 的影响

本轮完成后：

```text
TODO-007 -> done
TODO-008 -> next
```

TODO-008 进入：

```text
PrioritySpaceDistributor / PriorityCountDistributor
```

边界仍然是纯算法层，不接 Qt / OCCT / Detail。
