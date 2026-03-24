#include "engine/executor.h"

#include <iostream>

namespace loongrisc {

Simulator::Simulator(std::size_t memoryBytes) : state_(memoryBytes) {}

void Simulator::loadProgramWords(const std::vector<std::uint32_t>& words,
                                 std::uint32_t baseAddr) {
  std::uint32_t addr = baseAddr;
  for (std::uint32_t word : words) {
    state_.memory.storeWord(addr, word);
    addr += 4U;
  }
  state_.pc = baseAddr;
}

bool Simulator::step(bool trace) {
  if (state_.pc + 3U >= state_.memory.size()) {
    return false;
  }
  const std::uint32_t raw = state_.memory.loadWord(state_.pc);
  const DecodedInstruction insn = decode(raw);
  return execute(insn, trace);
}

void Simulator::run(std::size_t maxSteps, bool trace) {
  for (std::size_t i = 0; i < maxSteps; ++i) {
    if (!step(trace)) {
      return;
    }
  }
}

bool Simulator::execute(const DecodedInstruction& insn, bool trace) {
  const std::uint32_t oldPc = state_.pc;
  std::uint32_t nextPc = oldPc + 4U;

  if (trace) {
    std::cout << "PC=0x" << std::hex << oldPc << " OP=" << opToString(insn.op)
              << " RAW=0x" << insn.raw << std::dec << "\n";
  }

  switch (insn.op) {
  case Op::kAdd: {
    const std::uint32_t result =
        state_.readReg(insn.rs) + state_.readReg(insn.rt);
    state_.writeReg(insn.rd, result);
    updateZnFlags(state_, result);
    break;
  }
  case Op::kSub: {
    const std::uint32_t result =
        state_.readReg(insn.rs) - state_.readReg(insn.rt);
    state_.writeReg(insn.rd, result);
    updateZnFlags(state_, result);
    break;
  }
  case Op::kAnd: {
    const std::uint32_t result =
        state_.readReg(insn.rs) & state_.readReg(insn.rt);
    state_.writeReg(insn.rd, result);
    updateZnFlags(state_, result);
    break;
  }
  case Op::kOr: {
    const std::uint32_t result =
        state_.readReg(insn.rs) | state_.readReg(insn.rt);
    state_.writeReg(insn.rd, result);
    updateZnFlags(state_, result);
    break;
  }
  case Op::kXor: {
    const std::uint32_t result =
        state_.readReg(insn.rs) ^ state_.readReg(insn.rt);
    state_.writeReg(insn.rd, result);
    updateZnFlags(state_, result);
    break;
  }
  case Op::kAddi: {
    const std::uint32_t result =
        state_.readReg(insn.rs) +
        static_cast<std::uint32_t>(signExtend16(insn.imm16));
    state_.writeReg(insn.rt, result);
    updateZnFlags(state_, result);
    break;
  }
  case Op::kLw: {
    const std::uint32_t addr =
        state_.readReg(insn.rs) +
        static_cast<std::uint32_t>(signExtend16(insn.imm16));
    const std::uint32_t value = state_.memory.loadWord(addr);
    state_.writeReg(insn.rt, value);
    break;
  }
  case Op::kSw: {
    const std::uint32_t addr =
        state_.readReg(insn.rs) +
        static_cast<std::uint32_t>(signExtend16(insn.imm16));
    state_.memory.storeWord(addr, state_.readReg(insn.rt));
    break;
  }
  case Op::kBeq: {
    if (state_.readReg(insn.rs) == state_.readReg(insn.rt)) {
      const std::int32_t offset = signExtend16(insn.imm16) << 2;
      nextPc = static_cast<std::uint32_t>(
          static_cast<std::int32_t>(oldPc + 4U) + offset);
    }
    break;
  }
  case Op::kBne: {
    if (state_.readReg(insn.rs) != state_.readReg(insn.rt)) {
      const std::int32_t offset = signExtend16(insn.imm16) << 2;
      nextPc = static_cast<std::uint32_t>(
          static_cast<std::int32_t>(oldPc + 4U) + offset);
    }
    break;
  }
  case Op::kJ: {
    nextPc = (oldPc & 0xF0000000U) | (insn.target26 << 2U);
    break;
  }
  case Op::kJal: {
    state_.writeReg(14, oldPc + 4U);
    nextPc = (oldPc & 0xF0000000U) | (insn.target26 << 2U);
    break;
  }
  case Op::kJr: {
    nextPc = state_.readReg(insn.rs);
    break;
  }
  case Op::kJalr: {
    state_.writeReg(insn.rd, oldPc + 4U);
    nextPc = state_.readReg(insn.rs);
    break;
  }
  case Op::kUnknown:
    return false;
  }

  state_.pc = nextPc;
  return true;
}

} // namespace loongrisc
