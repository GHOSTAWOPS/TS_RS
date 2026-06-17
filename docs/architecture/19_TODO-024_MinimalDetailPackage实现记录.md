# TODO-024 极简 Detail 包生成实现记录

## 结论

TODO-024 代码侧已实现极简 Detail 包生成：

```text
DetailMinimalSectionLinePackage
  -> DetailPackageWriter::writeMinimalSectionLinePackage
  -> Detail.xml
  -> Detail01.stl
  -> DetailPackageReader 重新读取校验
```

本节点只证明：

```text
TS_RS 可以生成一个 XML well-formed 的极简 Detail 包。
Detail.xml 固定写 <StyleRoot/>。
Detail01.stl root = DrawingRoot。
Detail01.stl 至少包含 HViewPorts / ViewPort / PartDetailDrawing / section-line。
Reader 能重新读回，knownSummary 统计到 1 个 ViewPort、1 个 PartDetailDrawing、1 条 section-line。
```

本节点尚未证明：

```text
旧 AutoCAD 插件 autoin 已接受该极简包。
Detail 包协议最低必填字段已完全闭合。
DrawingModel / RebarModel 到 Detail 包的正式映射已完成。
钢筋生成器、Viewer 选择系统或工程图标注已经接入 Detail。
```

## 实现内容

更新：

```text
app/src/drawing/detail/DetailPackageWriter.h
app/src/drawing/detail/DetailPackageWriter.cpp
app/tests/drawing/detail_package_writer_tests.cpp
```

新增公开 DTO：

```text
DetailSectionLine2d
  startX
  startY
  endX
  endY

DetailMinimalSectionLinePackage
  drawingName
  sectionLine
```

新增 Writer 方法：

```text
DetailPackageWriter::writeMinimalSectionLinePackage(
    const DetailMinimalSectionLinePackage& package,
    const std::string& targetDirectory)
```

## P0 行为

`writeMinimalSectionLinePackage` 当前执行：

```text
1. 检查 section-line 四个坐标均为 finite number。
2. 创建输出目录。
3. 写 Detail.xml = <StyleRoot/>。
4. 写 Detail01.stl = DrawingRoot + HViewPorts + section-line。
5. 用 DetailPackageReader 重新读取输出目录。
6. 校验文件数为 2。
7. 校验 Detail01.stl 包含：
   - viewPortCount = 1
   - partDetailDrawingCount = 1
   - sectionLineCount = 1
8. 失败时返回 DETAIL_WRITE_FAILED 或 DETAIL_WRITE_VALIDATE_FAILED。
```

当前极简 `Detail01.stl` 包含：

```text
DrawingRoot
  HViewPorts
    ViewPort
      PartDetailDrawing
        General-Info
        continue-line
        hidden-line
        central-line
        section-line / lines / Line1
        hatch-line
        Others
        steeljoint-line
      StbDetailDrawing / StbGroups
```

## 测试覆盖

新增测试覆盖：

```text
1. minimal package 写出 2 个文件。
2. Detail.xml 字节内容为 <StyleRoot/>\n。
3. Detail01.stl 含 DrawingRoot / section-line / Line1。
4. drawingName 中 XML 特殊字符会转义。
5. Reader 能重新读回生成包。
6. knownSummary 能统计 1 个 viewport、1 个 part drawing、1 条 section-line。
7. NaN / Inf 坐标被拒绝。
8. 坐标非法时不写 Detail.xml / Detail01.stl。
9. GC-004 固定验证包可被 Reader 读取。
10. GC-004 固定验证包和 Writer 同参数输出字节一致。
```

## RED / GREEN 记录

RED：

```text
cmake --build build/default --target tsrs_detail_package_writer_tests --config Debug
```

失败原因符合预期：

```text
DetailMinimalSectionLinePackage 不是 tsrs::detail 的成员
writeMinimalSectionLinePackage 不是 DetailPackageWriter 的成员
```

GREEN：

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

## GC-004 固定人工验证包

已新增：

```text
docs/validation/golden_cases/GC-004/README.md
docs/validation/golden_cases/GC-004/minimal_detail_package/Detail.xml
docs/validation/golden_cases/GC-004/minimal_detail_package/Detail01.stl
```

用途：

```text
给旧 AutoCAD 插件 autoin 人工验证使用。
```

SHA256：

```text
Detail.xml
  EA2FB44459C41800DF2251676DF6A72FB7B205FE7EB859925BA1AA2AFECEFFC0

Detail01.stl
  6A1FB5CBC165F1F26D3D8BE5D91E3F937CCF446109EDB84953F724B69744932D
```

当前状态：

```text
autoin 尚未人工执行。
不能宣称 Detail 兼容已闭合。
```

## 边界检查

本节点没有：

```text
接 Viewer。
接选择系统。
接钢筋生成器。
创建 DrawingModel。
创建 RebarModel。
调用 VisualTS / RebarSmart DLL / 3DE / CAA。
把 Detail 字段写入 RebarModel。
```

## 当前 GAP

| ID | 缺口 | 处理 |
| --- | --- | --- |
| TODO024-GAP-001 | 旧 AutoCAD 插件 autoin 尚未人工验证。 | 需要用户在 AutoCAD 插件环境中验证 GC-004 包。 |
| TODO024-GAP-002 | 极简包最低字段是否足够未知。 | autoin 失败后按错误补最小字段，不一次性复制真实复杂包。 |
| TODO024-GAP-003 | `General-Info` 当前只写少数字段。 | 仅服务 P0 极简验证，不能作为正式工程图字段映射。 |
| TODO024-GAP-004 | `StbTables` 当前未写。 | 如果 CAD 插件要求该节点，下一轮按 autoin 结果补。 |

## 下一步

如果 GC-004 autoin 人工验证通过：

```text
1. 回填 GC-004 README。
2. 将 TODO-024 从 blocked 改为 done。
3. 推进 TODO-020D。
```

如果 autoin 失败：

```text
1. 记录错误提示、截图、AutoCAD / 插件版本。
2. 对比真实 todo66 Detail01.stl。
3. 只补最小必填节点 / 属性。
4. 重新生成 GC-004 包并再次验证。
```
