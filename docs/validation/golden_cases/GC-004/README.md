# GC-004 Minimal Detail v2 Autoin

## 目标

验证 TODO-024 的最小 Detail 兼容出口：

```text
TS_RS v2 minimal sheet DetailNN.stl
  -> 旧图石 AutoCAD 插件导入按钮路径
  -> CAD 插件能读取 / 导入 / 显示最小 sheet 图元
```

本 case 只验证旧 CAD 插件对 minimal sheet 的最低容忍度。

不验证：

```text
完整工程图。
钢筋组。
StbTable / MaterialTable。
DrawingModel / RebarModel 映射。
真实剖切、投影、标注。
```

## 当前状态

```text
manual_autoin_passed_v2
```

done 含义：

```text
minimal sheet DetailNN.stl protocol reached manual autoin pass
```

done 不表示完整 Detail 兼容已经完成。

## 输入包

```text
docs/validation/golden_cases/GC-004/minimal_detail_package/Detail.xml
docs/validation/golden_cases/GC-004/minimal_detail_package/Detail01.stl
```

包内容：

```text
Detail.xml = <StyleRoot/>

Detail01.stl =
  DrawingRoot
  StbTables
  HViewPorts / ViewPort
  PartDetailDrawing / General-Info
  continue-line / hidden-line / central-line / section-line / hatch-line / steeljoint-line
  StbDetailDrawing / StbGroups stbGroupCount="0"
```

当前 SHA256：

```text
Detail.xml
  EA2FB44459C41800DF2251676DF6A72FB7B205FE7EB859925BA1AA2AFECEFFC0

Detail01.stl
  63ED30850AE2BFFA678DCFD8A2BD3DBA5F29C3DC30A9442763119EB5EA980E90
```

说明：

```text
Detail.xml 继续保留 <StyleRoot/>，用于普通 Detail 包目录兼容。
但本次按钮导入路径实际读取临时目录中的 DetailNN.stl。
Detail.xml 不是本次按钮路径的必要条件。
```

## 人工验证记录

验证环境：

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
  后续最小 StbGroup / pointStb 兼容样本
  不是 TODO-024 P0 baseline
```

分项测试：

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

## 代码生成入口

对应实现：

```text
app/src/drawing/detail/DetailPackageWriter.cpp
DetailPackageWriter::writeMinimalSectionLinePackage
```

对应测试：

```text
tsrs_detail_package_writer_tests
```

自动验证覆盖：

```text
1. 该目录是可读取的两文件 Detail 包。
2. Detail01.stl 包含 v2 baseline 节点。
3. Detail01.stl 包含 StbTables。
4. Detail01.stl 包含 StbDetailDrawing / StbGroups。
5. StbGroups 的 stbGroupCount 为 0。
6. section-line 至少有 1 条 Line。
7. 该目录内容与 writeMinimalSectionLinePackage 同参数输出字节一致。
```

## 历史逆向证据

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

## 当前结论

```text
TODO-024 可以标记 done。
P0 minimal sheet baseline 采用 v2_empty_groups。
v3_one_pointstb 仅作为后续最小钢筋组兼容样本。
完整 Detail / StbGroup / StbTable / MaterialTable 仍未完成。
```
