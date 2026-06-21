# GC-004 CAD 插件 autoin 路径与初筛字段逆向记录

## 结论

本记录只关闭两个问题：

```text
1. 旧 AutoCAD 图石插件 autoin 从哪里读取 DetailNN.stl。
2. autoin 对每个 DetailNN.stl 的第一层 XML 初筛字段是什么。
```

它本身只证明路径和初筛字段。
后续 2026-06-21 人工验证已证明 v2_empty_groups minimal sheet
可被旧图石 AutoCAD 插件导入按钮路径接受。

TODO-024 当前状态：

```text
manual_autoin_passed_v2
```

## 逆向对象

```text
文件：
  C:\Users\ghost\Desktop\reverse_engineering\autocad2020\FDrawing.arx

IDA database:
  C:\Users\ghost\Desktop\reverse_engineering\autocad2020\FDrawing.arx.i64

IDA MCP session:
  fdrawing_arx
```

## autoin 命令入口

命令表项：

```text
ARXCOMMA 0x1808762a0 -> 0x180786be8

group  = CWSNSteel
global = _CWSNImportDirAuto
local  = autoin
flags  = 0x1
handler = 0x180005961
```

调用链：

```text
autoin / _CWSNImportDirAuto
  -> 0x180005961
  -> sub_180524CC0()
  -> sub_1800066A9(qword_1807C4FE0)
  -> sub_180556580()
```

`sub_180556580` 是 CAD 插件导入主逻辑。

## 路径来源

`sub_180556580` 中存在两个路径来源。

### Ctrl + Shift 路径选择

按键检测：

```text
0x180556741  GetAsyncKeyState(0x11)  // VK_CONTROL
0x180556755  GetAsyncKeyState(0x10)  // VK_SHIFT
```

同时按住 Ctrl + Shift 执行 `autoin` 时：

```text
0x1805567b3  SHBrowseForFolderW
0x1805567cc  SHGetPathFromIDListW
```

结论：

```text
按住 Ctrl + Shift 执行 autoin 可以弹出目录选择器。
这是 TODO-024 推荐人工验证方式。
```

### 默认临时目录

未按 Ctrl + Shift 时：

```text
0x1805567e7  GetTempPathW
0x180556802  "msohtmplcllip"
```

结论：

```text
默认读取：
  %TEMP%\msohtmplcllip

注意拼写是 msohtmplcllip。
不要和系统中常见的 msohtmlclip / msohtmlclip1 混淆。
```

## 文件枚举

`sub_180556580` 拼接并枚举：

```text
selected_or_temp_dir\*.stl
```

关键地址：

```text
0x1805568bb  "\\*.stl"
0x180556904  CFileFind::FindFile
0x1805569a7  CFileFind::FindNextFileW
0x1805569f1  CFileFind::GetFilePath
```

每个 `.stl` 路径之后进入：

```text
0x180556a54  sub_180004A0C
sub_180004A0C -> sub_180585B30
```

结论：

```text
autoin 主入口先枚举目录中的 *.stl。
当前没有证据表明它先按固定路径读取 Detail.xml。
Detail.xml 是否参与其他样式逻辑仍是 GAP，不能说完全不用。
```

## DetailNN.stl 初筛 XML 链

`sub_180585B30` 使用 COM XML DOM 读取 `.stl` XML。

它按顺序查找：

```text
DrawingRoot
  HViewPorts
    ViewPort
      PartDetailDrawing
        General-Info
```

关键地址：

```text
0x180585bfd  "DrawingRoot"
0x180585c38  "HViewPorts"
0x180585c72  "ViewPort"
0x180585ca9  "PartDetailDrawing"
0x180585ce0  "General-Info"
```

随后读取 `General-Info` 上的：

```text
ExportYesNo
```

关键地址：

```text
0x180585d26  "ExportYesNo"
0x180585dc9  读取常量 "F"
0x180585e3d  test edi, edi
0x180585e3f  setnz r15b
```

控制流含义：

```text
ExportYesNo == "F" 时，返回 0。
ExportYesNo != "F" 时，返回 1。
```

真实 todo66 样本中 `Detail01.stl` 到 `Detail04.stl` 均为：

```text
ExportYesNo="T"
```

因此 GC-004 极简包采用：

```text
General-Info ExportYesNo="T"
```

## 人工验证建议

推荐方式：

```text
1. 准备干净目录：
   docs/validation/golden_cases/GC-004/minimal_detail_package

2. 目录中只保留：
   Detail.xml
   Detail01.stl

3. 打开 AutoCAD，并确认旧图石插件已加载。

4. 按住 Ctrl + Shift 执行 autoin / 点击导入按钮。

5. 如果弹出目录选择器，选择 minimal_detail_package 目录。

6. 记录结果：
   pass / 明确错误 / 崩溃。
```

备选方式：

```text
1. 创建 %TEMP%\msohtmplcllip。
2. 清空其中旧 *.stl。
3. 复制 GC-004 minimal_detail_package 下的 Detail01.stl。
4. 可同时复制 Detail.xml，但当前主入口证据显示 autoin 先枚举 *.stl。
5. 正常执行 autoin。
```

## 用户运行观察

用户曾将 `Detail01.stl` 放入 `%TEMP%\msohtmplcllip` 后触发 AutoCAD 致命错误。

该观察不能直接判定为 TS_RS Writer 已失败，原因：

```text
1. 当时的 GC-004 版本缺少 ExportYesNo。
2. 目录中是否混有其他 *.stl 未完全确认。
3. 极简 XML 仍可能缺少 CAD 插件后续创建图元需要的字段。
```

后续已使用 v2_empty_groups / v3_one_pointstb 重新验证。
v2_empty_groups 作为 TODO-024 P0 baseline。

## 当前仍未关闭的 GAP

```text
GAP-GC004-AUTOIN-001：
  v2_empty_groups 已证明 CAD 插件 minimal sheet 路径接受
  StbTables / StbDetailDrawing / 完整 General-Info 组合；
  完整 StbGroup / 表格仍未验证。

GAP-GC004-AUTOIN-002：
  Detail.xml 是否在其他样式路径中被读取。

GAP-GC004-AUTOIN-003：
  v2_empty_groups Test C 已确认 v2 图元被读取 / 显示。
```
