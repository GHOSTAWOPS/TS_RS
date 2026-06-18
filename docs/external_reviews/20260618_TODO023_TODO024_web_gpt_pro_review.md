# TODO-023 / TODO-024 Web GPT Pro 外部审查记录

审查时间：2026-06-18

审查状态：completed

审查节点：

```text
TODO-023 DetailPackageWriter round-trip
TODO-024 极简 Detail 包生成
```

审查对话：

```text
https://chatgpt.com/c/6a28c1d7-c50c-83ec-8bf4-29c4df9ea152
```

上传审查包：

```text
review_packages/TODO-023_024_web_gpt_pro_review_20260618_101947.zip
```

审查包目录：

```text
review_packages/TODO-023_024_web_gpt_pro_review_20260618_101947/
```

审查对象版本：

```text
TODO-023 commit/tag = e5bec8f / tsrs-todo-023-detail-writer-roundtrip-20260617
TODO-024 commit/tag = 71f2355 / tsrs-todo-024-minimal-detail-package-20260617
GitHub = https://github.com/GHOSTAWOPS/TS_RS.git
```

本地验证状态：

```text
默认 CTest = 22/22 pass
TODO-024 当前状态 = blocked
blocked 原因 = 等待旧 AutoCAD 图石插件 autoin 人工验证 GC-004 minimal_detail_package
```

## Verdict

```text
CONDITIONAL GO
```

解释：

```text
TODO-023 / TODO-024 没有被外部审查无条件放行。
TODO-023 preserve-mode 方向可继续，但必须补强坏包拒绝、真实样本证据和运行时字节校验。
TODO-024 必须继续 blocked，直到旧 AutoCAD 插件 autoin 人工验证有结果。
```

## Critical

### 1. 真实 todo66 证据门禁仍然不可复现

证据：

```text
docs/detail/00_Detail包证据索引.md 仍把真实样本定位到本机路径。
docs/validation/fixtures/detail_todo66_manifest.md 承认真实证据可能是本机 fixture + hash。
detail_package_reader_tests.cpp 和 detail_package_writer_tests.cpp 找不到 todo66 fixture 时返回 77，由 CTest 记为 skipped。
```

影响：

```text
TODO-023 preserve-mode round-trip 在开发者本机可能成立，
但对外部审查、CI、他人机器并不可复现。
TODO-024 极简包也缺少可复现真实对照基线。
```

建议：

```text
继续后续 Detail 节点前处理。
优先加入脱敏 repo-local fixture。
如果不能入库，至少把真实 fixture probe 改为显式 blocked / required manual evidence，
避免 22/22 pass 被误读为真实样本已覆盖。
```

处理状态：

```text
deferred / still open

当前只保留 local fixture probe + CTest skipped 显式可见策略。
repo-local sanitized todo66 fixture 尚未提供。
后续进入更广泛 Detail 兼容节点前继续处理。
```

### 2. writePreserveMode() 会先写出坏包再验证失败

证据：

```text
app/src/drawing/detail/DetailPackageWriter.cpp 的 writePreserveMode()
没有先检查 package.ok() 或 DetailFileSnapshot::ok()，而是直接遍历 package.files 写文件。

app/tests/drawing/detail_package_writer_tests.cpp 的 expectWriterValidatesOutput()
把坏 XML 先写出、之后触发 DETAIL_WRITE_VALIDATE_FAILED 固化为预期。
```

影响：

```text
Writer P0 不是“只接受有效 Reader 输出并保守回写”，
而是允许把坏输入复制到目标目录后再报告失败。
后续接真实用户目录时，可能留下半成品 / 损坏包。
```

建议：

```text
TODO-023 之后必须处理。
最低要求：writePreserveMode() 遇到 !package.ok() 时直接拒绝写出。
更稳妥：写入 target.tmp，re-read 验证成功后再替换正式目录。
```

处理状态：

```text
fixed

writePreserveMode() 现在先拒绝 !package.ok() 输入，
不会把 malformed source package 写入目标目录。
同时已引入 target.tmp 验证后替换目标目录。
```

