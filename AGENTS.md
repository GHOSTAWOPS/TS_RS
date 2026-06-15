# TS_RS Agent 工作规范

本目录是独立项目，不再沿用父目录旧 VisualTS 1:1 逆向复刻路线。

## 核心路线

```text
STEP/STP only 输入。
Qt6 负责独立桌面 UI。
OCCT 负责 STEP 导入、三维显示、选择、几何计算和实体化。
RebarSmart 证据驱动钢筋生成逻辑复刻。
老图石 / VisualTS 只作为 Detail 包、工程图字段、CAD 插件兼容证据源。
```

## 禁止事项

```text
不要调用 RebarSmart DLL。
不要调用 VisualTS EXE / DLL。
不要依赖 3DE / CAA / CATIA / ACIS / HOOPS / Codejock 商业运行时。
不要迁入旧实现项目【图石钢筋1比1复刻】里的 rebar 业务代码作为 TS_RS 主线。
不要让 domain/rebar 依赖 TopoDS / AIS / BRep 等 OCCT 细节。
不要用“OCCT 能做到什么”替代“RebarSmart 参数和算法证据是什么”。
```

## 旧实现项目参考边界

```text
旧实现项目：
  C:\Users\ghost\Desktop\reverse_engineering\【03】图石软件\【图石钢筋1比1复刻】

新项目：
  C:\Users\ghost\Desktop\reverse_engineering\【03】图石软件\【TS_RS】
```

允许参考旧实现项目：

```text
CMake 查找 Qt / OCCT。
STEP / STP 导入。
AIS Viewer。
选择 / 高亮。
XCAF / TopoDS 遍历。
剖切 / 颜色 / 稳定 ID 思路。
```

禁止迁入旧实现项目：

```text
直接用 OCCT 生成钢筋的 rebar 业务逻辑。
EdgeToRebarFactory。
FaceRebarGenerator。
PolylineRebarGenerator。
RebarCreationCommandService 中直接造钢筋的业务流程。
```

## 执行规则

```text
每轮只推进 todo.csv 中 status=next 的一个节点。
先读 todo.csv，再读该任务 evidence 指向的文档。
每轮讨论和决策都要落文档。
废弃草案不要保留在当前开发文档里。
不确定的 RebarSmart 行为优先用 IDA MCP / PDF / 实际运行确认。
字段、单位、枚举、算法低置信时必须标 GAP。
```

## 代码阶段规则

```text
先补测试，再实现。
涉及代码、测试、构建脚本的节点，commit 前必须做 xhigh 只读 review。
xhigh 只 review，不修改；修改由主流程完成。
子代理完成后关闭已完成且不再需要的代理，减少代理负担。
```

## Git 规则

```text
每完成一个清晰节点就 commit。
重要节点打 annotated tag，形成可回退时间线。
commit message 写清楚 TODO 编号和交付物。
```
