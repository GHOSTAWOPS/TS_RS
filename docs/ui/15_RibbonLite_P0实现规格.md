# RibbonLite P0 实现规格

## 结论

TS_RS P0 不引入 Codejock，也不做复杂商业 Ribbon。

P0 使用 Qt Widgets 自研最小 RibbonLite：

```text
RibbonBar
RibbonPage
RibbonGroup
RibbonButton
CommandRegistry
```

目标是表达老图石页签和按钮分组，不追求像素级复刻。

## P0 能力

必须支持：

```text
固定页签。
固定分组。
固定按钮。
按钮文本。
按钮 enabled / disabled。
按钮 commandId。
可测试 objectName。
```

不支持：

```text
用户自定义 Ribbon。
拖拽排序。
主题系统。
复杂动画。
QML。
运行时插件化。
图标最终美术。
```

## 当前实现阶段

TODO-015 只实现主窗口骨架，还不是 RibbonLite。

当前允许用 `QTabWidget` 占位，但测试必须锁定：

```text
开始
显示
钢筋
查询
工程图
```

后续 RibbonLite TODO 再替换为专用组件。

## 组件边界

RibbonLite 只负责命令入口。

禁止：

```text
在 RibbonButton 里写钢筋算法。
在 RibbonPage 里直接改 RebarModel。
在 Ribbon 层直接调用 OCCT 拓扑。
```

允许：

```text
按钮触发 commandId。
Command handler 切换右侧 Dock。
Command handler 发起 Application Service。
```
