#include <cstdint>
#include <vector>

#include "gtest/gtest.h"

#include "engine/executor.h"

namespace {

std::uint32_t encodeI(std::uint8_t opcode, std::uint8_t rs, std::uint8_t rt,
                      std::uint16_t imm16) {
  return (static_cast<std::uint32_t>(opcode) << 26U) |
         (static_cast<std::uint32_t>(rs) << 21U) |
         (static_cast<std::uint32_t>(rt) << 16U) |
         static_cast<std::uint32_t>(imm16);
}

std::uint32_t encodeR(std::uint8_t opcode, std::uint8_t rs, std::uint8_t rt,
                      std::uint8_t rd, std::uint8_t funct) {
  return (static_cast<std::uint32_t>(opcode) << 26U) |
         (static_cast<std::uint32_t>(rs) << 21U) |
         (static_cast<std::uint32_t>(rt) << 16U) |
         (static_cast<std::uint32_t>(rd) << 11U) |
         static_cast<std::uint32_t>(funct);
}

std::uint32_t encodeJ(std::uint8_t opcode, std::uint32_t target26) {
  return (static_cast<std::uint32_t>(opcode) << 26U) | (target26 & 0x03FFFFFFU);
}

loongrisc::Simulator makeSim(const std::vector<std::uint32_t>& program) {
  loongrisc::Simulator sim(4096);
  sim.loadProgramWords(program, 0);
  return sim;
}

} // namespace

TEST(SimInstructionTest, AddWorks) {
  auto sim = makeSim({encodeI(0x01, 0, 1, 10), encodeI(0x01, 0, 2, 20),
                      encodeR(0x00, 1, 2, 3, 0x00)});
  sim.run(3);
  EXPECT_EQ(sim.state().readReg(3), 30U);
}

TEST(SimInstructionTest, SubWorks) {
  auto sim = makeSim({encodeI(0x01, 0, 1, 20), encodeI(0x01, 0, 2, 9),
                      encodeR(0x00, 1, 2, 3, 0x01)});
  sim.run(3);
  EXPECT_EQ(sim.state().readReg(3), 11U);
}

TEST(SimInstructionTest, AndOrXorWork) {
  auto sim =
      makeSim({encodeI(0x01, 0, 1, 0x0F0F), encodeI(0x01, 0, 2, 0x00FF),
               encodeR(0x00, 1, 2, 3, 0x02), encodeR(0x00, 1, 2, 4, 0x03),
               encodeR(0x00, 1, 2, 5, 0x04)});
  sim.run(5);
  EXPECT_EQ(sim.state().readReg(3), 0x000F);
  EXPECT_EQ(sim.state().readReg(4), 0x0FFF);
  EXPECT_EQ(sim.state().readReg(5), 0x0FF0);
}

TEST(SimInstructionTest, AddiSupportsSignedImmediate) {
  auto sim = makeSim({encodeI(0x01, 0, 1, 3), encodeI(0x01, 1, 1, 0xFFFF)});
  sim.run(2);
  EXPECT_EQ(sim.state().readReg(1), 2U);
}

TEST(SimInstructionTest, LwAndSwWork) {
  auto sim = makeSim({encodeI(0x01, 0, 1, 256), encodeI(0x01, 0, 2, 0x1234),
                      encodeI(0x07, 1, 2, 0), encodeI(0x06, 1, 3, 0)});
  sim.run(4);
  EXPECT_EQ(sim.state().readReg(3), 0x1234U);
}

TEST(SimInstructionTest, BeqTakenSkipsInstruction) {
  auto sim = makeSim({encodeI(0x01, 0, 1, 7), encodeI(0x01, 0, 2, 7),
                      encodeI(0x09, 1, 2, 1), encodeI(0x01, 0, 3, 99),
                      encodeI(0x01, 0, 3, 42)});
  sim.run(5);
  EXPECT_EQ(sim.state().readReg(3), 42U);
}

TEST(SimInstructionTest, BneTakenSkipsInstruction) {
  auto sim = makeSim({encodeI(0x01, 0, 1, 7), encodeI(0x01, 0, 2, 8),
                      encodeI(0x0A, 1, 2, 1), encodeI(0x01, 0, 3, 99),
                      encodeI(0x01, 0, 3, 42)});
  sim.run(5);
  EXPECT_EQ(sim.state().readReg(3), 42U);
}

TEST(SimInstructionTest, JumpsToTarget) {
  auto sim = makeSim({encodeJ(0x0D, 3), encodeI(0x01, 0, 1, 11),
                      encodeI(0x01, 0, 1, 22), encodeI(0x01, 0, 1, 33)});
  sim.run(2);
  EXPECT_EQ(sim.state().readReg(1), 33U);
}

TEST(SimInstructionTest, JalWritesLinkRegister) {
  auto sim = makeSim({encodeJ(0x0E, 2), encodeI(0x01, 0, 1, 11),
                      encodeI(0x01, 0, 1, 44), 0xFFFFFFFFU});
  sim.run(3);
  EXPECT_EQ(sim.state().readReg(14), 4U);
  EXPECT_EQ(sim.state().readReg(1), 44U);
}

TEST(SimInstructionTest, JrUsesRegisterAsPc) {
  auto sim = makeSim({encodeI(0x01, 0, 1, 12), encodeR(0x08, 1, 0, 0, 0x00),
                      encodeI(0x01, 0, 2, 99), encodeI(0x01, 0, 2, 55)});
  sim.run(3);
  EXPECT_EQ(sim.state().readReg(2), 55U);
}

TEST(SimInstructionTest, JalrWritesRdAndJumps) {
  auto sim = makeSim({encodeI(0x01, 0, 1, 12), encodeR(0x08, 1, 0, 5, 0x01),
                      encodeI(0x01, 0, 2, 99), encodeI(0x01, 0, 2, 77)});
  sim.run(3);
  EXPECT_EQ(sim.state().readReg(5), 8U);
  EXPECT_EQ(sim.state().readReg(2), 77U);
}

TEST(SimInstructionTest, WriteToR0IsIgnored) {
  auto sim = makeSim({encodeI(0x01, 0, 0, 123)});
  sim.run(1);
  EXPECT_EQ(sim.state().readReg(0), 0U);
}

TEST(SimInstructionTest, UnknownInstructionStopsStep) {
  auto sim = makeSim({0xFFFFFFFFU});
  EXPECT_FALSE(sim.step());
}
