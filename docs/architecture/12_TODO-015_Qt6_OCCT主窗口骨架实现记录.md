# TODO-015 Qt6 OCCT 主窗口骨架实现记录

## 结论

```text
TODO-015 已完成。

TS_RS 现在具备可启动的 Qt6 主窗口骨架：
  MainWindow
  中央占位 OccViewerWidget
  左侧工程树 Dock
  右侧当前钢筋生成器 Dock
  底部消息 Dock
  老图石外壳五个顶层页签
```

本轮只做 UI / presentation 空壳，不实现 STEP 导入，不实现钢筋业务，不实现 Detail。

## 控制合同

Primary Setpoint：

```text
让 TS_RS 先具备一个可启动、可定位、可承载后续 UI 命令的 Qt6 主窗口骨架。
```

Acceptance：

```text
1. 先补主窗口骨架测试，再实现。
2. 测试覆盖主窗口标题、老图石五个顶层页签、中央 Viewer、左/右/底 Dock。
3. 测试覆盖 Dock 实际停靠区域。
4. 测试覆盖 MainWindow show smoke 和 tsrs_app --smoke。
5. 默认 CTest 通过。
6. xhigh 只读 review 的 Critical / Important 均已关闭。
```

Guardrail Metrics：

```text
不实现 STEP 导入。
不实现钢筋业务。
不实现 Detail。
不引入 OCCT 业务语义到 ui 层。
不让 QWidget 骨架变成未来钢筋算法依赖。
```

## 新增文件

```text
app/src/presentation/occ/OccViewerWidget.h
app/src/presentation/occ/OccViewerWidget.cpp
app/src/ui/mainwindow/MainWindow.h
app/src/ui/mainwindow/MainWindow.cpp
app/src/app/main.cpp
app/tests/ui/main_window_skeleton_tests.cpp
app/tests/ui/app_startup_smoke_tests.cpp
```

## 修改文件

```text
app/CMakeLists.txt
app/tests/CMakeLists.txt
```

## 实现摘要

```text
MainWindow：
  顶部五页签：开始 / 显示 / 钢筋 / 查询 / 工程图
  左 Dock：工程树
  右 Dock：当前钢筋生成器
  底 Dock：消息
  中央：OccViewerWidget 占位

OccViewerWidget：
  中央灰底占位 widget
  objectName = central_occ_viewer
```

## RED 记录

先补测试后构建，确认失败：

```text
fatal error C1083:
无法打开包括文件:
  ui/mainwindow/MainWindow.h
```

## 调试记录

1. 首轮验证确认 Qt6 已安装，CMake 可找到 Widgets。
2. `findChild<OccViewerWidget*>` 触发 Qt 模板约束后，补了 `Q_OBJECT`。
3. Debug 运行时又暴露出 PATH 未包含 `debug/bin`，补了 CTest 运行环境。
4. xhigh review 指出 Dock 区域、show smoke 和 app 启动 smoke 证据不足。
5. 补了 Dock area 断言、MainWindow show / processEvents 和 tsrs_app --smoke 测试。
6. 二次 xhigh review 指出 Qt platform plugin / runtime 不能无条件偏向 debug 路径。
7. 补了 Debug / 非 Debug 配置分流，避免 Release / RelWithDebInfo 加载 debug Qt plugin。
8. 最终默认 CTest 从 11/11 扩展为 13/13，并保持全绿。

## 验证

default 验证：

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
13/13 tests passed, 0 tests failed.
```

护栏检查：

```powershell
rg -n "TopoDS_|AIS_|BRep|TopAbs_|gp_|Geom_" app/src/rebarsmart app/tests/rebarsmart app/src/geometry/kernel app/tests/geometry/geometry_engine_contract_tests.cpp app/src/ui app/src/presentation
git diff --check
```

结果：

```text
OCCT leak check:
  no matches。

git diff --check:
  exit 0，仅 CRLF warning。
```

## xhigh review 记录

第一次复审：

```text
Verdict: Not Ready
Important:
  Qt 测试运行时配置无条件优先使用 debug Qt plugin / debug bin，
  Release / RelWithDebInfo 下 smoke 证据不成立。
Minor:
  实现记录测试数量不一致。
  新增文件清单漏 app_startup_smoke_tests.cpp。
```

处理：

```text
1. CMake 中区分 TSRS_QT_PLATFORM_PLUGIN_PATH 与 TSRS_QT_DEBUG_PLATFORM_PLUGIN_PATH。
2. CTest 环境使用 $<CONFIG:Debug> generator expression 分流 debug / release runtime。
3. 实现记录修正为 13/13，并补 app_startup_smoke_tests.cpp。
4. review_packages/ 加入 .gitignore，避免临时外部审查包污染提交。
```

## 对 todo.csv 的影响

本轮完成后：

```text
TODO-015 -> done
TODO-016 -> next
```

说明：

```text
TODO-016 已按外部技术评审调整为：
  老图石 Ribbon 外壳与命令映射收口。

不再直接进入 STEP 导入与水泥灰显示。
```
