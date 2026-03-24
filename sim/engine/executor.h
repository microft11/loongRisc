#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "core/cpu.h"
#include "isa/decoder.h"

namespace loongrisc {

class Simulator {
public:
  explicit Simulator(std::size_t memory_bytes);

  void load_program_words(const std::vector<std::uint32_t>& words,
                          std::uint32_t base_addr = 0);
  bool step(bool trace = false);
  void run(std::size_t max_steps, bool trace = false);

  CpuState& state() {
    return state_;
  }
  const CpuState& state() const {
    return state_;
  }

private:
  bool execute(const DecodedInstruction& insn, bool trace);
  CpuState state_;
};

} // namespace loongrisc