### 3. TODO-024 必须继续 blocked

证据：

```text
docs/architecture/19_TODO-024_MinimalDetailPackage实现记录.md 已写明 autoin 尚未人工执行。
docs/validation/golden_cases/GC-004/README.md 状态为 blocked_waiting_manual_autoin。
GC-004 测试只证明 Writer 输出能被 TS_RS Reader 再读，并与仓库内固定包字节一致。
```

影响：

```text
当前极简包只证明 TS_RS 自己能读自己写出的包。
不能证明旧 AutoCAD 插件 autoin 接受。
不能证明 Detail 协议最低必填字段已经闭合。
```

建议：

```text
TODO-024 继续 blocked。
autoin 人工验证通过前，不得改为 done。
不得把 TODO-024 当作 Detail 兼容完成证据。
```

处理状态：

```text
accepted / blocked
```

### 4. TODO-024 最小包字段仍是猜测最小值

证据：

```text
minimalSheetXml() 只生成：
DrawingRoot / HViewPorts / ViewPort / PartDetailDrawing / General-Info /
section-line / StbDetailDrawing / StbGroups。

没有写 StbTables。

真实样本观察结构中 DrawingRoot 顶层包含：
StbTables 和 HViewPorts。
General-Info 真实字段也明显多于当前极简包。
```

影响：

```text
如果旧 CAD 插件对 StbTables、General-Info 字段或空子容器有最小要求，
当前极简包很可能失败。
```

建议：

```text
保持 blocked。
autoin 失败时不要复制真实复杂包。
按最小字段顺序补：
1. <StbTables/> 顶层空容器
2. General-Info 基础数值字段
3. BasePoint_* / Range_*
4. CutPlane* / TopDir*
5. 空的 circles / Arcs / Ellipses / EllipseArcs / Splines 子容器
```

处理状态：

```text
pending autoin result
```

## Important

### 1. preserve-mode 保守性来自 rawXml，不是 rawAttributes / unknownChildren

证据：

```text
docs/architecture/17_TODO-023_DetailPackageWriter_roundtrip_scope.md
和 docs/detail/02_Detail_roundtrip_policy.md 已经写明：
rawAttributes / unknownChildren 当前只是 diagnostics/guardrail。

但 detail_package_writer_tests.cpp 的回归比较可能让人误读为
rawAttributes / unknownChildren 数量不减少就等于未知字段被结构化保留。
```

影响：

```text
未来做结构化 XML mutation 时，容易高估当前 DTO 能力。
```

建议：

```text
在 DetailPackageWriter.h/.cpp 或 docs/architecture/17 中补硬约束：

TODO-023 preserve guarantee comes from rawXml passthrough only.
rawAttributes/unknownChildren are diagnostics, not mutation-ready preservation data.

结构化修改前必须先引入 raw subtree / unknown text / namespace / attribute ordering 模型。
```

处理状态：

```text
fixed / documented

docs/architecture/17_TODO-023_DetailPackageWriter_roundtrip_scope.md 已补硬约束。
docs/detail/02_Detail_roundtrip_policy.md 已补 rawXml passthrough 口径。
```

### 2. Writer 运行时验证弱于文档承诺

证据：

```text
docs/architecture/18_TODO-023_DetailPackageWriterRoundTrip实现记录.md 写明：
unchanged input file -> exact rawXml bytes written as output file contents。

但 DetailPackageWriter.cpp 运行时只比较 knownSummary、
rawAttributes.size()、unknownChildren.size()，
没有对输出文件内容做 hash / byte equality 校验。
```

影响：

```text
后续如果改动影响 rawXml 写出字节，测试可能覆盖不到运行时场景。
```

建议：

```text
preserve-mode validate 阶段加入 source rawXml == written file bytes。
```

处理状态：

```text
fixed

writePreserveMode() validate 阶段已比较 source rawXml 与临时写出文件字节。
```

### 3. XML declaration / encoding / namespace 证据不足

