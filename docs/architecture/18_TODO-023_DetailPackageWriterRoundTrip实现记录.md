# TODO-023 DetailPackageWriter round-trip 实现记录

## 结论

TODO-023 完成了 Detail 包 preserve-mode Writer P0：

```text
DetailPackageReader
  -> DetailPackageSnapshot
  -> DetailPackageWriter::writePreserveMode
  -> output directory
  -> DetailPackageReader
  -> round-trip validation
```

本节点只证明：

```text
Reader 读到的 rawXml 可以原文写回。
写出后 Reader 能再次读取。
文件数量、rootName、knownSummary、unknownChildren、rawAttributes 的 P0 统计不减少。
真实 todo66 fixture 可执行 preserve-mode round-trip probe。
```

本节点不证明：

```text
Detail minimal generate 已完成。
旧 AutoCAD 插件 autoin 可接受 TS_RS 生成的新包。
DrawingModel / RebarModel 字段映射已完成。
Viewer / 选择系统 / 钢筋生成器已经接入 Detail。
结构化 XML mutation 可安全保留属性顺序、未知文本节点和 namespace declaration。
```

## 实现内容

新增：

```text
app/src/drawing/detail/DetailPackageWriter.h
app/src/drawing/detail/DetailPackageWriter.cpp
app/tests/drawing/detail_package_writer_tests.cpp
```

更新：

```text
app/CMakeLists.txt
app/src/drawing/detail/DetailPackageReader.h
app/tests/CMakeLists.txt
```

新增 CTest：

```text
tsrs_detail_package_writer_tests
tsrs_detail_package_writer_todo66_fixture_probe
```

`todo66` fixture probe 使用 CTest `SKIP_RETURN_CODE 77`：

```text
本机真实样本存在 -> probe pass / fail。
本机真实样本不存在 -> CTest skipped。
```

它不会把缺少真实样本伪装成普通通过。

## Writer DTO

新增：

```text
DetailPackageWriteResult
  targetDirectory
  filesWritten
  diagnostics
  ok()

DetailPackageWriter
  writePreserveMode(package, targetDirectory)
```

新增 diagnostic code：

```text
DETAIL_WRITE_FAILED
DETAIL_WRITE_VALIDATE_FAILED
```

## P0 行为

`writePreserveMode` 当前执行：

```text
1. 拒绝 !package.ok() 的输入，避免坏包先写出再失败。
2. 创建 target.tmp 临时输出目录。
3. 遍历 DetailPackageSnapshot.files。
4. 按原 fileName 写出 file.rawXml。
5. 用 DetailPackageReader 重新读取 target.tmp。
6. 比较：
   - 文件数量
   - rawXml 字节完全一致
   - knownSummary
   - rawAttributes 数量不减少
   - unknownChildren 数量不减少
7. 验证通过后把旧 target 移到 target.backup。
8. 将 target.tmp 提交为 target。
9. 提交失败时尝试恢复旧 target。
10. 提交成功后清理 target.backup，避免 stale file / partial package。
11. 失败时返回 DETAIL_WRITE_FAILED 或 DETAIL_WRITE_VALIDATE_FAILED。
```

P0 强约束：

```text
unchanged input file -> exact rawXml bytes written as output file contents
```

## 测试覆盖

新增测试：

```text
tsrs_detail_package_writer_tests
```

覆盖：

```text
1. synthetic Detail package 可 preserve-mode round-trip。
2. Detail.xml / Detail01.stl rawXml 字节完全相等。
3. 写出后 Reader 能再读。
4. 文件数量一致。
5. knownSummary 一致。
6. unknownChildren / rawAttributes 数量不减少。
7. malformed source package 写出前会被拒绝，不会留下 Detail.xml / Detail01.stl。
8. unsafe fileName（绝对路径 / `..` / 非 basename）会被拒绝，不能逃逸 targetDirectory。
9. preserve-mode 使用临时目录事务写出，提交后不会保留旧目标目录中的 stale DetailNN.stl。
10. targetDirectory 带尾部分隔符时，临时目录仍创建为 target 的兄弟目录，不嵌套进 target 内部。
11. backup 目录被占用时写入失败，且旧 target 字节保持不变。
12. minimal writer 同样覆盖 stale target replacement 和 trailing separator target。
```

真实样本 probe：

```text
tsrs_detail_package_writer_todo66_fixture_probe
```

覆盖：

