# Golden Case 策略

本文档按外部技术评审意见补齐 TS_RS 的验证策略。

## 结论

TS_RS 后续不能只靠单元测试判断“复刻成功”。

必须逐步建立 golden case：

```text
真实 STEP / STP 样本
RebarSmart 参数输入
TS_RS 生成结果
Detail 包输出
AutoCAD 插件 autoin 人工验证结果
```

P0A 阶段只要求极少量 golden case，但必须覆盖真实链路。

## 为什么需要 golden

本项目有三类风险，普通单测挡不住：

1. OCCT 几何结果和 3DE / CAA 可能有容差差异。
2. RebarSmart 单位 / 枚举 / 默认值可能理解错。
3. Detail 包可能 XML 合法但 CAD 插件不接受。

所以验证必须分层：

```text
L0：纯算法单测
L1：STEP / OCCT / generator 集成测试
L2：真实样本 + Detail 包 + CAD 插件人工验证
```

## Golden Case 目录建议

后续建议使用：

```text
docs/validation/golden_cases/
  GC-001/
    README.md
    input.json
    expected_summary.json
    actual_summary.json
    verification.md
```

注意：

```text
大型 STEP / STP 样本默认不进 git。
只记录文件名、大小、hash、来源和本机路径。
```

## Golden Case 元数据

每个 case 至少记录：

```text
caseId
sourceStepFile
sourceStepSha256
stepFileSize
selectedBindingIds
generatorType
inputParameters
expectedRebarCount
expectedCenterlineLengthRange
expectedScheduleSummary
detailPackageExpectedFiles
autoinVerificationStatus
evidenceLevel
knownGaps
```

## P0A 最小 case

### GC-001：真实 STEP 打开和水泥灰显示

目标：

```text
证明 TS_RS 可以打开真实 STEP / STP，
并按 UI 规则统一水泥灰显示。
```

对应 todo：

```text
TODO-020
```

验收：

```text
导入成功。
shape / face / edge / vertex 数量记录。
Viewer 显示使用 cement-gray，不使用 STEP 原色。
```

### GC-002：TopologyBinding 稳定性

目标：

```text
同一 STEP 多次导入后，
关键 face / edge / vertex fingerprint 稳定。
```

对应 todo：

```text
TODO-021 或后续 TopologyBinding 实现节点。
```

验收：

```text
同一文件导入 5 次。
保存 selectedFaceId / selectedEdgeId。
重开后恢复到同类拓扑对象。
fingerprint 和 bbox / length 在容差内。
```

### GC-003：FixDistance 中心线生成

目标：

```text
从真实选择对象生成一组定距中心线。
```

注意：

```text
P0 只验收中心线，不验收完整投影、过孔、弯钩、实体化。
```

### GC-004：极简 Detail autoin

目标：

```text
生成 Detail.xml + Detail01.stl XML，
人工用老图石 CAD 插件 autoin 导入。
```

对应 todo：

```text
TODO-024
```

验收：

```text
CAD 插件能导入。
能看到一条 section-line 或最小图元。
记录 AutoCAD / 插件版本、操作步骤、截图、输出文件 hash。
```

## 容差规则

P0 建议：

```text
长度容差：1e-6 m 到 1e-4 m，按样本复杂度记录。
点坐标容差：1e-6 m 到 1e-4 m。
数量断言必须精确。
枚举值未确认时只断言 raw value，不断言中文名。
```

## 禁止事项

```text
不要把 Mock 几何 case 当成真实 golden。
不要没有 CAD 插件验证就宣称 Detail 兼容完成。
不要把旧项目【图石钢筋1比1复刻】生成结果当 TS_RS golden 真相。
不要把未确认 RebarSmart 行为写成 expected。
```