证据：

```text
docs/detail/02_Detail_roundtrip_policy.md 提到未来需要 namespace declarations、
encoding policy、strict validation mode。

当前 reader/writer 测试没有覆盖：
XML declaration
namespace declaration
非 UTF-8 / 含编码声明输入
```

影响：

```text
当前 todo66 样本可能没问题，但其他旧包可能出现 Reader/Writer 不兼容。
```

建议：

```text
进入更广泛 Detail 兼容前补三类样例：
1. 含 XML declaration 的包
2. 含 namespace 的包
3. 非 UTF-8 或声明编码的包
```

处理状态：

```text
deferred / still open

XML declaration / namespace / encoding 测试未在本轮补。
保留为后续 Detail 兼容扩展前的专项任务。
```

### 4. GC-004 是自举夹具，不是兼容证据

证据：

```text
GC-004 测试验证的是仓库内 minimal_detail_package 与当前 Writer 输出完全一致。
```

影响：

```text
GC-004 只是锁定 Writer 当前输出形状。
如果 Writer 猜错，这个 fixture 也会把错误冻住。
```

建议：

```text
把 GC-004 明确标成：
writer regression fixture, not compatibility proof。

兼容性证据只能来自 autoin 人工验证记录。
```

处理状态：

```text
fixed / documented

docs/validation/golden_cases/GC-004/README.md 已明确：
writer regression fixture, not compatibility proof。
```

### 5. General-Info 取值形式仍有未证实假设

证据：

```text
minimalSheetXml() 写：
<General-Info DrawingName="..." DrawingUnit="mm" DrawingScale="1" GeneralScale="1"/>

docs/detail/01_DetailXML_observed_schema.md 只证明字段存在，
没有证明词法形式一定是当前写法。
```

影响：

```text
如果旧 CAD 插件要求固定枚举、数值格式或配套字段成组出现，
当前极简包会失败。
```

建议：

```text
保持 blocked。
autoin 失败后优先补证据最强、最小的一组字段，
不要直接全量复制真实 General-Info。
```

处理状态：

```text
pending autoin result
```

### 6. Writer 没有事务性写出

证据：

```text
docs/detail/02_Detail_roundtrip_policy.md 已写 target.tmp -> validate -> swap。
DetailPackageWriter.cpp 目前直接在目标目录写文件。
```

影响：

```text
真实导出目录失败时可能留下半成品。
非空目录中旧 Detail02.stl 等 stale 文件也可能导致 Reader 验证异常。
```

建议：

```text
任何用户可见导出前必须补。
最晚应在 TODO-020D / TODO-020E / TODO-021 之前加上。
```

处理状态：

```text
fixed

DetailPackageWriter preserve-mode 与 minimal package 写出已使用 target.tmp 验证后替换目标目录。
仍未实现旧 target backup / rollback，作为用户可见导出前残余风险保留。
```

### 7. TODO-020D / 020E / 020G 与 TODO-024 依赖策略需要细化

证据：

```text
todo.csv 当前：
TODO-020D 依赖 TODO-024
TODO-020G 依赖 TODO-024
TODO-021 依赖 TODO-020E; TODO-020G; TODO-024
```

影响：

```text
如果 020D / 020E / 020G 都必须等 autoin，
会把与 Detail 兼容无关的导入、binding、CommandService 边界工作一起阻塞。
```

建议：

```text
TODO-020D / TODO-020E / TODO-020G 可以继续，不必等 autoin。
TODO-021 继续等待 TODO-024 autoin 至少出一个结果。
```

处理状态：

```text
fixed

todo.csv 已调整：
TODO-020D = next，依赖 TODO-020B; TODO-023。
TODO-020G 不再依赖 TODO-024。
TODO-021 继续依赖 TODO-024。
```

## Minor

### 1. TODO-023 / TODO-024 共用 writer tests，节点信号有点混

建议：

```text
后续可拆分：
tsrs_detail_package_writer_roundtrip_tests
tsrs_detail_package_minimal_package_tests
```

