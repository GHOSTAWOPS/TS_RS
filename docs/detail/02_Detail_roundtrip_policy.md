# Detail round-trip policy

## 结论

Detail Reader / Writer 的第一目标不是“写出漂亮的新工程图”，而是：

```text
保守读取旧包。
尽量无损回写旧包。
不丢未知节点。
不丢未知属性。
为后续极简 autoin 验证建立最低协议。
```

## 为什么先 round-trip

Detail 包是老图石 CAD 插件兼容出口。

当前风险：

```text
字段必填性未知。
属性顺序是否敏感未知。
未知节点能否丢弃仍未知。
Detail.xml 是否必须存在未知。
CAD 插件容忍度未知。
```

所以 P0 必须先做：

```text
Lossless-ish round-trip first.
```

## Reader P0 策略

Reader 读入：

```text
Detail.xml
DetailNN.stl
```

Reader 输出：

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
```

TODO-022 P0 额外边界：

```text
只读旧包。
只做统计视图和 rawXml 保留。
不做 DrawingModel 字段映射。
不做 RebarModel 映射。
不写 Detail 包。
不声称 CAD 插件兼容。
```

真实证据门禁：

```text
TODO-022 必须使用 docs/validation/fixtures/detail_todo66_manifest.md
指向的真实样本，或使用从该样本脱敏得到的 repo-local fixture。

synthetic XML 只能补边界测试，不能替代真实样本。
```

要求：

```text
1. DetailNN.stl 按 XML 读取。
2. rawXml 必须可用于未修改回写。
3. 识别的 knownSummary 只做统计，不删除未知字段。
4. 解析失败必须保留文件名和错误位置。
```

## Writer P0 策略

Writer P0 支持两种模式：

### Preserve mode

输入：

```text
Reader 读到的 DetailPackageSnapshot
```

输出：

```text
结构和原文件等价的 Detail 包。
```

要求：

```text
未修改文件优先原文回写。
不重排未知节点。
不格式化导致字段顺序大幅变化。
不主动删除空容器。
```

### Minimal generate mode

输入：

```text
最小 DrawingModel / 手写 fixture。
```

输出：

```text
Detail.xml
Detail01.stl
```

要求：

```text
Detail.xml 默认写 <StyleRoot/>。
Detail01.stl root 写 DrawingRoot。
至少包含 HViewPorts / ViewPort / PartDetailDrawing / section-line。
能用于 TODO-024 autoin 人工验证。
```

## 文件事务策略

写出 Detail 包必须走临时目录：

```text
target/
  Detail.xml
  Detail01.stl

target.tmp/
  Detail.xml
  Detail01.stl
```

流程：

```text
1. 写 target.tmp。
2. 校验 XML well-formed。
3. 校验 DetailNN 编号连续。
4. 校验 count 字段和实际节点数量。
5. 备份旧 target。
6. 原子替换或尽量短窗口替换。
7. 失败则保留旧 target。
```

P0 可以先在测试目录内实现，不碰用户真实目录。

## Unknown preservation

必须保留：

```text
未知 element。
未知 attribute。
未知 namespace。
未知文本节点。
未知空容器。
原始文件名。
sheetIndex。
```

P0 不承诺：

```text
保留字节级完全一致。
保留属性原始顺序。
保留 XML declaration。
保留原始缩进。
```

但是如果没有修改某个文件，优先 rawXml 原文回写。

## Validation

Reader validation：

```text
Detail.xml 存在。
至少一个 DetailNN.stl 存在。
每个 DetailNN.stl 是 well-formed XML。
rootName = DrawingRoot。
StbGroups.stbGroupCount 和 StbGroup# 数量差异只报警，不直接失败。
```

Writer validation：

```text
写出后能重新 Reader 读入。
文件数量一致。
关键统计一致。
未知节点数量不减少。
未知属性数量不减少。
```

Autoin validation：

```text
必须人工执行。
旧 CAD 插件能导入，才算兼容证据闭合。
```

## Diagnostics

错误码：

```text
DETAIL_PACKAGE_MISSING
DETAIL_STYLE_MISSING
DETAIL_SHEET_MISSING
DETAIL_XML_PARSE_FAILED
DETAIL_ROOT_UNEXPECTED
DETAIL_SHEET_INDEX_GAP
DETAIL_COUNT_MISMATCH
DETAIL_UNKNOWN_DROPPED
DETAIL_WRITE_VALIDATE_FAILED
DETAIL_AUTOIN_NOT_VERIFIED
```

每个 diagnostic 必须包含：

```text
fileName
sheetIndex
nodePath
severity
message
```

## 测试计划

### Round-trip fixture

使用：

```text
docs/phase1/todo66/Detail.xml
docs/phase1/todo66/Detail01.stl
docs/phase1/todo66/Detail02.stl
docs/phase1/todo66/Detail03.stl
docs/phase1/todo66/Detail04.stl
```

断言：

```text
Reader 能读。
Writer 能写。
写出后 Reader 能再读。
文件数量一致。
rootName 一致。
StbGroup / StbRow / MaterialTable / section-line 统计不减少。
```

### Unknown fixture

构造：

```text
给 Detail01.stl 加未知节点和未知属性。
```

断言：

```text
round-trip 后未知节点 / 属性仍存在。
```

### Minimal fixture

构造：

```text
Detail.xml = <StyleRoot/>
Detail01.stl = DrawingRoot + HViewPorts + section-line 一条线。
```

断言：

```text
Reader 能读。
Writer 能写。
等待 TODO-024 人工 autoin。
```

## 与 TS_RS 模型边界

DetailPackageSnapshot：

```text
兼容包模型。
服务 Reader / Writer / round-trip。
```

DrawingModel：

```text
TS_RS 内部工程图模型。
后续由 DrawingModel 映射到 DetailPackage。
```

RebarModel：

```text
TS_RS 内部钢筋模型。
不直接保存 Detail 字段。
```

禁止：

```text
为了适配 Detail 字段，把 RebarModel 改成 StbGroup 结构。
```

## 后续 TODO

```text
TODO-022：DetailPackageReader P0。
TODO-023：DetailPackageWriter round-trip。
TODO-024：极简 Detail 包生成 + autoin 人工验证。
```
