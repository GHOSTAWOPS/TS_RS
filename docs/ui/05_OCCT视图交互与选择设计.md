# OCCT 视图交互与选择设计

本文档记录 TS_RS 中 OCCT Viewer 的视图、剖视、选择、高亮设计。

## 当前结论

已确认：

```text
显示页签必须补齐 XYZ 方向剖视和基础旋转。
不做独立“选择”顶层选项卡。
选择由 OCCT Viewer + 状态栏 + 当前命令控制。
STEP / STP 模型统一水泥灰显示。
```

## 显示页签 P0 视图操作

P0 按图石分组：

```text
取景：
  全显
  实时缩放
  实时平移
  窗口缩放

方位：
  轴测
  俯视 / 正视 / 侧视
  垂直于
  旋转

渲染：
  线框
  渲染
  上色

钢筋显示：
  激活
  钝化

切平面：
  X 向剖视
  Y 向剖视
  Z 向剖视
  设置旋转
```

钢筋显示语义：

```text
激活：显示钢筋
钝化：隐藏钢筋
```

窗口组口径：

```text
图石里的窗口重叠 / 水平平铺 / 垂直平铺不进入 P0。
TS_RS 第一版只做单个中央 OCCT Viewer。
```

## OCCT Viewer 视图旋转

P0 必做：

```text
鼠标拖拽旋转视图
显示页签 / 工具栏提供旋转模式入口
旋转过程中保持选中对象高亮
旋转过程中保持 STEP 水泥灰显示
旋转只改变相机，不改变模型坐标和钢筋数据
```

说明：

```text
方位里的“旋转”是视图旋转。
切平面里的“设置旋转”是剖切面角度调整。
两者必须在 UI 和代码命令中分开。
```

## XYZ 方向剖视

OCCT 能力结论：

```text
支持。
```

依据：

```text
OCCT 标准可视化层提供 Graphic3d_ClipPlane。
剖切面可以用 gp_Pln 定义。
ClipPlane 可以 AddClipPlane 到整个 V3d_View，也可以 AddClipPlane 到单个 AIS_InteractiveObject。
通过 SetOn(true/false) 控制启用，通过 V3d_View::Update() 刷新。
```

P0：

```text
X 方向剖视
Y 方向剖视
Z 方向剖视
清除剖视
```

P1：

```text
任意剖切面
剖切面拖拽
多剖切面组合
保存 / 恢复剖视状态
```

设置旋转：

```text
属于切平面工具。
用于把当前切平面旋转一定角度，不是视图旋转。
P0 先做角度输入 + 预览 + 确定 / 取消。
```

实现口径：

```text
XYZ 剖视：创建对应法向的 gp_Pln，再写入 Graphic3d_ClipPlane。
剖切面旋转：用 gp_Trsf::SetRotation(gp_Ax1, angle) 或等价方式旋转当前 gp_Pln，再调用 Graphic3d_ClipPlane::SetEquation(newPlane)。
清除剖视：关闭或移除当前 Graphic3d_ClipPlane。
```

边界：

```text
显示页签剖视是 Viewer 显示剖切，不等于工程图剖切线算法。
旋转剖切面只更新 clip plane，不旋转 STEP 模型，也不旋转钢筋对象。
```

技术口径：

```text
Graphic3d_ClipPlane
gp_Pln
gp_Trsf
gp_Ax1
V3d_View
AIS_InteractiveContext
```

## 选择交互方案

已确认：

```text
不新增独立“选择”顶层选项卡。
选择作为 OCCT Viewer 的交互模式实现。
```

交互：

```text
1. 在命令中或 Viewer 状态栏切换选择过滤：点 / 边 / 面 / 实体。
2. 鼠标移动到可选对象上，Viewer 临时高亮。
3. 鼠标点击后，对象进入已选中状态并保持高亮。
4. 右侧 Dock 显示对象信息。
5. 底部状态栏显示选择模式、选择数量、命令提示。
```

技术依据：

OCCT 支持：

```text
AIS_InteractiveContext
AIS_Shape::SelectionMode(TopAbs_VERTEX)
AIS_Shape::SelectionMode(TopAbs_EDGE)
AIS_Shape::SelectionMode(TopAbs_FACE)
StdSelect_BRepSelectionTool
MoveTo
Select
SelectedInteractive
检测高亮 / 选中高亮
```

## 为什么不建议单独做选择页签

原因：

```text
1. 选择不是独立业务模块，而是 Viewer 和命令共同使用的交互状态。
2. 钢筋创建、参考面定义、工程图剖切都会触发不同选择模式。
3. 如果做成顶层“选择”页签，用户需要频繁切换页签，反而打断操作。
4. 第一版顶层页签保持老图石外壳：开始 / 显示 / 钢筋 / 查询 / 工程图。
```

保留口径：

```text
如果 P1 后出现复杂选择集管理、过滤器管理、按属性选择、框选反选等需求，
再评估是否拆出独立“选择”页签。
```

## STEP / STP 模型显示颜色

已确认：

```text
OCCT Viewer 中，导入的 STEP / STP 模型统一使用水泥灰显示。
不管模型本身有没有颜色，都不作为主显示颜色。
```

建议配色：

```text
结构模型：水泥灰
点 / 线 / 面选中：红色
钢筋轴线 / 钢筋实体：橙黄
当前高亮 / 强调高亮：洋红
参考几何：青色或黄色半透明
剖切面：半透明蓝灰
```

P1 可选：

```text
调试开关：显示 STEP 原始颜色。
按直径 / 等级 / 错误状态给钢筋重新着色。
```

## 颜色语义

P0 固定：

```text
红色：点 / 线 / 面选中
橙黄：钢筋轴线 / 钢筋实体
洋红：高亮、定位、查找结果、悬停强调
水泥灰：STEP / STP 结构模型
```

说明：

红色只表示几何子对象选择，不表示错误。

洋红表示临时强调或定位高亮，不表示已选中集合。
