# TODO-022 DetailPackageReader P0 实现记录

## 结论

TODO-022 完成了旧图石 Detail 包的只读 Reader P0：

```text
Detail package directory
  -> Detail.xml
  -> DetailNN.stl XML
  -> DetailPackageSnapshot
  -> DetailFileSnapshot
  -> knownSummary
  -> rawXml / fileName / sheetIndex
```

本节点只证明：

```text
旧 Detail 包可按 XML 读取。
Reader 能保留 rawXml / 原文件名 / sheetIndex。
Reader 能输出 P0 统计视图。
真实 todo66 fixture manifest hash 可校验。
```

本节点不证明：

```text
Detail Writer 已完成。
round-trip 已完成。
极简 Detail 包可被 CAD 插件 autoin 接受。
DrawingModel / RebarModel 字段映射已完成。
Viewer / 钢筋生成器已经接入 Detail。
```

## 实现内容

新增：

```text
app/src/drawing/detail/DetailPackageReader.h
app/src/drawing/detail/DetailPackageReader.cpp
app/tests/drawing/detail_package_reader_tests.cpp
```

更新：

```text
app/CMakeLists.txt
app/tests/CMakeLists.txt
```

新增 CMake target：

```text
tsrs_detail
tsrs_detail_package_reader_tests
```

## Reader DTO

P0 DTO：

```text
DetailPackageSnapshot
  sourceDirectory
  files[]
  diagnostics[]

DetailFileSnapshot
  fileName
  sheetIndex
  rootName
  rawXml
  knownSummary
  rawAttributes
  unknownChildren
  diagnostics

DetailKnownSummary
  xmlNodeCount
  viewPortCount
  partDetailDrawingCount
  stbTableCount
  stbRowCount
  materialTableCount
  matRowCount
  sectionLineCount
  stbGroupElementCount
  stbGroupsContainerCount
  stbGroupEntryCount
  stdElementCount
  stbGeoElementCount
  faceEdgeCount

DetailRawAttribute
  nodePath
  name
  value
  namespaceUri
  qualifiedName

DetailUnknownChild
  nodePath
  name
  namespaceUri
```

诊断：

```text
DETAIL_PACKAGE_MISSING
DETAIL_STYLE_MISSING
DETAIL_SHEET_MISSING
DETAIL_XML_PARSE_FAILED
DETAIL_ROOT_UNEXPECTED
DETAIL_SHEET_INDEX_GAP
```

## 真实样本验证

真实样本：

```text
C:\Users\ghost\Desktop\reverse_engineering\【03】图石软件\docs\phase1\todo66
```

manifest：

```text
docs/validation/fixtures/detail_todo66_manifest.md
```

测试校验：

```text
Detail.xml sha256
Detail01.stl sha256
Detail02.stl sha256
Detail03.stl sha256
Detail04.stl sha256
```

真实样本统计：

```text
Detail01.stl nodes=138 viewPorts=1 partDrawings=1 stbTables=1 stbRows=7 materialTables=1 matRows=2 sectionLines=8 stbGroups=13 stds=19 stbGeos=23 faceEdges=7
Detail02.stl nodes=67  viewPorts=1 partDrawings=1 stbTables=0 stbRows=0 materialTables=0 matRows=0 sectionLines=4 stbGroups=7  stds=9  stbGeos=9  faceEdges=3
Detail03.stl nodes=73  viewPorts=1 partDrawings=1 stbTables=0 stbRows=0 materialTables=0 matRows=0 sectionLines=10 stbGroups=6 stds=10 stbGeos=12 faceEdges=0
Detail04.stl nodes=81  viewPorts=1 partDrawings=1 stbTables=0 stbRows=0 materialTables=0 matRows=0 sectionLines=10 stbGroups=8 stds=13 stbGeos=15 faceEdges=0
```

## 测试

新增测试：

```text
tsrs_detail_package_reader_tests
```

覆盖：

```text
1. synthetic Detail package 可读取。
2. rawXml 保留。
3. rawAttributes 和 unknownChildren 保留。
4. rawAttributes 保留属性名和值。
5. unknownChildren 保留未知节点名和路径。
6. malformed XML diagnostic 包含 line / column。
7. Detail1 / Detail03 / Detail100 sheetIndex 按完整数字解析。
8. missing package 返回 DETAIL_PACKAGE_MISSING。
9. 真实 todo66 fixture hash 与 manifest 一致。
10. 真实 Detail.xml root = StyleRoot。
11. 真实 Detail01..04.stl root = DrawingRoot。
12. 真实 Detail01..04.stl knownSummary 与观察值一致。
```

