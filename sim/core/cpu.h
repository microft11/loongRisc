#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace loongrisc {

struct StatusRegister {
  bool z = false;
  bool n = false;
};

class Memory {
public:
  explicit Memory(std::size_t bytes);

  std::uint32_t loadWord(std::uint32_t addr) const;
  void storeWord(std::uint32_t addr, std::uint32_t value);
  std::size_t size() const {
    return data_.size();
  }

private:
  void checkAligned(std::uint32_t addr) const;
  void checkBounds(std::uint32_t addr) const;
  std::vector<std::uint8_t> data_;
};

class CpuState {
public:
  explicit CpuState(std::size_t memoryBytes);

  std::uint32_t readReg(std::uint8_t idx) const;
  void writeReg(std::uint8_t idx, std::uint32_t value);

  std::uint32_t pc = 0;
  StatusRegister sr{};
  Memory memory;

  static constexpr std::size_t kRegisterCount = 16;

private:
  std::array<std::uint32_t, kRegisterCount> regs_{};
};

void updateZnFlags(CpuState& state, std::uint32_t result);

} // namespace loongrisc
