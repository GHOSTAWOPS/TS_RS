# 钢筋创建 Dock 面板与 RebarSmart 弹窗一致性规则

本文档记录 TS_RS 钢筋创建 Dock 面板的硬规则。

## 核心结论

```text
后续所有钢筋创建 Dock 面板，都必须与 RebarSmart 对应创建弹窗保持一致。
```

这里的“一致”指：

```text
参数分组一致
字段槽位一致
选择对象类型一致
默认值 / 单位 / 枚举语义一致
联动规则和校验规则按 RebarSmart 证据复刻
```

不要求一致的是：

```text
容器形式：RebarSmart 是 3DE 弹窗，TS_RS 是右侧 Dock
按钮文案：RebarSmart 的“确定”在 TS_RS 中可叫“应用”
对象拾取方式：TS_RS 适配 OCCT Viewer
错误提示位置：TS_RS 可放在 Dock 顶部或底部状态区
调试信息：TS_RS 可增加折叠调试区
```

## 不允许的做法

禁止：

```text
为了 MVP 随便删 RebarSmart 弹窗里的字段
为了界面简洁随便合并 RebarSmart 的参数分组
把 RebarSmart 没有的核心参数体系替换成自研简化参数
只做“够用”的简化版钢筋生成器
没有证据就改字段含义、默认值、单位或枚举语义
```

如果某个字段暂时无法实现：

```text
字段槽位仍保留
状态标记为待复刻 / 暂不可用
缺口写入对应专项文档和 todo
后续用 IDA MCP / PDF / 运行确认闭合
```

## 面板映射

P0 8 个钢筋创建命令：

```text
定距钢筋
  -> FixDistancePanel
  -> GenerateRebarFixDistanceDlg

定数钢筋
  -> FixNumberPanel
  -> GenerateRebarFixNumberDlg

角筋
  -> CornerPanel
  -> GenerateRebarCornerDlg

拉结筋
  -> TiePanel
  -> GenerateRebarTieDlg

插筋
  -> DowelPanel
  -> GenerateRebarDowelDlg

柱锥环盘面筋
  -> FrustumPanel
  -> GenerateRebarFrustumDlg

箍筋
  -> StirrupPanel
  -> GenerateRebarStirrupDlg

自定义钢筋
  -> CustomizePanel
  -> GenerateRebarCustomizeDlg
```

扩展候选命令：

```text
等参扫掠钢筋
  -> SweepPanel
  -> GenerateRebarSweepDlg

多段线筋
  -> PolylinePanel
  -> GenerateRebarPolylineDlg

螺旋筋
  -> HelixPanel
  -> GenerateRebarHelixDlg

平移复制钢筋
  -> CopyTranslatePanel
  -> GenerateRebarCopyTranslateDlg

对称复制钢筋
  -> CopySymmetryPanel
  -> GenerateRebarCopySymmetryDlg

旋转复制钢筋
  -> CopyRotationPanel
  -> GenerateRebarCopyRotationDlg
```

## Dock 统一结构

每个钢筋创建 Dock 可以有 TS_RS 统一外框：

```text
Dock 顶部摘要
  当前命令
  当前对象
  选择状态
  状态 / 错误

RebarSmart 原始参数分组
  按对应 GenerateRebarXXXDlg 保持字段和分组

操作区
  预览
  应用
  取消

可选折叠调试区
  对象 ID
  绑定 Face / Edge / Point
  generator 名称
  evidence / gap 标记
```

统一外框不能替代 RebarSmart 原始参数分组。

## Dock 顶部摘要规则

每个钢筋创建 / 编辑 Dock 顶部固定显示四行摘要：

```text
当前命令
当前对象
选择状态
状态提示
```

示例：

```text
当前命令：定数钢筋
当前对象：新建钢筋组
选择状态：布筋面 2，引导线 1，参考线 0
状态提示：可预览
```

当前对象展示规则：

```text
创建钢筋：
  显示“新建钢筋组 / 新建定距钢筋 / 新建定数钢筋”

编辑钢筋组：
  显示“G-001 定距钢筋 / G-002 箍筋”

查看单根钢筋：
  显示“G-001 / #12，所属组 G-001 定距钢筋”
```

状态提示示例：

```text
可预览
缺少布筋面
缺少引导线
参数错误
预览中
已生成，等待选择对象
```

折叠调试区：

```text
对象 ID
commandId
selectedFaceIds
selectedEdgeIds
generatorClass
evidenceStatus
gapStatus
```

边界：

```text
1. 顶部摘要是 TS_RS 统一外框。
2. 顶部摘要不能替代 RebarSmart 原始弹窗参数分组。
3. 当前对象不能只显示内部 ID，必须显示用户能理解的钢筋组 / 单根 / 新建状态。
4. 内部 ID 和绑定信息只放折叠调试区。
```

## 证据优先级

每个 Dock 面板设计必须按这个优先级闭合：

```text
1. RebarSmart 使用说明截图 / PDF 操作说明
2. GenerateRebarXXXDlg.CATNls / CATRsc
3. ResourceRebarSmart 默认 ini / 样式字段
4. RebarCreation.dll 命令和 UI 初始化逻辑
5. AfrRebarObjImp.dll 的 GenerateRebarXXXImp
6. AfrRebarCore3D.dll 公共字段和算法
7. RebarSmart 实际运行确认
```

没有证据时：

```text
不能拍脑袋简化
不能写成确定结论
只能标记为缺口
```

## 定数钢筋截图证据

用户提供的 RebarSmart / 3DE 定数钢筋截图确认：

```text
定数钢筋原始界面是弹窗。
TS_RS 改成右侧 Dock，但内容必须对齐该弹窗。
```

截图中可见主要区域：

```text
定位
  布筋面
  偏移面
  保护层厚度
  反向
  钢筋层数
  单层 / 层间距

布筋区间
  布筋引导线表格
  区间首端距离
  区间末端距离
  钢筋根数

轴线经孔洞处理
  处理方式

轴线延长 / 截断
  首端距离 / 参考对象
  末端距离 / 参考对象

生成实体钢筋对象
```

该截图后续作为 `FixNumberPanel` 第一证据之一。