TODO-022 外部评审后，真实 todo66 fixture probe 已拆成独立 CTest：

```text
tsrs_detail_package_reader_todo66_fixture_probe
```

如果本机找不到真实样本，该测试返回 77 并由 CTest 标记为 skipped，
不再把“缺真实样本”伪装成普通通过。

专项验证：

```powershell
cmd.exe /c "call ""D:\Visual Studio 2026\Community\Common7\Tools\VsDevCmd.bat"" -arch=x64 -host_arch=x64 >nul && cmake --build build/default --config Debug --target tsrs_detail_package_reader_tests && ctest --test-dir build/default --output-on-failure -C Debug -R tsrs_detail_package_reader"
```

初次 TODO-022 结果：

```text
1/1 passed。
```

外部评审补强后结果：

```text
2/2 passed。
```

默认 CTest：

```powershell
ctest --test-dir build/default --output-on-failure -C Debug
```

初次 TODO-022 结果：

```text
19/19 passed。
```

外部评审补强后结果：

```text
20/20 passed。
```

构建环境说明：

```text
当前 PowerShell shell 没有预加载 Visual Studio INCLUDE / LIB。
需要通过 VsDevCmd 包住新增 C++ target 构建。

并行全量构建曾触发 vcpkg runtime copy 到同一 build/default 目录的 Permission denied。
串行构建 `cmake --build build/default --config Debug -- -j1` 通过。
这是既有测试 runtime copy 策略的并行竞争风险，不是 Detail Reader 逻辑失败。
后续可单独收敛为 shared runtime copy target。
```

## xhigh review

本节点提交前执行了 xhigh 只读 review。

首次 review：

```text
Critical: none
Important: README.md 超出本轮允许修改范围。
Minor: 真实 todo66 fixture 缺失时测试会跳过真实 manifest probe。
```

处理：

```text
README.md 变更已从本轮 diff 移除。
Minor 项保留为后续 fixture/gate 策略风险，不阻塞 TODO-022。
```

复审：

```text
Verdict: Ready
Critical: none
Important: none
Evidence: README.md 已不在 diff 中，剩余 diff 均在本轮允许范围内。
```

## 边界检查

本节点没有修改：

```text
app/src/rebarsmart
app/src/ui
app/src/presentation/occ
app/src/application
```

本节点没有：

```text
写 Detail 包。
创建 DrawingModel。
创建 RebarModel。
连接 Viewer。
连接钢筋生成器。
调用 VisualTS / RebarSmart DLL / 3DE / CAA。
```

## 当前 GAP

| ID | 缺口 | 处理 |
| --- | --- | --- |
| TODO022-GAP-001 | rawXml 已保留，但 Writer round-trip 尚未实现。 | TODO-023。 |
| TODO022-GAP-002 | unknownChildren 已记录未知 element 路径 / 名称 / namespace，但不做完整未知子树模型。 | TODO-023 preserve-mode 优先 rawXml 原文回写。 |
| TODO022-GAP-003 | rawAttributes 已记录属性名和值，但不保留原始属性顺序 / quote 风格。 | TODO-023 只做 rawXml passthrough；后续结构化 XML mutation 前再扩展。 |
| TODO022-GAP-004 | CAD 插件 minimal sheet 导入验证。 | 已由 TODO-024 / GC-004 manual_autoin_passed_v2 关闭；完整 Detail 兼容仍是后续缺口。 |
| TODO022-GAP-005 | DetailNN.stl 编号缺口目前只 warning。 | Writer / package validation 阶段继续收紧。 |
| TODO022-GAP-006 | 非 UTF-8 / GBK 旧 Detail XML 尚未用真实样本验证。 | 后续收集更多旧包时补编码观测。 |

## 下一步

下一步是：

```text
TODO-023 DetailPackageWriter round-trip
```

边界：

```text
只做 Reader -> Writer preserve-mode round-trip。
Writer P0 必须 rawXml passthrough。
不做 minimal generate。
不接 Viewer。
不接钢筋生成器。
不创建 RebarModel / DrawingModel 映射。
不宣称 CAD 插件兼容完成。
```
