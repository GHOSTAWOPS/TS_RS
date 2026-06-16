# 老图石命令到 RebarSmart 生成器映射表

## 目的

本表解决一个核心问题：

```text
外壳像老图石；
钢筋算法按 RebarSmart；
两者之间必须有明确命令映射。
```

没有这张表，后续 UI 容易变成“两边都不像”。

## P0 映射口径

| 老图石入口 | TS_RS 命令 | RebarSmart 证据 / Generator | P0 状态 | 说明 |
|---|---|---|---|---|
| 线配筋 | Rebar.Create.FixDistance | FixDistance / GenerateRebarFixDistance | P0A 候选 | 优先做最小闭环之一 |
| 线配筋 / 定数语义 | Rebar.Create.FixNumber | FixNumber / GenerateRebarFixNumber | P0B 候选 | 不和 FixDistance 同时抢 P0A |
| 面配筋 | Rebar.Create.SurfaceDistribution | FixDistance/FixNumber + GuideSurface | P1 | 需要 TopologyBinding 和面偏移证据 |
| 自配筋 | Rebar.Create.Customize | Customize | P1/P2 | RebarSmart 自定义钢筋证据后置 |
| 箍筋 | Rebar.Create.Stirrup | Stirrup | P1 | 需要闭合线 / 箍筋参数证据 |
| 插筋 | Rebar.Create.Dowel | Dowel | P1 | 点式布置、锚固规则需确认 |
| 拉结筋 | Rebar.Create.Tie | Tie | P1 | 连接两层/两组钢筋逻辑需确认 |
| 螺旋筋 | Rebar.Create.Helix | Helix | P2 | 半径、螺距、圈数和空间路径复杂 |
| 角筋 | Rebar.Create.Corner | Corner | P2 | 几何条件和排序规则后置 |
| 柱锥环盘面筋 | Rebar.Create.Frustum | Frustum | P2 | 锥面/环盘几何复杂 |
| 钢筋移动 | Rebar.Edit.Move | RebarModel edit service | P1 | 通过钢筋树右键进入 |
| 钢筋复制 | Rebar.Edit.Copy | RebarModel edit service | P1 | 平移/对称/旋转复制后置 |
| 钢筋显示/隐藏 | Rebar.View.Visibility | Viewer + RebarModel state | P1 | P0 可只保留入口 |
| 查询/统计 | Rebar.Query.Schedule | ScheduleModel | P1/P2 | 查询页签 P0 保留空壳 |

## P0 收缩规则

P0 不再追求 8 个钢筋创建命令全部闭合。

P0A 只追求一个最小闭环：

```text
STEP 导入
Viewer 显示
选择边/面
TopologyBinding 保存/恢复
FixDistance 或 FixNumber 二选一
RebarModel
极简 Detail 包
CAD 插件 autoin 验证
```

其余命令：

```text
按钮可存在。
状态为 placeholder / disabled / P1/P2。
不能作为 P0 验收项。
```

## 后续补证据

每个映射项后续必须补：

```text
输入对象类型
参数面板来源
默认值来源
单位和枚举来源
生成器职责
Detail 字段影响
P0/P1/P2 状态
GAP 编号
```
