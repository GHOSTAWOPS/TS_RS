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
Detail01.stl = DrawingRoot + HViewPorts + PartDetailDrawing + section-line/Line1
```

当前 SHA256：

```text
Detail.xml
  EA2FB44459C41800DF2251676DF6A72FB7B205FE7EB859925BA1AA2AFECEFFC0

Detail01.stl
  6A1FB5CBC165F1F26D3D8BE5D91E3F937CCF446109EDB84953F724B69744932D
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
3. 执行 autoin。
4. 选择本目录下 minimal_detail_package。
5. 观察是否成功导入。
6. 截图保存到本目录。
7. 回填下方验证记录。
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
