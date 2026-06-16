# TODO-012 OcctGeometryEngine P0 实现记录

## 结论

```text
TODO-012 已完成。

TS_RS 现在具备 IGeometryEngine 的 OCCT P0 后端：
  OcctGeometryEngine
```

本轮只实现已冻结的几何底座能力，不接 Qt / Detail，不实现 FixDistanceGenerator / FixNumberGenerator，不提前写 RebarSmart 钢筋生成业务规则。

## 控制合同

Primary Setpoint：

```text
让 RebarSmart-style 生成器后续可以通过 IGeometryEngine 使用 OCCT 的曲线长度、按弧长取点、单位切线和 polyline 组装能力。
```

Acceptance：

```text
1. 先补 OcctGeometryEngine 测试，再实现。
2. 测试覆盖 registerEdge / curveLength / pointAtLength / tangentAtLength / makePolylineCurve。
3. 直线 edge 长度为 10.0。
4. pointAtLength(4.5) 返回 (4.5, 0, 0)。
5. tangentAtLength(4.5) 返回 unit tangent (1, 0, 0)。
6. polyline (0,0,0)->(1,0,0)->(1,1,0) 长度为 2.0。
7. polyline pointAtLength(1.5) 返回 (1, 0.5, 0)。
8. wrong kind / missing ref / out-of-range / invalid polyline 均返回稳定 diagnostic。
9. targeted CTest 通过。
10. 默认 CTest 通过。
11. OCCT 细节不泄漏到 rebarsmart 或 geometry/kernel。
```

Guardrail Metrics：

```text
不接 Qt。
不接 Detail。
不实现钢筋生成器。
不调用 RebarSmart DLL。
不调用 VisualTS EXE / DLL。
不依赖 3DE / CAA / CATIA / ACIS / HOOPS / Codejock 运行时。
不修改 IGeometryEngine 公共接口。
不让 rebarsmart / geometry/kernel 引入 TopoDS / AIS / BRep / TopAbs / gp / Geom。
```

## 新增文件

```text
app/src/geometry/occt/OcctGeometryEngine.h
app/src/geometry/occt/OcctGeometryEngine.cpp
app/tests/geometry/occt_geometry_engine_tests.cpp
```

## 修改文件

```text
app/CMakeLists.txt
app/tests/CMakeLists.txt
```

## 实现摘要

```text
tsrs_geometry_occt：
  单独的 OCCT 静态库 target。
  链接 tsrs_app_core 和 OCCT 几何相关库。
  避免把 OCCT 依赖挂到 tsrs_app_core。

OcctGeometryEngine：
  registerEdge(stableId, TopoDS_Edge)
  curveLength(GeometryRef)
  pointAtLength(GeometryRef, lengthM)
  tangentAtLength(GeometryRef, lengthM)
  makePolylineCurve(points)
```

内部策略：

```text
1. stableId -> CurveRecord。
2. CurveRecord 由一个或多个 TopoDS_Edge segment 组成。
3. registerEdge 注册单段 OCCT edge。
4. makePolylineCurve 将相邻点对组装成多段 line edge。
5. curveLength 返回所有 segment 的总长度。
6. pointAtLength / tangentAtLength 先定位 segment，再用 BRepAdaptor_Curve + GCPnts_AbscissaPoint 按弧长反查参数。
```

说明：

```text
当前 P0 的 polyline 是多段 edge 记录，不是持久 TopoDS_Wire。
这是为 FixDistance / FixNumber P0 提供中心线采样能力的最小实现。
后续如果需要真实 wire/topology mutation，应另开 TODO，不在本轮偷偷扩大范围。
```

## RED 记录

先补测试和 CMake 测试目标后构建，确认失败：

```text
fatal error C1083:
无法打开包括文件:
  geometry/occt/OcctGeometryEngine.h
```

## 调试记录

第一次只构建 `tsrs_occt_geometry_engine_tests` 后直接跑全量 test target，导致其他测试可执行文件未构建而 Not Run。

根因：

```text
测试执行方式错误，不是代码失败。
```

随后直接运行 OCCT 测试 exe，出现：

```text
exit=-1073741515
```

根因：

```text
Windows 运行时找不到 vcpkg / OCCT 相关 DLL。
```

处理：

