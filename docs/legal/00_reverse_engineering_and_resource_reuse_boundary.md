# 逆向分析与资源复用边界

本文档按外部技术评审意见补齐 TS_RS 的合规和工程边界。

## 结论

TS_RS 是独立开源 / 自研系统方向。

允许使用旧软件和 RebarSmart 作为行为证据来源，但不允许引入商业运行时依赖，也不允许直接复用受保护资源。

## 允许

### 行为观察

允许：

```text
阅读使用说明 PDF。
观察 RebarSmart / 老图石实际运行行为。
记录菜单、参数、默认值、输出文件结构。
记录自己生成的 STEP / Detail / CSV / XML 输出。
```

### 逆向证据

允许：

```text
用 IDA MCP 查询函数名、调用链、常量、字段、字符串和控制流。
把不确定行为标记为 GAP。
用伪代码理解算法意图。
```

要求：

```text
不确定的算法行为不能写成确定结论。
必须记录证据来源和置信等级。
```

### 工程参考

允许参考旧实现项目：

```text
CMake 查找 Qt / OCCT。
STEP 导入。
AIS Viewer。
选择 / 高亮。
XCAF / TopoDS 遍历。
剖切 / 颜色 / 稳定 ID 思路。
Detail writer 的事务和验证思想。
```

## 禁止

### 禁止商业运行时依赖

禁止：

```text
调用 RebarSmart DLL。
调用 VisualTS EXE / DLL。
依赖 3DE / CAA / CATIA。
依赖 ACIS / HOOPS / Codejock。
要求 USB 加密狗作为 TS_RS 运行条件。
```

### 禁止直接复用商业资源

禁止：

```text
复制 RebarSmart / 老图石的图标、位图、界面资源。
复制商业软件原始字符串资源作为 TS_RS 产品文案。
复制受保护的示例工程作为发布资产。
复制 DLL 内二进制数据表作为运行资源。
```

### 禁止迁入旧路线钢筋业务

旧实现项目【图石钢筋1比1复刻】只作为 Qt / OCCT 工程零件库参考。

禁止迁入：

```text
EdgeToRebarFactory。
FaceRebarGenerator。
PolylineRebarGenerator。
RebarCreationCommandService 中直接造钢筋的业务流程。
旧项目直接用 OCCT 写钢筋的业务逻辑。
```

原因：

```text
旧项目路线是“OCCT 直接重写钢筋”，
TS_RS 当前路线是“RebarSmart 证据驱动钢筋逻辑复刻”。
```

## 文档引用规则

文档中引用逆向结论时必须写明：

```text
证据来源：
  PDF / INI / CATNls / CATRsc / IDA / 实际运行 / 输出文件

置信等级：
  High / Medium / Low / GAP

是否可编码：
  yes / partial / no
```

## 代码 review 护栏

每个代码节点需要检查：

```powershell
rg -n "RebarSmart.*\\.dll|VisualTS|CAA|CATIA|ACIS|HOOPS|Codejock|hasp|dongle|加密狗" app docs
rg -n "TopoDS_|AIS_|BRep|TopAbs_|gp_|Geom_" app/src/domain app/src/rebarsmart
```

出现商业运行时依赖或 domain/rebarsmart OCCT 泄漏时，必须停止并整改。

## 用户运行环境说明

用户可以保留老图石、RebarSmart、AutoCAD 插件作为验证工具。

但 TS_RS 自身目标是：

```text
不需要 USB 狗。
不需要商业 CAD / 3DE 运行时才能启动。
不依赖老图石安装目录。
```

