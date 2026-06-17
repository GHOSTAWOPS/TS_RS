# Detail 包证据索引

## 结论

TODO-018 只做 Detail 包证据索引和 round-trip policy，不写 C++。

当前可用真实样本来自：

```text
C:\Users\ghost\Desktop\reverse_engineering\【03】图石软件\docs\phase1\todo66
```

可提交追溯清单：

```text
docs/validation/fixtures/detail_todo66_manifest.md
```

TODO-022 启动条件：

```text
必须使用该本机真实样本并校验 manifest，
或使用由该 manifest 派生的 repo-local 脱敏 fixture。

只用手写 synthetic XML 不能标记 TODO-022 done。
```

样本包括：

```text
Detail.xml
Detail01.stl
Detail02.stl
Detail03.stl
Detail04.stl
下料表.xls
消力池下游侧带齿槽底板结构图石钢筋模型.sfl
截图
```

核心观察：

```text
Detail.xml = 14 bytes，内容为 <StyleRoot/>。
用户已反馈多台电脑生成结果一致。
生成工程图和 CAD 导入时 Detail.xml 修改时间不更新。

DetailNN.stl = XML 文本，不是标准 STL 三角网格。
真实工程图、剖切线、钢筋组、下料表字段主要在 DetailNN.stl。
```

## 证据等级

### E1：用户运行回填样本

路径：

```text
docs/phase1/todo66/
```

状态：

```text
可作为 TS_RS 当前 Detail 包主证据。
```

包含：

```text
Detail.xml
Detail01.stl
Detail02.stl
Detail03.stl
Detail04.stl
下料表.xls
源 SFL
截图
```

约束：

```text
本样本证明旧图石可生成非空 DetailNN.stl。
本样本不证明 CAD 插件已经成功导入 TS_RS 生成的新包。
```

### E2：旧实现项目文档

路径：

```text
【图石钢筋1比1复刻】/docs/phase1/runtime_capture/todo_065_generate_package_and_schedule/
【图石钢筋1比1复刻】/docs/phase1/app_build_reports/m2_drawing_035_run_001.md
【图石钢筋1比1复刻】/docs/phase1/app_build_reports/m2_drawing_036_run_001.md
【图石钢筋1比1复刻】/docs/phase1/app_build_reports/m2_drawing_037_run_001.md
```

用途：

```text
只作为 Detail 字段、样本观察和旧验证记录参考。
不迁入旧实现项目的代码。
不把旧实现项目的技术路线带入 TS_RS。
```

### E3：AutoCAD 插件人工验证

当前状态：

```text
未闭合。
```

后续 TODO-024 必须人工验证：

```text
极简 Detail.xml + Detail01.stl XML 能否被旧 AutoCAD 插件 autoin 接受。
```

## 当前样本摘要

### Detail.xml

```text
bytes = 14
content = <StyleRoot/>
```

判断：

```text
当前不能把 Detail.xml 当成主要工程图数据源。
TS_RS P0 Writer 应优先生成同样的空 StyleRoot 占位文件。
```

### Detail01.stl

观察：

```text
root = DrawingRoot
包含 StbTables / HViewPorts
包含 StbTable / MaterialTable
包含 PartDetailDrawing
包含 StbDetailDrawing / StbGroups
含 section-line / StbGroup / StbGeo / FaceEdge 等节点
```

### Detail02.stl 至 Detail04.stl

观察：

```text
root = DrawingRoot
包含 HViewPorts / ViewPort
Detail02 示例中 StbTables 为空容器
仍包含 PartDetailDrawing 与 StbDetailDrawing / StbGroups
```

初步结论：

```text
Detail01 可能承担主图 / 总表。
Detail02+ 可能承担分图 / 局部图。
不能假设每个 DetailNN 都有 StbTable / MaterialTable。
```

## 已观察到的关键节点

```text
DrawingRoot
StbTables
StbTable
StbRow#
StbSeg#
MaterialTable
MatRow#
HViewPorts
ViewPort
PartDetailDrawing
General-Info
continue-line
hidden-line
central-line
section-line
hatch-line
Others
steeljoint-line
StbDetailDrawing
StbGroups
StbGroup#
Std#
StbGeo#
FaceEdge
Line
```

## P0 Reader / Writer 顺序

```text
TODO-022 DetailPackageReader P0
  -> 只读旧包，统计节点和关键字段。
  -> 保留 rawXml / 原文件名 / sheetIndex。
  -> 不创建 DrawingModel。
  -> 不创建 RebarModel。
  -> 不写新 Detail 包。

TODO-023 DetailPackageWriter round-trip
  -> Reader -> Writer 保守回写，未知节点 / 未知属性不丢。

TODO-024 极简 Detail 包生成
  -> 生成 Detail.xml + Detail01.stl XML 一条 section-line。
  -> 人工 CAD 插件 autoin 验证。
```

## 禁止事项

```text
不要把 DetailNN.stl 当标准 STL mesh 读取。
不要用 Detail 字段反向污染 RebarModel。
不要在未 round-trip 前直接写复杂 DrawingModel。
不要删除未知节点 / 未知属性。
不要声称 Detail 兼容已成立，直到 autoin 人工验证通过。
```

## 待确认缺口

```text
GAP-DETAIL-001：CAD 插件是否要求 Detail.xml 必须存在，即使内容为空。
GAP-DETAIL-002：DetailNN.stl 文件数量和图纸数量如何对应。
GAP-DETAIL-003：StbTable / MaterialTable 是否只应写主图 Detail01。
GAP-DETAIL-004：未知节点 / 属性的顺序是否敏感。
GAP-DETAIL-005：AutoCAD 插件对极简 section-line 包的最低字段要求。
GAP-DETAIL-006：下料表.xls 与 DetailNN.stl 中 StbTables 是否同源同事务。
```
