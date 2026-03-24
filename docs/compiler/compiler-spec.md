# LoongRisc 编译器规范

本文件定义 LoongRisc 的编译器侧约束。
ISA 语义以 `docs/isa/isa-spec.md` 为准。

## 1. 适用范围

- C/C++ 整数程序代码生成
- 前端、后端与运行时之间的 ABI 约定
- 异构扩展 builtin/intrinsic 映射规则

## 2. ABI 约定

### 2.1 寄存器约定

| 寄存器 | 别名 | 角色 | 保存者 |
| --- | --- | --- | --- |
| `r0` | `zero` | 常量 0 | 无 |
| `r1-r3` | `a0-a3` | 参数/返回值寄存器（`a0` 返回） | 调用者 |
| `r4-r11` | `s0-s7` | 被调用者保存寄存器 | 被调用者 |
| `r12` | `fp` | 帧指针（可选） | 被调用者 |
| `r13` | `sp` | 栈指针 | 被调用者 |
| `r14` | `lr` | 链接寄存器 | 调用序列 |
| `r15` | `pc` | 程序计数器 | 硬件 |

### 2.2 栈帧规则

- 栈向下增长（高地址到低地址）
- 栈按字对齐
- 前 4 个整数参数通过 `r1-r4` 传递
- 额外参数由调用点压栈传递
- 32 位返回值放在 `r1`
- 需要时 64 位返回值放在 `r1/r2`
- 被调用者使用 `r4-r13` 时必须保存并恢复

### 2.3 标准调用序列

- 调用：`JAL target`（返回地址写入 `r14`）
- 返回：`JR r14`
- 非叶子函数若会覆盖 `r14`，需显式保存恢复

## 3. 立即数与分支降级规则

- 算术立即数：`imm16` 符号扩展
- 逻辑立即数（`ANDI/ORI/XORI`）：`imm16` 零扩展
- 分支位移：PC 相对，按 4 字节缩放
- 超范围分支/跳转：由编译器生成跳板或 veneer

## 4. Builtin 与 Intrinsic 映射

前端 builtin 原型：

```c
void __builtin_acc_config(unsigned int acc_id, unsigned int kernel_id, void *params);
void __builtin_acc_launch(unsigned int acc_id, unsigned int kernel_id, void *args);
void __builtin_acc_wait(unsigned int acc_id);
void __builtin_acc_sync(unsigned int acc_mask);
void __builtin_acc_sync_all(void);
unsigned int __builtin_acc_query(unsigned int acc_id);
void __builtin_acc_ctl_write(unsigned int reg_id, unsigned int value);
```

后端映射规则：

| Builtin | 降级模式 |
| --- | --- |
| `__builtin_acc_config(acc, kid, ptr)` | `ACC_CFG2 ptr, ((kid << 4) \| acc)` |
| `__builtin_acc_launch(acc, kid, arg)` | `ACC_LAUNCH arg, ((kid << 4) \| acc)` |
| `__builtin_acc_wait(acc)` | `ACC_WAIT r0, acc` |
| `__builtin_acc_sync(mask)` | `ACC_SYNC r0, mask` |
| `__builtin_acc_sync_all()` | `ACC_BARRIER` |
| `__builtin_acc_query(acc)` | `ACC_STAT2 acc, rd_tmp` |
| `__builtin_acc_ctl_write(reg, val)` | `ACC_MOV reg, val` |

LLVM 落地模型：

1. builtin -> 目标相关 LLVM intrinsic
2. intrinsic -> SelectionDAG/GlobalISel 模式
3. 模式 -> 机器指令编码

### 4.1 参数指针与 descriptor（与 ISA 对齐）

- `__builtin_acc_config` 的第三参 `params`（以及实现中传给 `ACC_CFG2` 的指针）**必须**指向满足 `docs/isa/isa-spec.md` **§7.3** 的 **v0 descriptor**：含 `magic`、`version`、`command`、`payloadWords` 及字对齐载荷。
- 编译器前端 **不负责** 自动把任意 C 结构体填成合法 descriptor；应由 **运行时库或手写代码** 按 ABI 构造。编译器可（可选）提供内联封装或诊断未对齐指针。
- `params` 所指对象须 **字对齐**，且在被 `ACC_CFG2` 读取前，对主机侧而言应已完成写入（在 **LoongRisc-Sim** 目标下，与 ISA §7.4.1 一致时，无需额外 fence；见下节）。

### 4.2 同步类 builtin 与内存可见性

ISA 对 `ACC_WAIT` / `ACC_SYNC` / `ACC_BARRIER` 的 **主存可见性** 在 **§7.4** 分「仿真器」与「目标硬件」两级约定。编译器规范约定如下：

- **默认目标为 LoongRisc-Sim（参考仿真器）** 时：映射到上述同步指令的 builtin（`__builtin_acc_wait` / `__builtin_acc_sync` / `__builtin_acc_sync_all`）**不要求** 编译器在前后自动插入额外 `FENCE` 或缓存维护序列；软件可依赖 ISA §7.4.1 所述语义。
- **目标为带 cache / 弱序内存的真实硬件** 且平台采用 ISA §7.4.2 **方案 B** 时：编译器或平台 ABI **必须** 明确：
  - 是否在 intrinsic  lowering 中插入显式同步（例如未来的 `FENCE` 指令或 MMIO 序列），或
  - 要求用户/运行时仅在 **non-cacheable** 区域与加速器共享缓冲，并写入用户文档。
- 在未声明平台内存模型前，**不得** 假定同步类 builtin 隐含硬件级 cache 一致性；与 ISA §7.4.2 冲突的实现须在平台文档中单列。

### 4.3 与运行时的分工

- **编译器**：保证 builtin → 指令映射正确、调用约定与寄存器使用正确；可按目标特性决定是否插入 fence。
- **运行时**：提供 descriptor 构造、版本协商、载荷编码；在硬件方案 B 下提供「刷 cache / 映射 uncached」等封装时，应在 runtime 文档中与编译器约定调用边界。

## 5. 后端启动清单

- 定义目标寄存器信息（`r0-r15` 与别名）
- 实现调用约定与栈帧生成
- 添加核心 ISA 指令选择模式
- 需要时添加 `MOV/NOP` 伪指令展开
- 按 `encoding-table.md` 实现 MC 编解码
- 实现汇编解析与打印
- 添加基础代码生成测试（MIR + asm）

## 6. 测试建议

- ABI：调用者/被调用者保存正确性
- 分支：前向/后向位移边界
- 立即数：符号扩展/零扩展行为
- Builtin：每个 builtin 产出预期机器序列
- 端到端 C 测试：循环、递归、访存、函数调用链
