# GC-001 真实 STEP 打开和水泥灰显示

## 目标

验证 TODO-020 的最小真实样本链路：

```text
真实 STEP / STP
  -> StepImportCommandService
  -> StepDisplayModel
  -> OCCT Viewer 水泥灰显示入口
```

本 case 只证明导入和显示入口可走通，不证明：

```text
TopologyBinding 稳定。
Viewer 子对象选择。
钢筋生成器接入。
Detail 包兼容。
```

## 输入样本

```text
文件：C:\Users\ghost\Desktop\reverse_engineering\【03】图石软件\123.stp
来源：用户从老图石导出的钢筋 STEP。
是否提交 git：否，文件较大且属于本地真实工程样本。
大小：87746388 bytes。
修改时间：2026-06-05 10:02:17。
SHA256：6A3C4F2044C2CC1F1123F0F58C61B2C869FBAD3110A3585FDAB49E69DAF52A2A
```

## 验证命令

### 可视化截图

截图文件：

```text
docs/validation/golden_cases/GC-001/tsrs_app_123stp_window_20260617.png
```

观察结果：

```text
TS_RS 主窗口成功打开。
中央 OCCT Viewer 显示 123.stp 模型。
消息栏显示：
  STEP 已导入：123.stp，faces=3016，edges=9048，vertices=18096
模型按统一浅灰 / 水泥灰风格显示，未使用 STEP 源文件颜色。
```

### 导入统计

```powershell
$env:PATH='D:\Work\vcpkg\installed\x64-windows\debug\bin;D:\Work\vcpkg\installed\x64-windows\bin;' + $env:PATH
$env:TSRS_LOCAL_STEP_SAMPLE='..\123.stp'
build\default\tsrs_step_import_local_sample_probe_tests.exe
```

结果：

```text
source=..\123.stp
rootCount=1
solidCount=754
faceCount=3016
edgeCount=9048
vertexCount=18096
```

### App smoke

```powershell
$env:QT_QPA_PLATFORM='minimal'
$env:QT_QPA_PLATFORM_PLUGIN_PATH='D:\Work\vcpkg\installed\x64-windows\debug\Qt6\plugins\platforms'
$env:PATH='D:\Work\vcpkg\installed\x64-windows\debug\bin;D:\Work\vcpkg\installed\x64-windows\bin;' + $env:PATH
build\default\tsrs_app.exe --smoke --step ..\123.stp
```

结果：

```text
exit code = 0
```

## 显示规则

TODO-020 的显示规则来自：

```text
app/src/presentation/style/StepModelDisplayStyle.h
app/src/presentation/style/StepModelDisplayStyle.cpp
```

当前固定为：

```text
semanticName = cement-gray
red = 0.56
green = 0.58
blue = 0.59
useSourceStepColor = false
```

对应测试：

```text
tsrs_step_model_display_style_tests
```

## 当前未闭合

| ID | 缺口 | 状态 |
| --- | --- | --- |
| GC001-GAP-001 | 当前 app smoke 使用 Qt minimal，不产生人工截图。 | 已补可视化截图 `tsrs_app_123stp_window_20260617.png`。 |
| GC001-GAP-002 | 当前没有自动检查屏幕像素是否真为水泥灰。 | 已有人眼截图确认；后续可用 viewer 截图/像素检查补自动 gate。 |
| GC001-GAP-003 | 当前只验证 `123.stp` 一个真实样本。 | 后续补结构 STEP / 更多工程样本。 |

## 结论

```text
GC-001 导入统计、app smoke 和可视化截图已通过。
TODO-020 已完成 xhigh 只读 review，Critical / Important 均无阻塞。
```
