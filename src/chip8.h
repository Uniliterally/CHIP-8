// CHIP-8 Language Interpreter

#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <array>
#include <random>

namespace Chip8
{
    static constexpr const uint8_t fontHeight   = 5;
    static constexpr const uint8_t screenWidth  = 64;
    static constexpr const uint8_t screenHeight = 32;

    // Screen is monochrome, 1 bit per pixel
    static constexpr const uint16_t screenSizePixels = screenWidth * screenHeight;
	static constexpr const uint16_t screenSizeBytes  = screenSizePixels / 8;
    static constexpr const uint16_t memorySize   = 0x1000;
    static constexpr const uint16_t programStart = 0x0200;
    

    struct Registers {
        uint8_t  V[16];  // General Purpose Registers
        uint8_t  DT;     // Delay Timer
        uint8_t  ST;     // Sound Timer
        uint16_t I;      // Address Register

        // Pseudo Registers (Can't be accessed from program)
        uint8_t  SP;     // Stack Pointer
        uint16_t PC;     // Program Counter
        uint16_t STK[16]; // Stack
    };

    union Memory {
        // CHIP-8 has 1KB of memory
        uint8_t byte[memorySize];

        // First 0x200 bytes are reserved for interpreter
        struct {
            struct Registers reg;
            std::array<uint8_t, (16 * fontHeight)> font;
            uint8_t screen[screenSizeBytes];
			uint8_t keys[16];
        };
    };

    // For ease of splitting opcodes
    union OPCode {
        uint16_t opcode;
        
        struct bytes {
            uint8_t lo;
            uint8_t hi;
        } bytes;

        struct nybbles {
            uint8_t d : 4;
            uint8_t c : 4;
            uint8_t b : 4;
            uint8_t a : 4;
        } nybbles;
    };

    class CPU {
        public:
            CPU();
            ~CPU();

            void ExecuteStep();
            void LoadProgram(uint8_t* bufIn, uint16_t bufLen);
			void GetScreen(uint8_t*  bufOut, uint16_t bufLen);

			bool GetDrawFlag() { return draw; }
			void ClearDrawFlag() { draw = false; }
			void SetKeyUp(uint8_t key)   { mem.keys[key] = 0; }
			void SetKeyDown(uint8_t key);
        private:
            Memory mem;
            std::default_random_engine rand;
            std::uniform_int_distribution<short> randDist = std::uniform_int_distribution<short>(0, 0xFF);
			bool draw;
			bool keyWait;
			uint8_t keyReg;
			uint8_t timerDec;
    };
}

#endif // CHIP8_H
