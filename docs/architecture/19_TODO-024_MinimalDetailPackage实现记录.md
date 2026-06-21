# TODO-024 Minimal Detail Package 实现记录

## 结论

TODO-024 已从 blocked 更新为 done。

done 的精确定义是：

```text
minimal sheet DetailNN.stl protocol reached manual autoin pass
```

也就是：

```text
TS_RS 当前 P0 minimal sheet baseline = v2_empty_groups。
v2_empty_groups 已通过旧图石 AutoCAD 插件导入按钮路径人工验证。
GC-004 fixture 与 DetailPackageWriter::writeMinimalSectionLinePackage 输出字节一致。
```

TODO-024 done 不表示：

```text
完整 Detail 协议完成。
StbGroup 完成。
StbTable 完成。
MaterialTable 完成。
DrawingModel 完成。
RebarModel -> DetailExporter 完成。
钢筋生成器 / Viewer 选择 / 工程图标注已经接入 Detail。
```

## 人工验证结论

人工验证环境：

```text
AutoCAD 版本：AutoCAD 2020
插件导入方式：使用图石当前可成功的 AutoCAD 插件导入按钮路径
临时目录：M:\Device\C\Users\x\AppData\Local\Temp\msohtmplcllip
验证方式：图石临时目录替换法
截图：screenshot not attached
```

验证结果：

```text
旧 v1 minimal package:
  failed
  AutoCAD 插件 unhandled e06d7363h exception

v2_empty_groups:
  passed

v3_one_pointstb:
  passed
  仅作为后续最小 StbGroup / pointStb 兼容样本
  不作为 TODO-024 P0 baseline
```

三个实际测试：

```text
Test A:
  只放 v2 Detail01.stl
  独立单 sheet 导入成功

Test B:
  用 v2 替换真实包 Detail01.stl
  导入成功

Test C:
  用 v2 替换真实包 Detail03.stl
  导入成功
  已确认 v2 图元确实被读取 / 显示
```

关键判断：

```text
因为 v2_empty_groups 不含钢筋组也能成功，
TODO-024 P0 minimal package 以 v2_empty_groups 为基准。
```

## v2 P0 Baseline

当前 `DetailPackageWriter::writeMinimalSectionLinePackage` 写出：

```text
Detail.xml
  <StyleRoot/>

Detail01.stl
  DrawingRoot
    StbTables
    HViewPorts
      ViewPort
        PartDetailDrawing
          General-Info
          continue-line
          hidden-line
          central-line
          section-line
          hatch-line
          steeljoint-line
        StbDetailDrawing
          StbGroups stbGroupCount="0"
```

P0 v2 hard requirements：

```text
DrawingUnit 使用数值形式：
  DrawingUnit="1000"

ZValue 使用真实图石样本兼容格式：
  0.000000:0.000000:10.000000

每个 line category 保留：
  lines
  circles
  Arcs
  Ellipses
  EllipseArcs
  Splines

StbGroups 可以为空：
  stbGroupCount="0"
```

P0 不生成：

```text
StbGroup
StbGeo
StbTable
MaterialTable
```

## 实现内容

代码：

```text
app/src/drawing/detail/DetailPackageWriter.cpp
```

测试：

```text
app/tests/drawing/detail_package_writer_tests.cpp
```

GC-004 fixture：

```text
docs/validation/golden_cases/GC-004/minimal_detail_package/Detail.xml
docs/validation/golden_cases/GC-004/minimal_detail_package/Detail01.stl
```

新增协议文档：

```text
docs/detail/03_MinimalDetailPackage_v2_protocol.md
```

## TDD 记录

RED：

```powershell
cmd /c "call ""D:\Visual Studio 2026\Community\VC\Auxiliary\Build\vcvars64.bat"" >nul && cmake --build build/default --target tsrs_detail_package_writer_tests --config Debug && ctest --test-dir build/default -C Debug -R tsrs_detail_package_writer --output-on-failure"
```

失败符合预期：

```text
expected v2 minimal Detail01.stl to contain StbTables, v2 General-Info, StbGroups=0, escaped drawing name, and one section Line1
```

GREEN：

```text
tsrs_detail_package_writer_tests = passed
tsrs_detail_package_writer_todo66_fixture_probe = passed
```

## 当前 GC-004 Hash

```text
Detail.xml
  EA2FB44459C41800DF2251676DF6A72FB7B205FE7EB859925BA1AA2AFECEFFC0

Detail01.stl
  63ED30850AE2BFFA678DCFD8A2BD3DBA5F29C3DC30A9442763119EB5EA980E90
```

## 边界检查

本节点没有：

```text
实现 DrawingModel。
实现 RebarModel。
实现 StbGroupWriter。
实现 StbTable / MaterialTable。
接 Viewer。
接钢筋生成器。
调用 AutoCAD / ARX / DBX。
调用 RebarSmart DLL / VisualTS EXE/DLL / 3DE / CAA / CATIA / ACIS / HOOPS / Codejock。
把真实复杂 Detail 包整包复制进 minimal writer。
把 v3_one_pointstb 作为 P0 baseline。
```

## 剩余缺口

| ID | 缺口 | 处理 |
| --- | --- | --- |
| TODO024-GAP-001 | 完整 Detail 协议未完成。 | 后续按真实工程图字段逐步验证。 |
| TODO024-GAP-002 | StbGroup / pointStb 只知道 v3 最小样本可导入。 | v3 只作为后续兼容样本，不进入 TODO-024 P0 baseline。 |
| TODO024-GAP-003 | StbTable / MaterialTable 未验证。 | 后续单独做表格兼容 POC。 |
| TODO024-GAP-004 | Detail.xml 在按钮导入路径中不是本次必要条件。 | 当前仍写 `<StyleRoot/>` 保持普通 Detail 包目录兼容。 |
| TODO024-GAP-005 | ZValue 三段数值语义仍未完全解释。 | 当前只按真实图石兼容格式输出，不扩展语义结论。 |

## 下一步

TODO-024 已经不再阻塞后续 Viewer 选择前置链路。

下一阶段仍应按当前 todo 顺序推进：

```text
TODO-020D  TopologyBindingRegistry P1 hardening
TODO-020E  StepSession / ImportedModelStore 主链路
TODO-020G  CommandService skeleton guardrails
TODO-021   Viewer 选择系统
```

不要直接跳到完整工程图、钢筋表或 generator -> Detail 闭环。
