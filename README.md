# loongRisc

LoongRisc 仿真器开发仓库。

## 文档导航

- `docs/README.md`：规范总览
- `docs/isa/isa-spec.md`：ISA 规范
- `docs/isa/encoding-table.md`：指令编码总表
- `docs/compiler/compiler-spec.md`：编译器规范
- `docs/simulator/simulator-spec.md`：仿真器规范

## 代码结构

- `sim/`：仿真器代码（按模块拆分，`.h/.cc` 同目录，分层 CMake）
- `tests/`：基于 gtest 的指令级单元测试

## 运行测试

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## 代码格式化（clang-format）

- 风格配置文件：`.clang-format`
- 手动格式化：`./scripts/format.sh`
- 仅检查格式：`./scripts/format-check.sh`
- 脚本会扫描已追踪和未追踪源码（自动排除 `.gitignore`）
- CMake 目标：
  - `cmake --build build --target format`
  - `cmake --build build --target format-check`

### Push 前自动格式化与检查

执行一次安装 hook：

```bash
./scripts/install-git-hooks.sh
```

之后每次 `git push` 前会自动执行：

1. `clang-format` 自动格式化
2. 若产生改动则阻止 push（提醒你先提交格式化结果）
3. 无改动则允许 push

## 编译方式