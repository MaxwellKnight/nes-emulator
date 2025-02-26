#include <iostream>
#include <vector>
#include "../include/nes.h"

int main() {
  // Fibonacci sequence calculator with correct memory storage
  std::vector<uint8_t> program = {
    0xA9, 0x00,  // LDA #$00
    0x85, 0x30,  // STA $30
    0xA9, 0x01,  // LDA #$01
    0x85, 0x31,  // STA $31

    0xA2, 0x02,  // LDX #$02 (Start index at 2)

    0xA5, 0x30,  // LDA $30 - Load second-to-last Fibonacci number
    0x18,        // CLC - Clear carry before addition
    0x65, 0x31,  // ADC $31 - Add last Fibonacci number
    0x95, 0x32,  // STA $32,X - Store result (new Fibonacci number)

    // Shift values for the next iteration
    0xA5, 0x31,  // LDA $31 - Load last number
    0x85, 0x30,  // STA $30 - Move it to second-to-last
    0xA5, 0x32,  // LDA $32 - Load newly computed Fibonacci number
    0x85, 0x31,  // STA $31 - Move it to last

    0xE8,        // INX
    0xE0, 0x0A,  // CPX #$0A
    0xD0, 0xEE,  // BNE Loop_Fibonacci (Jump back if X < 10)

    0xA9, 0x00,  // LDA #$00 (Start sum at 0)
    0xA2, 0x00,  // LDX #$00 (Start index at 0)

    0x18,        // CLC - Clear carry
    0x75, 0x30,  // ADC $30,X (Add Fibonacci number)
    0xE8,        // INX - Increment index
    0xE0, 0x0A,  // CPX #$0A - Check if done
    0xD0, 0xF9,  // BNE Sum_Loop (-7 bytes back to loop)

    0x8D, 0x2F, 0x00,  // STA $002F (Store sum)
    0xEA               // NOP
  };

  NES nes(true);
  nes.load_program(program);
  nes.execute(200);
  nes.print_results();
  std::cout << "\nFibonacci Sequence:" << std::endl;
  nes.print_zero_page(0x30, 0x39);
  std::cout << "\nSum of Fibonacci numbers:" << std::endl;
  nes.print_zero_page(0x2F, 0x2F);
  return 0;
}
