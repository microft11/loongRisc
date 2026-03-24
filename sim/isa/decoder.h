#pragma once

#include <cstdint>
#include <string>

namespace loongrisc {

enum class Op {
  kAdd,
  kSub,
  kAnd,
  kOr,
  kXor,
  kAddi,
  kLw,
  kSw,
  kBeq,
  kBne,
  kJ,
  kJal,
  kJr,
  kJalr,
  kUnknown,
};

struct DecodedInstruction {
  Op op = Op::kUnknown;
  std::uint8_t rs = 0;
  std::uint8_t rt = 0;
  std::uint8_t rd = 0;
  std::uint8_t funct = 0;
  std::uint16_t imm16 = 0;
  std::uint32_t target26 = 0;
  std::uint32_t raw = 0;
};

DecodedInstruction decode(std::uint32_t raw);
std::string opToString(Op op);
std::int32_t signExtend16(std::uint16_t imm16);

} // namespace loongrisc
