# TS_RS

TS_RS 是新的独立钢筋建模程序方向。

核心路线：

```text
Qt6 做独立桌面 UI。
OCCT 做 STEP/STP 导入、三维显示、选择和几何计算。
RebarSmart 作为钢筋生成逻辑的证据源。
老图石 / VisualTS 只作为 Detail 包、工程图字段、CAD 插件兼容证据源。
```

硬边界：

```text
不调用 RebarSmart DLL。
不调用 VisualTS EXE / DLL。
不依赖 3DE / CAA / CATIA / ACIS / HOOPS / Codejock 商业运行时。
不迁入父目录旧 rebar 业务实现作为主线。
```

当前阶段：

```text
TODO-020 已完成：真实 STEP/STP 导入与水泥灰显示。
TODO-020B 已完成：ShapeStore + TopologyBindingRegistry P0 library。
TODO-020D 已完成：TopologyBindingRegistry endpoint canonicalization +
geometryFingerprint fallback + low-confidence guard。
TODO-020E 已完成：StepSession / ImportedModelStore 导入会话主链路。
TODO-020G 已完成：CommandService skeleton guardrails。
TODO-021 已完成：Viewer selection mode 和 stable binding smoke 出口。
TODO-020F 已完成：STEP import unit and scale policy。
TODO-022 已完成：DetailPackageReader P0。
TODO-023 已完成：DetailPackageWriter preserve-mode round-trip。
TODO-024 代码侧已完成：极简 Detail 包和 GC-004 固定验证输入。

P0A 最小真实闭环尚未放行。
Detail 兼容尚未放行，TODO-024 只证明 minimal sheet format compatibility。
当前 next：TODO-026 .tsrebar minimal save/open。
TODO-025 已完成 RebarModel minimal transaction。
domain/rebar 必须继续保持不依赖 OCCT / AIS 细节。
```

关键入口：

```text
docs/adr/ADR-0001_STEP_only_RebarSmart_Detail路线.md
docs/architecture/00_系统架构总览.md
docs/architecture/01_纯算法层实现计划.md
docs/architecture/02_TODO-006_app工程骨架实现记录.md
docs/roadmap/00_路线图.md
docs/roadmap/01_长期Goal执行目标.md
docs/external_reviews/20260617_after_TODO020B_conditional_go.md
docs/rebarsmart/00_证据索引.md
docs/rebarsmart/01_GenerateRebarData字段地图.md
docs/validation/fixtures/detail_todo66_manifest.md
todo.csv
```