```text
1. 在本机 vcpkg fallback 中设置 TSRS_VCPKG_INSTALLED_DIR。
2. 给 tsrs_occt_geometry_engine_tests 设置 CTest PATH 环境。
3. 给 tsrs_occt_geometry_engine_tests 增加 post-build runtime bin 复制。
4. 复制目标只在 build/tests 输出目录，不污染源码树，不修改系统 PATH。
5. JXR 的 FindJXR.cmake 通过本机 vcpkg fallback 加入 CMAKE_MODULE_PATH。
```

后续用户侧看到的系统弹窗：

```text
由于找不到 TKGeomBase.dll，无法继续执行代码。
```

补充根因：

```text
直接运行 tests/tsrs_occt_geometry_engine_tests.exe 时，
Windows 不会自动使用 CTest 的 PATH 环境。
因此需要在构建输出目录放置 vcpkg runtime DLL。
```

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

& $cmake -S app -B build\todo-012-targeted -G Ninja -DCMAKE_MAKE_PROGRAM="$ninja"
& $cmake --build build\todo-012-targeted --target tsrs_occt_geometry_engine_tests
Push-Location build\todo-012-targeted
try {
    & $cmake -E env CTEST_OUTPUT_ON_FAILURE=1 ctest -R tsrs_occt_geometry_engine_tests --timeout 60
} finally {
    Pop-Location
}
```

结果：

```text
1/1 Test #9: tsrs_occt_geometry_engine_tests ... Passed
100% tests passed, 0 tests failed out of 1
```

直接运行验证：

```powershell
& $cmake -S app -B build\todo-012-runtime2 -G Ninja -DCMAKE_MAKE_PROGRAM="$ninja"
& $cmake --build build\todo-012-runtime2 --target tsrs_occt_geometry_engine_tests
& "$PWD\build\todo-012-runtime2\tests\tsrs_occt_geometry_engine_tests.exe"
```

结果：

```text
direct_exit=0
```

说明：

```text
直接运行验证用于覆盖用户看到的系统弹窗问题。
CTest 通过只能证明测试环境 PATH 正确；
direct_exit=0 才能证明构建输出目录已带齐运行时 DLL。
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
9/9 tests passed, 0 tests failed.
```

护栏检查：

```powershell
rg -n "TopoDS_|AIS_|BRep|TopAbs|gp_|Geom_" app/src/rebarsmart app/tests/rebarsmart app/src/geometry/kernel app/tests/geometry/geometry_engine_contract_tests.cpp
git diff --check
```

结果：

```text
OCCT leak check:
  no matches。

git diff --check:
  exit 0，仅 CRLF warning。
```

## 能力边界

已具备：

```text
1. 手工注册 TopoDS_Edge 为 Curve ref。
2. 根据 Curve ref 计算总长度。
3. 根据 Curve ref 和弧长取三维点。
4. 根据 Curve ref 和弧长取单位切线。
5. 用点列创建可被 IGeometryEngine 消费的 polyline Curve ref。
```

未具备：

```text
1. STEP shape 自动导入和 TopologyBinding。
2. 用户选择 edge / face 后自动绑定到 GeometryRef。
3. projectWire / offsetWire / sweepCircle。
4. 真实 TopoDS_Wire 持久化。
5. 曲线容差和复杂 curved edge 的真实工程 golden。
6. FixDistance / FixNumber 钢筋生成逻辑。
```

## GAP

| GAP | 内容 | 当前处理 |
|---|---|---|
| GEOM-GAP-001 | stableId 最终格式仍未与 TopologyBinding 对齐 | TODO-016/017 前后闭合 |
| GEOM-GAP-002 | P0 polyline 是多段 edge registry，不是真实 wire | 当前可满足中心线采样；后续 wire 能力另开任务 |
| GEOM-GAP-003 | 曲线容差只覆盖直线和折线测试 | 后续接 STEP 样本和 curved edge 测试 |
| GEOM-GAP-004 | OCCT CMake 依赖 fallback 使用本机 vcpkg 路径 | 后续补正式依赖配置文档和可移植 toolchain 入口 |
| GEOM-GAP-005 | post-build 复制整个 vcpkg runtime bin 只适合作为本机开发便利 | 后续发布包需要精确 runtime 收敛策略 |

## 对 todo.csv 的影响

本轮完成后：

```text
TODO-012 -> done
TODO-013 -> next
```
