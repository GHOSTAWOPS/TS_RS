# TODO-026 Tsrebar Minimal Save/Open 实现记录

## 结论

TODO-026 已完成 `.tsrebar` 最小工程文件保存和回读。

本轮只建立 TS_RS 内部主工程格式的 P0A 最小持久化边界：

```text
TsrebarProjectDocument
  -> writeTsrebarProjectFile(...)
  -> readTsrebarProjectFile(...)
```

本轮不接 UI，不接 Detail，不接 Viewer，不接钢筋生成器。

`.tsrebar` 是 TS_RS 内部主工程格式。Detail 包仍是导出格式，不能反向污染
`.tsrebar` 或 `RebarModel`。

## 新增代码

新增：

```text
app/src/project/tsrebar/TsrebarProjectFile.h
app/src/project/tsrebar/TsrebarProjectFile.cpp
app/tests/project/tsrebar_project_file_tests.cpp
```

新增 CMake target：

```text
tsrs_project_tsrebar
tsrs_tsrebar_project_file_tests
```

## P0A 保存字段

当前 `.tsrebar` P0A 使用 JSON，固定格式版本：

```text
format = "tsrebar"
formatVersion = 1
```

保存项目摘要：

```text
project.projectId
project.sourceStepPath
project.sourceStepSha256
project.sourceLengthUnit
project.sourceToMeterScale
```

保存已提交钢筋组摘要：

```text
rebarGroups[].groupId
rebarGroups[].commandId
rebarGroups[].styleName
rebarGroups[].grade
rebarGroups[].diameterM
rebarGroups[].bundleCount
rebarGroups[].centerlineStableIds[]
```

## 明确不保存

本轮不保存：

```text
preview draft
dirty flag
OCCT shape / TopoDS
AIS object
Detail XML
StbGroup / StbGeo / StbTable / MaterialTable
UI window state
generator runtime object
```

原因：

```text
1. preview / dirty 是运行时事务状态，不是工程事实。
2. OCCT / AIS 是显示和几何运行时对象，不能进入内部工程格式。
3. Detail 包是导出兼容格式，不是 TS_RS 主保存格式。
4. RebarModel 当前只保存 P0A 最小钢筋组摘要，后续字段必须按业务证据扩展。
```

## 诊断码

新增诊断：

```text
TSREBAR_OK
TSREBAR_IO_ERROR
TSREBAR_INVALID_FORMAT
TSREBAR_UNSUPPORTED_VERSION
```

读取逻辑会拒绝：

```text
非 JSON object
format 不是 tsrebar
formatVersion 不是当前版本
缺失 project object
project 必需字段缺失或类型错误
缺失 rebarGroups array
rebarGroups 元素不是 object
rebarGroups 必需字段缺失或类型错误
```

## 测试

新增：

```text
app/tests/project/tsrebar_project_file_tests.cpp
```

覆盖：

```text
1. writer 能写出最小 .tsrebar JSON。
2. 写出文件包含 format = tsrebar。
3. 写出文件不包含 TopoDS_ / AIS_ / Detail。
4. reader 能读回 writer 产物。
5. projectId / sourceStepPath / sourceStepSha256 / sourceLengthUnit /
   sourceToMeterScale round-trip。
6. rebarGroups / centerlineStableIds round-trip。
7. unsupported formatVersion 返回 TSREBAR_UNSUPPORTED_VERSION。
8. malformed JSON 返回 TSREBAR_INVALID_FORMAT。
9. 缺失 project object 返回 TSREBAR_INVALID_FORMAT。
10. rebarGroups 不是 array 返回 TSREBAR_INVALID_FORMAT。
11. rebarGroups 字段类型错误返回 TSREBAR_INVALID_FORMAT。
```

## TDD 与故障定位记录

RED：

```text
cmake --build build/default --config Debug --target tsrs_tsrebar_project_file_tests
```

初始结果：

```text
fatal error C1083: cannot open include file:
project/tsrebar/TsrebarProjectFile.h
```

实现后出现 CTest 超时：

```text
ctest --test-dir build/default -C Debug -R "tsrs_tsrebar_project_file_tests" -V --output-on-failure
```

症状：

```text
CTest timeout 10s
```

人工运行时弹出系统错误框：

```text
由于找不到 Qt6Cored.dll，无法继续执行代码。
```

根因：

```text
tsrs_tsrebar_project_file_tests 链接 Qt6::Core，
但 CTest 环境没有注入 vcpkg debug/bin runtime PATH。
```

修复：

```text
app/tests/CMakeLists.txt
  -> 为 tsrs_tsrebar_project_file_tests 设置 PATH，
     注入 TSRS_VCPKG_DEBUG_RUNTIME_BIN_DIR / TSRS_VCPKG_RUNTIME_BIN_DIR。
```

GREEN：

```text
ctest --test-dir build/default -C Debug -R "tsrs_tsrebar_project_file_tests" -V --output-on-failure
```

结果：

```text
1/1 passed
tsrs_tsrebar_project_file_tests Passed 0.03 sec
```

## xhigh review 修复

本轮 xhigh 只读 review 返回：

```text
Overall: blocked
Critical: 无
Important:
  1. reader schema 校验太松，坏文件会被读成 ok=true。
  2. round-trip 测试没有覆盖全部 P0A group 字段。
```

处理：

```text
1. 补充 project / rebarGroups / group 字段存在性和类型校验。
2. 补充完整 group 字段 round-trip 断言。
3. 补充缺 project、rebarGroups 非 array、group 字段类型错误测试。
```

修复后专项验证：

```text
ctest --test-dir build/default -C Debug -R "tsrs_tsrebar_project_file_tests" --output-on-failure
```

结果：

```text
1/1 passed
```

## 默认验证

默认串行构建和 CTest：

```text
cmake --build build/default --config Debug -- -j1
ctest --test-dir build/default -C Debug --output-on-failure
```

结果：

```text
28/28 passed
```

边界扫描：

```text
rg -n "TopoDS_|AIS_|BRep|TopAbs|gp_|Geom_" app/src/domain app/src/project app/src/rebarsmart app/src/drawing
rg -n "drawing/detail|DetailPackageWriter|DetailPackageReader|rebarsmart/generators|FixDistanceGenerator|FixNumberGenerator" app/src/project app/src/ui app/src/presentation/occ app/src/application/commands
```

结果：

```text
无命中。
```

## 边界

本轮没有做：

```text
不实现 ScheduleModel。
不接 RebarCreationCommandService 生成器主链。
不导出 Detail 包。
不实现 DrawingModel。
不接 Viewer。
不保存 OCCT / AIS / Detail 对象。
不扩展完整钢筋字段。
```

## 对下一节点的影响

TODO-026 完成后，可以进入：

```text
TODO-027 ScheduleModel 基础下料表 P0
```

TODO-027 必须继续保持：

```text
ScheduleModel 从 RebarModel 生成基础数量 / 长度摘要。
Detail 字段不能反向污染 RebarModel 或 ScheduleModel。
domain/rebar 仍不能依赖 OCCT / AIS / Qt / Detail。
```
