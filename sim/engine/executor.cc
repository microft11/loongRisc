#include "engine/executor.h"

#include <iostream>

namespace loongrisc {

Simulator::Simulator(std::size_t memory_bytes) : state_(memory_bytes) {}

void Simulator::load_program_words(const std::vector<std::uint32_t>& words,
                                   std::uint32_t base_addr) {
  std::uint32_t addr = base_addr;
  for (std::uint32_t word : words) {
    state_.memory.store_word(addr, word);
    addr += 4U;
  }
  state_.pc = base_addr;
}

bool Simulator::step(bool trace) {
  if (state_.pc + 3U >= state_.memory.size()) {
    return false;
  }
  const std::uint32_t raw = state_.memory.load_word(state_.pc);
  const DecodedInstruction insn = decode(raw);
  return execute(insn, trace);
}

void Simulator::run(std::size_t max_steps, bool trace) {
  for (std::size_t i = 0; i < max_steps; ++i) {
    if (!step(trace)) {
      return;
    }
  }
}

bool Simulator::execute(const DecodedInstruction& insn, bool trace) {
  const std::uint32_t old_pc = state_.pc;
  std::uint32_t next_pc = old_pc + 4U;

  if (trace) {
    std::cout << "PC=0x" << std::hex << old_pc
              << " OP=" << op_to_string(insn.op) << " RAW=0x" << insn.raw
              << std::dec << "\n";
  }

  switch (insn.op) {
  case Op::kAdd: {
    const std::uint32_t result =
        state_.read_reg(insn.rs) + state_.read_reg(insn.rt);
    state_.write_reg(insn.rd, result);
    update_zn_flags(state_, result);
    break;
  }
  case Op::kSub: {
    const std::uint32_t result =
        state_.read_reg(insn.rs) - state_.read_reg(insn.rt);
    state_.write_reg(insn.rd, result);
    update_zn_flags(state_, result);
    break;
  }
  case Op::kAnd: {
    const std::uint32_t result =
        state_.read_reg(insn.rs) & state_.read_reg(insn.rt);
    state_.write_reg(insn.rd, result);
    update_zn_flags(state_, result);
    break;
  }
  case Op::kOr: {
    const std::uint32_t result =
        state_.read_reg(insn.rs) | state_.read_reg(insn.rt);
    state_.write_reg(insn.rd, result);
    update_zn_flags(state_, result);
    break;
  }
  case Op::kXor: {
    const std::uint32_t result =
        state_.read_reg(insn.rs) ^ state_.read_reg(insn.rt);
    state_.write_reg(insn.rd, result);
    update_zn_flags(state_, result);
    break;
  }
  case Op::kAddi: {
    const std::uint32_t result =
        state_.read_reg(insn.rs) +
        static_cast<std::uint32_t>(sign_extend_16(insn.imm16));
    state_.write_reg(insn.rt, result);
    update_zn_flags(state_, result);
    break;
  }
  case Op::kLw: {
    const std::uint32_t addr =
        state_.read_reg(insn.rs) +
        static_cast<std::uint32_t>(sign_extend_16(insn.imm16));
    const std::uint32_t value = state_.memory.load_word(addr);
    state_.write_reg(insn.rt, value);
    break;
  }
  case Op::kSw: {
    const std::uint32_t addr =
        state_.read_reg(insn.rs) +
        static_cast<std::uint32_t>(sign_extend_16(insn.imm16));
    state_.memory.store_word(addr, state_.read_reg(insn.rt));
    break;
  }
  case Op::kBeq: {
    if (state_.read_reg(insn.rs) == state_.read_reg(insn.rt)) {
      const std::int32_t offset = sign_extend_16(insn.imm16) << 2;
      next_pc = static_cast<std::uint32_t>(
          static_cast<std::int32_t>(old_pc + 4U) + offset);
    }
    break;
  }
  case Op::kBne: {
    if (state_.read_reg(insn.rs) != state_.read_reg(insn.rt)) {
      const std::int32_t offset = sign_extend_16(insn.imm16) << 2;
      next_pc = static_cast<std::uint32_t>(
          static_cast<std::int32_t>(old_pc + 4U) + offset);
    }
    break;
  }
  case Op::kJ: {
    next_pc = (old_pc & 0xF0000000U) | (insn.target26 << 2U);
    break;
  }
  case Op::kJal: {
    state_.write_reg(14, old_pc + 4U);
    next_pc = (old_pc & 0xF0000000U) | (insn.target26 << 2U);
    break;
  }
  case Op::kJr: {
    next_pc = state_.read_reg(insn.rs);
    break;
  }
  case Op::kJalr: {
    state_.write_reg(insn.rd, old_pc + 4U);
    next_pc = state_.read_reg(insn.rs);
    break;
  }
  case Op::kUnknown:
    return false;
  }

  state_.pc = next_pc;
  return true;
}

} // namespace loongrisc
