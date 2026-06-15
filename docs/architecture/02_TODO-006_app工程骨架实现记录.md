# TODO-006 app 工程骨架实现记录

## 结论

```text
TODO-006 已完成。

TS_RS 现在有了最小 app 工程骨架：
  CMake configure 可通过。
  build 可通过。
  CTest 可跑通 1 个 smoke test。
```

本轮只建工程地基，不实现 RebarSmart 算法，不接 Qt / OCCT。

## 创建文件

```text
app/CMakeLists.txt
app/src/core/AppBuildInfo.h
app/src/core/AppBuildInfo.cpp
app/src/rebarsmart/.keep
app/tests/CMakeLists.txt
app/tests/smoke_tests.cpp
app/tests/rebarsmart/.keep
```

## 工程结构

```text
app/
├── CMakeLists.txt
├── src/
│   ├── core/
│   │   ├── AppBuildInfo.h
│   │   └── AppBuildInfo.cpp
│   └── rebarsmart/
│       └── .keep
└── tests/
    ├── CMakeLists.txt
    ├── smoke_tests.cpp
    └── rebarsmart/
        └── .keep
```

## 当前能力

```text
tsrs_app_core:
  最小静态库。
  只暴露 appName() smoke 接口。

tsrs_app_smoke_tests:
  验证 tsrs_app_core 可编译、可链接、可由 CTest 执行。
```

说明：

```text
AppBuildInfo 不是业务模块。
它只是让 CMake / CTest 有一个真实 target 可编译。
TODO-007 才开始 SpaceListParser。
```

## 构建验证

普通 PowerShell 中没有自动加载 MSVC 编译器环境。
本机可通过 VS 2026 的 `vcvars64.bat` 注入环境后构建。

实际通过的验证命令：

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

& $cmake -S "$PWD\app" -B "$PWD\build\todo006-final2" -G Ninja "-DCMAKE_MAKE_PROGRAM=$ninja" "-DCMAKE_CXX_COMPILER=$cl"
& $cmake --build "$PWD\build\todo006-final2"
& $cmake -E chdir "$PWD\build\todo006-final2" ctest --output-on-failure
```

验证结果：

```text
configure_exit=0
build_exit=0
ctest_exit=0

1/1 Test #1: tsrs_app_smoke_tests ... Passed
100% tests passed, 0 tests failed out of 1
```

## 护栏检查

已检查：

```text
todo.csv 中本轮开始时只有 TODO-006 是 next。
app/ 中没有 TopoDS / AIS / BRep / TopAbs / QWidget / QObject。
app/ 中没有 RebarSmart DLL 调用。
app/ 中没有 GenerateRebar / FixDistance / FixNumber 算法实现。
build/ 目录被 .gitignore 忽略。
```

## xhigh 只读 review

本轮涉及 CMake / 测试骨架，按项目规则执行了 xhigh 只读 review。

结论：

```text
无 Critical / Important 阻塞。
```

Minor：

```text
1. 当前 smoke test 只验证可编译 / 可链接 / 可运行。
   TODO-007 起必须补真正算法语义测试，不能长期只靠 smoke test。

2. CMake 同时设置全局 CMAKE_CXX_STANDARD 和 target_compile_features。
   当前不影响构建，后续多 target 时可逐步收敛到目标级约束。
```

处理：

```text
两个 Minor 不阻塞 TODO-006。
TODO-007 开始补纯算法行为测试。
```

## 旧实现项目命名修正

后续文档和任务中统一使用下面命名：

```text
总工作目录：
  C:\Users\ghost\Desktop\reverse_engineering\【03】图石软件

旧实现项目：
  C:\Users\ghost\Desktop\reverse_engineering\【03】图石软件\【图石钢筋1比1复刻】

新项目：
  C:\Users\ghost\Desktop\reverse_engineering\【03】图石软件\【TS_RS】
```

旧实现项目只允许作为 Qt / OCCT 工程经验参考：

```text
可参考：
  CMake 找 Qt / OCCT。
  STEP / STP 导入。
  AIS Viewer。
  选择 / 高亮。
  XCAF / TopoDS 遍历。
  剖切 / 颜色 / 视图控制。
  稳定 ID / TopologyBinding 思路。

禁止作为业务真相迁入：
  直接用 OCCT 创建钢筋的 rebar 生成器。
  EdgeToRebarFactory。
  FaceRebarGenerator。
  PolylineRebarGenerator。
  RebarCreationCommandService 中直接造钢筋的业务流程。
```

## 下一步建议

插入一个只读审计节点：

```text
TODO-006A：
  审计旧实现项目【图石钢筋1比1复刻】中哪些 Qt / OCCT 工程零件可复用。

边界：
  只读审计，不搬代码。
  输出 A / B / C 三类清单。
```

完成 TODO-006A 后，再进入 TODO-007 `SpaceListParser`。
