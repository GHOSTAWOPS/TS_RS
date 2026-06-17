# GC-002 TopologyBinding 稳定性

## 目标

验证 TODO-020B 的最小拓扑绑定链路：

```text
STEP root shape
  -> ShapeStore
  -> face / edge / vertex binding
  -> stable id
  -> binding reference 保存 / 恢复
```

本 case 只证明 P0 绑定机制可运行，不证明：

```text
真实 Viewer 选择已完成。
复杂工程 STEP 没有 fingerprint collision。
钢筋生成器可接真实选择。
```

## 当前自动样本

```text
样本：测试进程内用 BRepPrimAPI_MakeBox(1.0, 2.0, 3.0) 生成。
格式：写出临时 STEP，再重新导入。
是否提交 git：否，测试运行时生成。
```

## 验证命令

```powershell
ctest --test-dir build/default -R tsrs_topology_binding_registry_tests --output-on-failure -C Debug
```

结果：

```text
1/1 passed。
```

覆盖点：

```text
同一 STEP 连续导入 5 次。
box face / edge / vertex 唯一 binding 数量为 6 / 12 / 8。
face / edge / vertex stableId 列表稳定。
edge binding reference 可 serialize / deserialize 后恢复。
restore 后 stableId / kind / geometryFingerprint / bbox / measure 一致。
missing / kind mismatch / ambiguous diagnostic 稳定。
```

## 当前未闭合

| ID | 缺口 | 状态 |
| --- | --- | --- |
| GC002-GAP-001 | 当前自动 case 是 box，不是真实工程 STEP。 | 后续使用 `123.stp` 扩展本 case，记录统计和 collision 情况。 |
| GC002-GAP-002 | 当前没有 Viewer 点击子对象到 binding id 的人工记录。 | TODO-021 处理。 |

## 结论

```text
GC-002 P0 自动验证已通过。
TopologyBinding 机制具备进入 Detail 前置验证阶段的最低条件。
```