```text
1. 读取 docs/phase1/todo66 真实 Detail 包。
2. preserve-mode 写到临时目录。
3. 写出文件数量等于源文件数量。
4. 写出后 Reader 能再读。
5. 每个文件 knownSummary 一致。
6. 每个文件 rawXml 字节完全相等。
```

## 验证记录

RED：

```text
目标构建失败，原因符合预期：
fatal error C1083: 无法打开包括文件: “drawing/detail/DetailPackageWriter.h”
```

专项验证：

```powershell
cmd /c 'call "D:\Visual Studio 2026\Community\VC\Auxiliary\Build\vcvars64.bat" >nul && cmake --build build/default --target tsrs_detail_package_writer_tests --config Debug && ctest --test-dir build/default -C Debug -R tsrs_detail_package_writer --output-on-failure'
```

结果：

```text
2/2 passed
```

默认 CTest：

```powershell
cmd /c 'call "D:\Visual Studio 2026\Community\VC\Auxiliary\Build\vcvars64.bat" >nul && ctest --test-dir build/default -C Debug --output-on-failure'
```

结果：

```text
22/22 passed
```

## xhigh review

本节点提交前执行了 xhigh 只读 review。

review 结论：

```text
Verdict: Ready
Critical: none
Important: none
```

Minor：

```text
1. Writer 公开 DTO 被手工构造时，fileName 需要 basename / target-boundary guardrail。
2. validation diagnostic 应输出更具体的 before / after 计数差异，方便真实样本排查。
```

处理：

```text
1. 已新增 unsafe fileName 测试。
2. 已新增 Writer 文件名保护：fileName 必须是非绝对路径 basename。
3. 已增强 DETAIL_WRITE_VALIDATE_FAILED message，包含 knownSummary / rawAttributes / unknownChildren 的 before / after 摘要。
4. 修复后重新运行 writer 专项与默认 CTest。
```

修复后验证：

```text
tsrs_detail_package_writer = 2/2 passed
default CTest = 22/22 passed
```

边界扫描：

```powershell
rg -n "TopoDS_|AIS_|BRep|TopAbs|gp_|Geom_|Rebar|FixDistance|FixNumber|Viewer|MainWindow|autoin|DrawingModel|RebarModel" app/src/drawing/detail app/tests/drawing docs/architecture/17_TODO-023_DetailPackageWriter_roundtrip_scope.md
```

结果：

```text
app/src/drawing/detail 和 app/tests/drawing 无 OCCT / Viewer / Rebar / autoin / DrawingModel / RebarModel 命中。
scope 文档中仅在 forbidden / non-goal 描述里出现这些词。
```

`domain/rebar` OCCT 泄漏检查：

```text
domain/rebar not present
```

## 边界检查

本节点没有：

```text
实现 minimal generate。
执行旧 AutoCAD 插件 autoin 验证。
创建 DrawingModel。
创建 RebarModel。
连接 Viewer。
连接选择系统。
连接钢筋生成器。
调用 VisualTS / RebarSmart DLL / 3DE / CAA。
```

## 当前 GAP

| ID | 缺口 | 处理 |
| --- | --- | --- |
| TODO023-GAP-001 | Writer P0 只做 rawXml passthrough，不做结构化 XML mutation。 | 后续如需 mutation，先建立强 XML model。 |
| TODO023-GAP-002 | preserve-mode / minimal writer 已实现 target.tmp + target.backup 事务写出；不是跨文件系统强原子替换。 | 如后续需要更强保证，再设计平台相关 atomic replace 策略。 |
| TODO023-GAP-003 | CAD 插件 minimal sheet 导入验证。 | 已由 TODO-024 / GC-004 manual_autoin_passed_v2 关闭；round-trip 不等于完整 Detail 兼容。 |
| TODO023-GAP-004 | malformed source 写出前拒绝已完成。 | 已按 TODO-023/024 外审整改。 |
| TODO023-GAP-005 | 非 UTF-8 / GBK 旧 Detail XML 尚未用更多真实样本验证。 | 后续收集更多旧包时补编码观测。 |

## 下一步

下一步是：

```text
TODO-024 极简 Detail 包生成 + autoin 验证
```

边界：

```text
生成最小 Detail.xml + Detail01.stl XML。
由用户或具备环境的一侧执行旧 AutoCAD 插件 autoin 人工验证。
不宣称兼容完成，直到 autoin 证据落文档。
```
