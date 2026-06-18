# GC-004 极简 Detail autoin

## 目标

验证 TODO-024 的最小 Detail 兼容出口：

```text
TS_RS 极简 Detail 包
  -> 旧 AutoCAD 插件 autoin
  -> 能导入并显示一条 section-line 或给出明确错误
```

本 case 只验证旧 CAD 插件对极简 Detail 包的最低容忍度。

不验证：

```text
完整工程图。
钢筋组 / 下料表。
DrawingModel / RebarModel 映射。
真实剖切、投影、标注。
```

## 输入包

```text
docs/validation/golden_cases/GC-004/minimal_detail_package/Detail.xml
docs/validation/golden_cases/GC-004/minimal_detail_package/Detail01.stl
```

包内容：

```text
Detail.xml = <StyleRoot/>
Detail01.stl = DrawingRoot + HViewPorts + PartDetailDrawing + General-Info ExportYesNo="T" + section-line/Line1
```

当前 SHA256：

```text
Detail.xml
  EA2FB44459C41800DF2251676DF6A72FB7B205FE7EB859925BA1AA2AFECEFFC0

Detail01.stl
  410F70E1EB77C0669933309DC13A4840649A98CB95CFD6F32BA03447725467DC
```

注意：

```text
GC-004 当前是 writer regression fixture, not compatibility proof。
它只能锁定 TS_RS 当前 Writer 输出形状。
旧 CAD 插件兼容性只能由 autoin 人工验证记录证明。
```

## autoin 逆向证据摘要

2026-06-18 已用 IDA MCP 复核旧 AutoCAD 图石插件：

```text
FDrawing.arx / FDrawing.arx.i64
IDA MCP session = fdrawing_arx
```

已确认：

```text
1. autoin 主逻辑在 sub_180556580。
2. 普通 autoin 默认读取 %TEMP%\msohtmplcllip。
3. 按住 Ctrl + Shift 执行 autoin 会弹出目录选择器。
4. 主入口枚举 selected_or_temp_dir\*.stl。
5. 每个 .stl 进入 sub_180585B30 做 XML 初筛。
6. 初筛链要求：
   DrawingRoot / HViewPorts / ViewPort / PartDetailDrawing / General-Info。
7. 初筛读取 General-Info.ExportYesNo。
8. 真实 todo66 Detail01..04 均为 ExportYesNo="T"。
```

详细记录：

```text
docs/validation/golden_cases/GC-004/cad_plugin_autoin_path_probe_20260618.md
```

## 代码生成入口

对应实现：

```text
app/src/drawing/detail/DetailPackageWriter.h
app/src/drawing/detail/DetailPackageWriter.cpp
```

对应 API：

```text
DetailPackageWriter::writeMinimalSectionLinePackage
```

对应测试：

```text
tsrs_detail_package_writer_tests
```

自动验证覆盖：

```text
1. 该目录是可读取的两文件 Detail 包。
2. Detail01.stl 统计到 1 条 section-line。
3. 该目录内容与 writeMinimalSectionLinePackage 同参数输出字节一致。
```

## 人工验证步骤

在安装了旧图石 AutoCAD 插件的机器上执行：

```text
1. 打开 AutoCAD。
2. 加载 / 确认旧图石 CAD 插件可用。
3. 按住 Ctrl + Shift 执行 autoin / 点击导入按钮。
4. 如果弹出目录选择器，选择本目录下 minimal_detail_package。
5. 观察是否成功导入。
6. 截图保存到本目录。
7. 回填下方验证记录。
```

备选：

```text
1. 创建并清空 %TEMP%\msohtmplcllip。
2. 将 minimal_detail_package 内的 Detail01.stl 复制进去。
3. 可同时复制 Detail.xml，但当前主入口证据显示 autoin 先枚举 *.stl。
4. 正常执行 autoin。
```

## 验证记录

当前状态：

```text
blocked_waiting_manual_autoin
```

原因：

```text
当前 Codex 环境无法直接执行用户 AutoCAD 插件 autoin。
```

待回填：

```text
验证日期：
验证人：
机器：
AutoCAD 版本：
旧图石 CAD 插件版本 / 来源：
autoin 命令是否可用：
导入结果：pass / fail
可见结果：
错误提示：
截图文件：
输出文件：
备注：
```

## 判定规则

通过：

```text
CAD 插件能导入该目录。
AutoCAD 中能看到一条 section-line 或等价最小图元。
```

失败但可继续：

```text
CAD 插件给出明确错误。
记录错误后，下一轮只补最小缺失字段。
```

失败且阻塞：

```text
autoin 命令不可用。
CAD 插件未安装。
CAD 插件启动依赖缺失。
```

## 当前结论

```text
代码侧极简包生成已具备验证输入。
旧 AutoCAD 插件兼容尚未闭合。
TODO-024 在人工 autoin 通过前不能作为 Detail 兼容完成证据。
```
