#include "isa/decoder.h"

namespace loongrisc {

namespace {
constexpr std::uint8_t bits(std::uint32_t value, std::uint8_t hi,
                            std::uint8_t lo) {
  return static_cast<std::uint8_t>((value >> lo) &
                                   ((1U << (hi - lo + 1U)) - 1U));
}
} // namespace

DecodedInstruction decode(std::uint32_t raw) {
  DecodedInstruction insn{};
  insn.raw = raw;

  const std::uint8_t opcode = bits(raw, 31, 26);
  insn.rs = bits(raw, 25, 21);
  insn.rt = bits(raw, 20, 16);
  insn.rd = bits(raw, 15, 11);
  insn.funct = bits(raw, 5, 0);
  insn.imm16 = static_cast<std::uint16_t>(raw & 0xFFFFU);
  insn.target26 = raw & 0x03FFFFFFU;

  if (opcode == 0x00U) {
    switch (insn.funct) {
    case 0x00:
      insn.op = Op::kAdd;
      return insn;
    case 0x01:
      insn.op = Op::kSub;
      return insn;
    case 0x02:
      insn.op = Op::kAnd;
      return insn;
    case 0x03:
      insn.op = Op::kOr;
      return insn;
    case 0x04:
      insn.op = Op::kXor;
      return insn;
    default:
      insn.op = Op::kUnknown;
      return insn;
    }
  }

  if (opcode == 0x08U) {
    switch (insn.funct) {
    case 0x00:
      insn.op = Op::kJr;
      return insn;
    case 0x01:
      insn.op = Op::kJalr;
      return insn;
    default:
      insn.op = Op::kUnknown;
      return insn;
    }
  }

  switch (opcode) {
  case 0x01:
    insn.op = Op::kAddi;
    break;
  case 0x06:
    insn.op = Op::kLw;
    break;
  case 0x07:
    insn.op = Op::kSw;
    break;
  case 0x09:
    insn.op = Op::kBeq;
    break;
  case 0x0A:
    insn.op = Op::kBne;
    break;
  case 0x0D:
    insn.op = Op::kJ;
    break;
  case 0x0E:
    insn.op = Op::kJal;
    break;
  default:
    insn.op = Op::kUnknown;
    break;
  }
  return insn;
}

std::string opToString(Op op) {
  switch (op) {
  case Op::kAdd:
    return "ADD";
  case Op::kSub:
    return "SUB";
  case Op::kAnd:
    return "AND";
  case Op::kOr:
    return "OR";
  case Op::kXor:
    return "XOR";
  case Op::kAddi:
    return "ADDI";
  case Op::kLw:
    return "LW";
  case Op::kSw:
    return "SW";
  case Op::kBeq:
    return "BEQ";
  case Op::kBne:
    return "BNE";
  case Op::kJ:
    return "J";
  case Op::kJal:
    return "JAL";
  case Op::kJr:
    return "JR";
  case Op::kJalr:
    return "JALR";
  default:
    return "UNKNOWN";
  }
}

std::int32_t signExtend16(std::uint16_t imm16) {
  return static_cast<std::int16_t>(imm16);
}

} // namespace loongrisc
