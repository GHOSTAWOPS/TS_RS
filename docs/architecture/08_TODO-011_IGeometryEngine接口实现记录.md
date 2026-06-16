# TODO-011 IGeometryEngine 接口实现记录

## 结论

```text
TODO-011 已完成。

TS_RS 现在具备生成器可依赖的最小几何抽象接口：
  IGeometryEngine
```

本轮只定义 geometry/kernel 纯接口和契约测试，不接 Qt / OCCT / Detail，不实现 OcctGeometryEngine，也不实现 FixDistance / FixNumber 生成器。

## 控制合同

Primary Setpoint：

```text
冻结 RebarSmart-style 生成器后续可依赖的最小几何接口边界。
```

Acceptance：

```text
1. 先补 IGeometryEngine 契约测试，再实现接口。
2. 契约测试覆盖 curveLength / pointAtLength / tangentAtLength / makePolylineCurve。
3. 失败结果必须 ok=false 且 diagnosticCode 明确。
4. 成功结果必须 ok=true 且 diagnosticCode=GEOM_OK。
5. tangentAtLength 明确返回 unit tangent。
6. makePolylineCurve 返回的 Curve ref 可继续被 curveLength / pointAtLength 消费。
7. targeted CTest 通过。
8. 默认 CTest 通过。
9. xhigh 只读 review 的 Critical / Important 均已关闭。
```

Guardrail Metrics：

```text
不接 Qt。
不接 OCCT。
不接 Detail。
不调用 RebarSmart DLL。
不调用 VisualTS EXE / DLL。
不引入 3DE / CAA / CATIA / ACIS / HOOPS / Codejock 运行时依赖。
不实现 OcctGeometryEngine。
不实现 FixDistanceGenerator / FixNumberGenerator。
不把旧实现项目 rebar 业务代码作为事实来源。
```

## 新增文件

```text
app/src/geometry/kernel/IGeometryEngine.h
app/tests/geometry/geometry_engine_contract_tests.cpp
```

## 接口

```cpp
class IGeometryEngine {
public:
    virtual ~IGeometryEngine() = default;

    virtual GeometryLengthResult curveLength(const GeometryRef& curveRef) const = 0;
    virtual GeometryPointResult pointAtLength(const GeometryRef& curveRef, double lengthM) const = 0;
    virtual GeometryVectorResult tangentAtLength(const GeometryRef& curveRef, double lengthM) const = 0;
    virtual GeometryRefResult makePolylineCurve(std::vector<GeometryPoint3d> points) = 0;
};
```

说明：

```text
tangentAtLength 返回 unit tangent。
GeometryRef.stableId 是上层 TopologyBinding / geometry implementation 的稳定引用字符串。
本接口不暴露 TopoDS / AIS / gp / Geom / QWidget 等实现细节。
```

## 诊断码

```text
GEOM_OK
GEOM_MISSING_REF
GEOM_WRONG_ENTITY_KIND
GEOM_LENGTH_OUT_OF_RANGE
GEOM_INVALID_INPUT
```

## 测试覆盖

```text
GE-001: 通过 IGeometryEngine 指针调用 curveLength，返回 10.0。
GE-002: pointAtLength(4.5) 返回 (4.5, 0, 0)。
GE-003: tangentAtLength(4.5) 返回 unit tangent (1, 0, 0)。
GE-004: makePolylineCurve 返回 Curve ref。
GE-005: polyline ref 可继续 curveLength，长度为 2.0。
GE-006: polyline ref 可继续 pointAtLength(1.5)，返回 (1, 0.5, 0)。
GE-007: wrong entity kind 返回 GEOM_WRONG_ENTITY_KIND 且 ok=false。
GE-008: missing ref 返回 GEOM_MISSING_REF 且 ok=false。
GE-009: length out of range 返回 GEOM_LENGTH_OUT_OF_RANGE 且 ok=false。
GE-010: invalid polyline 返回 GEOM_INVALID_INPUT 且 ok=false。
```

## RED 记录

先写测试后构建，确认失败：

```text
fatal error C1083:
无法打开包括文件:
  geometry/kernel/IGeometryEngine.h
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

& $cmake -S app -B build\todo-011 -G Ninja -DCMAKE_MAKE_PROGRAM="$ninja"
& $cmake --build build\todo-011
& $cmake --build build\todo-011 --target test

& $cmake -S app -B build\default -G Ninja -DCMAKE_MAKE_PROGRAM="$ninja"
& $cmake --build build\default
& $cmake --build build\default --target test
```

结果：

```text
build/todo-011 CTest:
  8/8 tests passed, 0 tests failed.

build/default CTest:
  8/8 tests passed, 0 tests failed.
```

## 护栏检查

已执行：

```powershell
git diff --check
rg -n "TopoDS_|AIS_|BRep|TopAbs|gp_|Geom_" app/src/rebarsmart app/tests/rebarsmart app/src/geometry/kernel app/tests/geometry
```

结果：

```text
git diff --check:
  exit 0，仅 CRLF warning。

geometry/rebarsmart OCCT leak check:
  no matches。
```

## xhigh 只读 review

代码 review：

```text
subagent:
  019ece57-b50d-79f3-b894-076fd96c443f

完成后已关闭。
```

结论：

```text
Critical:
  无。

Important:
  1. 失败 helper 没检查 ok=false。
  2. makePolylineCurve 返回 ref 没验证能继续被曲线接口消费。
  3. tangentAtLength 未明确 unit tangent。
```

主流程修复：

```text
1. expectDiagnostic 改为模板 helper，检查 !result.ok 和诊断码。
2. 成功 helper 补 GEOM_OK 断言。
3. MockGeometryEngine 对 polyline ref 支持 curveLength / pointAtLength。
4. 测试断言 polyline length=2.0，pointAtLength(1.5)=(1,0.5,0)。
5. tangentAtLength 增加 unit tangent 注释。
6. expectVector 断言向量模长约等于 1.0。
7. 测试文件显式 include <string_view>。
```

窄复审：

```text
subagent:
  019ece5e-2834-7d80-a144-d02f092be57e

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
| GEOM-GAP-001 | GeometryRef.stableId 的最终格式 | 留到 TopologyBinding / OcctGeometryEngine P0 决定 |
| GEOM-GAP-002 | curved edge 的 pointAtLength / tangentAtLength 容差 | 留到 TODO-012 用 OCCT 样本验证 |
| GEOM-GAP-003 | projectWire / offsetWire / sweepCircle 是否进接口 | P0 不纳入，避免接口过宽 |
| GEOM-GAP-004 | makePolylineCurve 返回的是临时 ref 还是持久 ref | 当前只冻结“可继续被接口消费”的契约 |

## 对 todo.csv 的影响

本轮完成后：

```text
TODO-011 -> done
TODO-012 -> next
```
