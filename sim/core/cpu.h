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

  std::uint32_t load_word(std::uint32_t addr) const;
  void store_word(std::uint32_t addr, std::uint32_t value);
  std::size_t size() const {
    return data_.size();
  }

private:
  void check_aligned(std::uint32_t addr) const;
  void check_bounds(std::uint32_t addr) const;
  std::vector<std::uint8_t> data_;
};

class CpuState {
public:
  explicit CpuState(std::size_t memory_bytes);

  std::uint32_t read_reg(std::uint8_t idx) const;
  void write_reg(std::uint8_t idx, std::uint32_t value);

  std::uint32_t pc = 0;
  StatusRegister sr{};
  Memory memory;

  static constexpr std::size_t kRegisterCount = 16;

private:
  std::array<std::uint32_t, kRegisterCount> regs_{};
};

void update_zn_flags(CpuState& state, std::uint32_t result);

} // namespace loongrisc
