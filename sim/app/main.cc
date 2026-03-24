#include <cstdint>
#include <iostream>
#include <vector>

#include "engine/executor.h"

namespace {

std::uint32_t encode_i(std::uint8_t opcode, std::uint8_t rs, std::uint8_t rt,
                       std::uint16_t imm16) {
  return (static_cast<std::uint32_t>(opcode) << 26U) |
         (static_cast<std::uint32_t>(rs) << 21U) |
         (static_cast<std::uint32_t>(rt) << 16U) |
         static_cast<std::uint32_t>(imm16);
}

std::uint32_t encode_r(std::uint8_t opcode, std::uint8_t rs, std::uint8_t rt,
                       std::uint8_t rd, std::uint8_t funct) {
  return (static_cast<std::uint32_t>(opcode) << 26U) |
         (static_cast<std::uint32_t>(rs) << 21U) |
         (static_cast<std::uint32_t>(rt) << 16U) |
         (static_cast<std::uint32_t>(rd) << 11U) |
         static_cast<std::uint32_t>(funct);
}

} // namespace

int main() {
  using namespace loongrisc;

  Simulator sim(64 * 1024);

  std::vector<std::uint32_t> program = {
      encode_i(0x01, 0, 1, 5),       // ADDI r1, r0, 5
      encode_i(0x01, 0, 2, 7),       // ADDI r2, r0, 7
      encode_r(0x00, 1, 2, 3, 0x00), // ADD  r3, r1, r2
      encode_r(0x00, 3, 1, 4, 0x01), // SUB  r4, r3, r1
      0xFFFFFFFFU,                   // UNKNOWN => stop
  };

  sim.load_program_words(program);
  sim.run(32, true);

  std::cout << "---- 结果寄存器 ----\n";
  std::cout << "r1 = " << sim.state().read_reg(1) << "\n";
  std::cout << "r2 = " << sim.state().read_reg(2) << "\n";
  std::cout << "r3 = " << sim.state().read_reg(3) << "\n";
  std::cout << "r4 = " << sim.state().read_reg(4) << "\n";
  std::cout << "SR.Z = " << sim.state().sr.z << ", SR.N = " << sim.state().sr.n
            << "\n";

  return 0;
}
