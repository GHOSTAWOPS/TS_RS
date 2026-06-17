# ApplicationService / CommandService 边界

本文档按外部技术评审意见补齐 TS_RS 的命令调用链边界。

## 结论

TS_RS 的 UI 不能直接调用 generator 改 RebarModel。

所有可提交业务动作必须经过：

```text
UI Command Handler
  -> Application Service / CommandService
  -> RebarSmart-style Generator
  -> RebarModel Transaction
  -> Preview / Commit / Rollback
```

这条链路是 P0A 后续开发的强制边界。

## 为什么要单独冻结

外部评审指出：

```text
如果不写死调用链，
后续容易在 UI 面板里直接 new Generator，
或者直接把 Viewer 选择对象写进 RebarModel。
```

这种写法短期快，但会造成三类问题：

1. UI 和钢筋算法耦合，后续很难测试。
2. RebarModel 被 AIS / TopoDS / QWidget 污染。
3. 预览、取消、撤销、保存 dirty 状态没有统一事务边界。

## 模块职责

### UI Command Handler

职责：

```text
响应 Ribbon / 菜单 / 右侧 Dock / 钢筋树右键事件。
读取当前面板字段。
读取当前选择状态。
把用户动作转换为 CommandRequest。
显示诊断和预览状态。
```

禁止：

```text
直接 new FixDistanceGenerator / FixNumberGenerator。
直接修改 RebarModel。
直接保存 TopoDS / AIS 指针。
直接解析 RebarSmart INI / CATNls。
直接写 Detail 包。
```

### Application Service / CommandService

职责：

```text
校验命令前置条件。
把 UI 参数 DTO 转成 generator input。
把 Viewer 选择转换为 TopologyBinding / GeometryRef。
调用 generator。
创建 RebarModel transaction。
管理 preview / apply / cancel。
生成统一 diagnostic。
```

P0 推荐服务：

```text
RebarCreationCommandService
StepImportCommandService
SelectionCommandService
DetailExportCommandService
```

注意：

```text
这些是 TS_RS 新服务名。
不要迁入旧实现项目【图石钢筋1比1复刻】里的 RebarCreationCommandService 业务逻辑。
旧项目同名或近似命名只能参考工程写法，不能作为钢筋业务真相。
```

### RebarSmart-style Generator

职责：

```text
按 RebarSmart 证据生成中心线 / 钢筋段 / 钢筋组的领域结果。
依赖 IGeometryEngine。
返回结构化 diagnostics。
```

禁止：

```text
依赖 QWidget / QAction / QDockWidget。
依赖 AIS_ / TopoDS_ / BRep / TopAbs / gp_ / Geom_。
读写 .tsrebar 文件。
写 Detail 包。
```

### RebarModel Transaction

职责：

```text
承载一次业务修改。
支持 preview result 和 committed result 分离。
维护 dirty 状态。
为后续 undo / redo 预留边界。
```

P0 可以先做最小事务：

```text
beginPreview(commandId)
replacePreview(rebarGroupDraft)
commitPreview()
cancelPreview()
```

但不允许 UI 绕过事务直接把生成结果塞进 RebarModel。

## P0A 命令链示例

### 定距中心线预览

```text
Ribbon: 钢筋 / 定距钢筋
  -> UI 打开右侧 FixDistance Dock
  -> 用户选择 Edge / Face
  -> SelectionCommandService 生成 TopologyBinding
  -> FixDistanceCommandHandler 收集参数
  -> RebarCreationCommandService.previewFixDistance(...)
  -> FixDistanceCenterlineGenerator
  -> RebarModelTransaction.replacePreview(...)
  -> presentation/occ 显示橙黄色预览
```

### 定距中心线应用

```text
用户点击应用
  -> RebarCreationCommandService.commitPreview()
  -> RebarModel 持久化新 RebarGroup
  -> ProjectDirtyState = dirty
  -> 工程树刷新
  -> 下料表基础摘要刷新
```

## 诊断策略

所有命令失败必须返回结构化 diagnostic：

```text
code
severity
message
source
relatedBindingId
evidenceGapId
```

禁止只在 UI 弹窗里写字符串后吞掉错误。

## 与当前已实现代码的关系

当前已完成：

```text
TODO-013 FixDistanceCenterlineGenerator P0
TODO-014 FixNumberCenterlineGenerator P0
```

它们仍是 generator 级能力，不等于 UI 命令链已经完成。

当前正在推进：

```text
TODO-020 STEP 导入与水泥灰显示
TODO-021 Viewer 选择系统
```

TODO-020 / TODO-021 完成前，不允许把真实 Viewer 选择直接接入 generator。

## Guardrail

后续代码 review 必须检查：

```text
ui/ 是否 include app/src/rebarsmart/generators 的具体实现头。
ui/ 是否直接修改 RebarModel。
rebarsmart/ 是否 include Qt / OCCT / AIS。
domain/rebar 是否出现 TopoDS_ / AIS_ / BRep / TopAbs / gp_ / Geom_。
CommandService 是否统一产生 diagnostics。
```

建议检查命令：

```powershell
rg -n "TopoDS_|AIS_|BRep|TopAbs_|gp_|Geom_" app/src/domain app/src/rebarsmart
rg -n "FixDistanceGenerator|FixNumberGenerator" app/src/ui
```

