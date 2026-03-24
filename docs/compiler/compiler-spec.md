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
