# TODO-024 xhigh 只读审查记录

## 审查元数据

```text
审查类型：xhigh internal read-only review
审查时间：2026-06-17
审查节点：TODO-024 极简 Detail 包生成
审查对象：工作区未提交 diff
子代理：todo024_detail_minimal_reviewer
结论：Conditional
```

## 审查结论

```text
Critical：无
Important：4
Minor：2
```

边界判断：

```text
代码侧未发现接入 Viewer、RebarModel、DrawingModel、钢筋生成器、
OCCT/AIS/TopoDS、VisualTS/RebarSmart DLL。

todo.csv 当前无 next，TODO-024=blocked，TODO-020D=pending，
符合“不推进下一个 next”。
```

## Critical

```text
无。
```

## Important 处理清单

### IR-001 README 是否越界

审查意见：

```text
README.md 发生未提交修改，但本节点允许边界只列了 Detail writer、writer 测试、docs 中 TODO-024/GC-004/todo 状态记录。
```

处理状态：

```text
rebutted
```

处理说明：

```text
README 是项目入口状态说明。
TODO-024 将 todo.csv 从 next 改为 blocked 后，如果 README 仍写旧 current next，
会导致后续 agent 和外部评审读取错误状态。
因此本次 README 修改属于 TODO 状态同步，不属于功能越界。
```

### IR-002 长期 goal 旧 current next 残留

审查意见：

```text
docs/roadmap/01_长期Goal执行目标.md 仍有“当前 next：TODO-023”残留。
```

处理状态：

```text
fixed
```

处理说明：

```text
已把旧块改为“当前状态”和“当前阻塞节点”。
长期 goal 现在明确：当前没有 next，TODO-024 blocked，需先完成 GC-004 autoin 人工验证。
```

### IR-003 非有限坐标只测 NaN

审查意见：

```text
测试只覆盖 quiet_NaN，未覆盖 +Inf / -Inf。
```

处理状态：

```text
fixed
```

处理说明：

```text
已补 +Inf / -Inf 测试。
```

### IR-004 数字格式依赖全局 locale

审查意见：

```text
formatDetailNumber 使用默认 ostringstream locale。
全局 locale 若变为小数逗号，Detail 坐标可能输出 110,5。
```

处理状态：

```text
fixed
```

处理说明：

```text
已在 formatDetailNumber 中使用 std::locale::classic()。
已补自定义小数逗号全局 locale 回归测试，不依赖系统是否安装特定区域设置。
```

## Minor 处理清单

### IR-005 XML 转义测试不完整

审查意见：

```text
测试只验证 < 和 &，未覆盖 >、双引号、单引号。
```

处理状态：

```text
fixed
```

处理说明：

```text
已补 drawingName 中 >、双引号、单引号的转义断言。
```

### IR-006 非空目标目录旧文件行为

审查意见：

```text
非空目标目录含旧 Detail02.stl 时，Reader validation 会失败。
这是安全行为，但可补测试锁住。
```

处理状态：

```text
deferred
```

处理说明：

```text
当前 TODO-024 P0 只要求生成极简包和 GC-004 验证输入。
正式事务写入和目标目录清理策略已在 docs/detail/02_Detail_roundtrip_policy.md 标记为后续节点。
本项不阻塞当前代码侧交付。
```

## 复验

修复后已运行：

```powershell
cmd /c 'call "D:\Visual Studio 2026\Community\VC\Auxiliary\Build\vcvars64.bat" >nul && cmake --build build/default --target tsrs_detail_package_writer_tests --config Debug && ctest --test-dir build/default -C Debug -R tsrs_detail_package_writer --output-on-failure'
```

结果：

```text
2/2 passed
```

提交前默认 CTest 已再次运行：

```powershell
cmd /c 'call "D:\Visual Studio 2026\Community\VC\Auxiliary\Build\vcvars64.bat" >nul && ctest --test-dir build/default -C Debug --output-on-failure'
```

结果：

```text
22/22 passed
```