处理状态：

```text
deferred
```

### 2. sheetIndex / 文件枚举规则未来兼容性仍需更多证据

建议：

```text
当前不是阻塞项。
把 P0 仅支持现有命名形态写入 docs 会更稳。
```

处理状态：

```text
deferred
```

### 3. 最小包 LF / CRLF 差异未验证

建议：

```text
等第一次 autoin 结果出来后再决定是否补这类测试。
```

处理状态：

```text
deferred pending autoin result
```

## Required Follow-up TODOs

必须先补：

```text
1. 为 TODO-022 / TODO-023 提供 repo-local sanitized todo66 fixture，
   或把 local fixture probe 改成 explicit blocked / explicit skipped，
   不计作可复现通过。

2. 在 TODO-023 scope 文档中强化：
   preserve-mode guarantee currently comes from rawXml passthrough only.
   rawAttributes/unknownChildren are diagnostics, not structural preservation.

3. TODO-024 继续 blocked，
   直到旧 AutoCAD 插件 autoin 有人工验证记录。

4. 明确 GC-004：
   regression fixture, not compatibility proof。
```

进入后续 Detail 代码前尽快补：

```text
1. writePreserveMode() 先拒绝 !package.ok() 输入。
2. 或引入 target.tmp -> validate -> swap。
3. 补 XML declaration / namespace / encoding 测试。
4. 运行时 validation 加字节级等价或 hash 比较。
```

如果 autoin 失败，按这个顺序补最小字段：

```text
1. 顶层 <StbTables/>
2. General-Info 最小数值字段：
   DrawingType / LevelDrawing / DrawingScale / GeneralScale
3. BasePoint_* / Range_*
4. CutPlane* / TopDir*
5. 空几何子容器：
   circles / Arcs / Ellipses / EllipseArcs / Splines
6. 仍失败时，再看 StbTable / MaterialTable 空容器
```

不要第一时间复制真实复杂 Detail01.stl。

关于后续顺序：

```text
外部审查建议：
TODO-020D / TODO-020E / TODO-020G 可以继续，不必等 autoin。
TODO-021 Viewer 选择系统建议继续等待 TODO-024 autoin 至少有结果。

当前项目 todo.csv 已按此建议调整：
TODO-020D / TODO-020E / TODO-020G 可以继续；
TODO-021 继续等待 TODO-024 autoin。
```

## 当前处理结论

```text
1. TODO-023 / TODO-024 外部审查已完成。
2. TODO-024 继续 blocked。
3. 已执行外审整改：
   - writePreserveMode() 拒绝 !package.ok() 输入。
   - preserve-mode validate 增加 rawXml 字节级比较。
   - Writer 使用 target.tmp 验证后提交。
   - Writer 使用 target.backup 备份旧 target。
   - commit 失败时尝试恢复旧 target。
   - backup 路径被占用时拒绝写入并保持旧 target 字节不变。
   - preserve-mode / minimal writer 都覆盖 stale target replacement。
   - preserve-mode / minimal writer 都覆盖 trailing separator target。
   - GC-004 已标记为 regression fixture，不是 compatibility proof。
   - todo.csv 已拆开 TODO-024 对 020D / 020E / 020G 的阻塞。
4. xhigh 只读复审：
   - 第一轮：Needs changes，指出旧 target backup / rollback 缺口。
   - 第二轮：Pass，Critical=None，Important=None，Minor=None。
5. 本地验证：
   - 默认 CTest = 22/22 passed。
   - todo.csv 唯一 next = TODO-020D。
   - Detail writer/test 边界扫描无 OCCT / Viewer / Rebar / DrawingModel / RebarModel 泄漏。
6. TODO-024 继续 blocked。
7. TODO-020D 已设置为 next。
8. 仍需后续处理：
   - repo-local sanitized todo66 fixture 或更硬的真实样本门禁策略。
   - XML declaration / namespace / encoding 测试。
   - GC-004 autoin 人工验证。
```
