# LoongRisc 仿真器规范

本仓库用于实现 LoongRisc 仿真器。本文件定义仿真器行为和模块边界。

## 1. 目标

- 按 ISA 语义正确执行 LoongRisc 程序
- 提供可重复、可调试的确定性运行行为
- 为异构加速器模型预留清晰扩展接口

## 2. 核心模块

- **Decoder（解码器）**：将 32 位指令拆分为 opcode 与字段
- **CPU State（CPU 状态）**：维护 `r0-r15`、`PC`、`SR` 与存储接口
- **Executor（执行器）**：按语义更新寄存器、内存与控制流
- **Memory（内存）**：字节编址、字对齐读写校验
- **Control Flow（控制流）**：分支/跳转目标计算与 PC 更新
- **Trace/Debug（可选）**：单步日志、寄存器快照、内存观测

## 3. 架构状态规则

- `r0` 恒为 0，写入无效
- 非控制流指令默认 `PC += 4`
- ISA 标注“更新标志位”的指令需更新 `SR.Z`/`SR.N`
- 对齐错误（`LW`/`SW` 非 4 字节对齐）策略需明确：
  - 建议默认行为：抛出异常并输出清晰诊断

## 4. 解码要求

- 支持 R/I/J 三种格式（见 `docs/isa/isa-spec.md`）
- `opcode=0x00` 与 `opcode=0x08` 必须按 `funct` 二级分发
- 解码表与 `docs/isa/encoding-table.md` 保持一致

## 5. 执行语义要求

- 算术/逻辑运算按 32 位回绕处理
- 分支位移计算：`sext(imm16) << 2`
- J 格式目标地址按高位拼接规则生成
- `JAL` 先写 `r14=PC+4` 再跳转
- `JR`/`JALR` 使用寄存器值作为绝对地址

## 6. 内存与地址空间

- 32 位统一字节编址
- `LW/SW` 必须 4 字节对齐
- `0xFFFF0000-0xFFFFFFFF` MMIO 区域建议通过设备回调实现

## 7. 异构扩展模型

仿真器建议抽象统一加速器后端接口：

- `configure(acc_id, kernel_id?, param_ptr)`
- `start(acc_id, kernel_id?, arg)`
- `wait(acc_id)`
- `sync(mask)`
- `query(acc_id) -> status`

推荐行为：

- `ACC_START`/`ACC_LAUNCH`：异步启动任务
- `ACC_WAIT`/`ACC_SYNC`/`ACC_BARRIER`：阻塞 CPU 直到条件满足
- `ACC_STAT`/`ACC_STAT2`：返回虚拟设备状态

## 8. 测试策略

- 单元测试：
  - 字段提取与解码分发表
  - ALU 运算语义
  - 分支/跳转目标计算
  - 对齐异常处理
- 集成测试：
  - 手写短程序执行
  - 函数调用与返回路径
  - 加速器指令流程（配置 -> 启动 -> 等待）
- 差分测试（后续）：
  - 与参考解释器比较随机指令流结果

## 9. 里程碑

- **M1**：核心整数 ISA（`ADD` 到 `JALR`、`LW/SW`）
- **M2**：汇编/反汇编与执行跟踪
- **M3**：异构扩展 `ACC_*` 仿真
- **M4**：程序加载器与更完整调试能力
