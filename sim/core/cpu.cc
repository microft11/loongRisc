#include "core/cpu.h"

#include <stdexcept>

namespace loongrisc {

Memory::Memory(std::size_t bytes) : data_(bytes, 0) {}

void Memory::checkAligned(std::uint32_t addr) const {
  if ((addr & 0x3U) != 0U) {
    throw std::runtime_error("memory access is not 4-byte aligned");
  }
}

void Memory::checkBounds(std::uint32_t addr) const {
  if (addr + 3U >= data_.size()) {
    throw std::runtime_error("memory access out of bounds");
  }
}

std::uint32_t Memory::loadWord(std::uint32_t addr) const {
  checkAligned(addr);
  checkBounds(addr);
  return static_cast<std::uint32_t>(data_[addr]) |
         (static_cast<std::uint32_t>(data_[addr + 1U]) << 8U) |
         (static_cast<std::uint32_t>(data_[addr + 2U]) << 16U) |
         (static_cast<std::uint32_t>(data_[addr + 3U]) << 24U);
}

void Memory::storeWord(std::uint32_t addr, std::uint32_t value) {
  checkAligned(addr);
  checkBounds(addr);
  data_[addr] = static_cast<std::uint8_t>(value & 0xFFU);
  data_[addr + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
  data_[addr + 2U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
  data_[addr + 3U] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
}

CpuState::CpuState(std::size_t memoryBytes) : memory(memoryBytes) {
  regs_.fill(0);
}

std::uint32_t CpuState::readReg(std::uint8_t idx) const {
  if (idx >= kRegisterCount) {
    throw std::runtime_error("invalid register index");
  }
  return regs_[idx];
}

void CpuState::writeReg(std::uint8_t idx, std::uint32_t value) {
  if (idx >= kRegisterCount) {
    throw std::runtime_error("invalid register index");
  }
  if (idx == 0) {
    return;
  }
  regs_[idx] = value;
}

void updateZnFlags(CpuState& state, std::uint32_t result) {
  state.sr.z = (result == 0U);
  state.sr.n = ((result & 0x80000000U) != 0U);
}

} // namespace loongrisc
