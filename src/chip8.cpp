// CHIP-8 Instruction Set

#include "chip8.h"

#include <string.h>

namespace Chip8
{
    CPU::CPU() : draw(false), keyWait(false), timerDec(0)
    {
        memset(mem.byte, 0, memorySize);
        mem.reg.PC = programStart;

        // Initialize font:
        mem.font = { 0xF0, 0x90, 0x90, 0x90, 0xF0,    // 0
                     0x20, 0x60, 0x20, 0x20, 0x70,    // 1
                     0xF0, 0x10, 0xF0, 0x80, 0xF0,    // 2
                     0xF0, 0x10, 0xF0, 0x10, 0xF0,    // 3
                     0x90, 0x90, 0xF0, 0x10, 0x10,    // 4
                     0xF0, 0x80, 0xF0, 0x10, 0xF0,    // 5
                     0xF0, 0x80, 0xF0, 0x90, 0xF0,    // 6
                     0xF0, 0x10, 0x20, 0x40, 0x40,    // 7
                     0xF0, 0x90, 0xF0, 0x90, 0xF0,    // 8
                     0xF0, 0x90, 0xF0, 0x10, 0xF0,    // 9
                     0xF0, 0x90, 0xF0, 0x90, 0x90,    // A
                     0xE0, 0x90, 0xE0, 0x90, 0xE0,    // B
                     0xF0, 0x80, 0x80, 0x80, 0xF0,    // C
                     0xE0, 0x90, 0x90, 0x90, 0xE0,    // D
                     0xF0, 0x80, 0xF0, 0x80, 0xF0,    // E
                     0xE0, 0x80, 0xF0, 0x80, 0x80 };  // F
    }

    CPU::~CPU()
    {
    }

    void CPU::ExecuteStep()
    {
		if (keyWait) {
			return;
		}

		// Timers run at 60Hz, CPU runs at 500Hz
		if (timerDec++ > 9) { // TODO: Time this properly
			timerDec = 0;
			// If timers are greater than 0, decrement
			mem.reg.DT -= mem.reg.DT > 0 ? 1 : 0;
			mem.reg.ST -= mem.reg.ST > 0 ? 1 : 0;
		}

        // Read OP Code, increment program counter
        OPCode inst;
        inst.bytes.hi = mem.byte[mem.reg.PC++];
        inst.bytes.lo = mem.byte[mem.reg.PC++];

        switch (inst.nybbles.a) {
        case 0x0: {
            if (inst.bytes.lo == 0xE0) { // CLS
                // Clear the display
				draw = true;
                for (auto& screenByte: mem.screen) {
                    screenByte = 0;
                }
            } else if (inst.bytes.lo == 0xEE) { // RET
                // Set program counter to top value of stack
                if (mem.reg.SP > 0) {
                    // SP is the next free stack address,
                    // so pre-decrement to access last entry
                    mem.reg.PC = mem.reg.STK[--mem.reg.SP];
                }
            }
        } break;
        case 0x1: { // JUMP
            // 0x1nnn : Jump to addr
            mem.reg.PC = inst.opcode & 0x0FFF;
        } break;
        case 0x2: { // CALL
            // 0x2nnn
            // Place Program Counter on stack and jump to new position
            if (mem.reg.SP < 16) {
                mem.reg.STK[mem.reg.SP++] = mem.reg.PC;
                mem.reg.PC = inst.opcode & 0x0FFF;
            }
        } break;
        case 0x3: { // SE byte
            // 0x3xkk : Skip next (2-byte) instruction if V[x] == kk
            if (mem.reg.V[inst.nybbles.b] == inst.bytes.lo) {
                mem.reg.PC += 2;
            }
        } break;
        case 0x4: { // SNE byte
            // 0x4xkk : Skip next (2-byte) instruction if V[x] != kk
            if (mem.reg.V[inst.nybbles.b] != inst.bytes.lo) {
                mem.reg.PC += 2;
            }
        } break;
        case 0x5: { // SE XY
            // 0x5xy0 : Skip next (2-byte) instruction if V[x] == V[y]
            if (mem.reg.V[inst.nybbles.b] == mem.reg.V[inst.nybbles.c]) {
                mem.reg.PC += 2;
            }
        } break;
        case 0x6: { // LD Byte
            // 0x6xkk : V[x] = kk
            mem.reg.V[inst.nybbles.b] = inst.bytes.lo;
        } break;
        case 0x7: { // ADD
            // 0x7xkk : V[x] += kk
            mem.reg.V[inst.nybbles.b] += inst.bytes.lo;
        } break;
        case 0x8: {
            // 0x8xyz
            // Everyone loves nested switch statements!
            switch (inst.nybbles.d) {
            case 0x0: { // LD XY
                mem.reg.V[inst.nybbles.b] = mem.reg.V[inst.nybbles.c];
            } break;
            case 0x1: { // OR
                mem.reg.V[inst.nybbles.b] |= mem.reg.V[inst.nybbles.c];
            } break;
            case 0x2: { // AND
                mem.reg.V[inst.nybbles.b] &= mem.reg.V[inst.nybbles.c];
            } break;
            case 0x3: { // XOR
                mem.reg.V[inst.nybbles.b] ^= mem.reg.V[inst.nybbles.c];
            } break;
            case 0x4: { // ADD XY
                // V[0xF] = Carry
                mem.reg.V[0xF] = (mem.reg.V[inst.nybbles.b] + mem.reg.V[inst.nybbles.c] > 0xFF);
                mem.reg.V[inst.nybbles.b] += mem.reg.V[inst.nybbles.c];
            } break;
            case 0x5: { // SUB XY
                // V[0xF] = !Borrow
                mem.reg.V[0xF] = (mem.reg.V[inst.nybbles.b] > mem.reg.V[inst.nybbles.c]);
                mem.reg.V[inst.nybbles.b] -= mem.reg.V[inst.nybbles.c];
            } break;
            case 0x6: { // SHR
                // V[0xF] = LSB, then div by 2
                mem.reg.V[0xF] = mem.reg.V[inst.nybbles.b] & 0x01;
                mem.reg.V[inst.nybbles.b] /= 2;
            } break;
            case 0x7: { // SUBN
                // V[0xF] = !Borrow
                mem.reg.V[0xF] = (mem.reg.V[inst.nybbles.c] > mem.reg.V[inst.nybbles.b]);
                mem.reg.V[inst.nybbles.b] = mem.reg.V[inst.nybbles.c] - mem.reg.V[inst.nybbles.b];
            } break;
            case 0xE: { // SHL
                // V[0xF] = MSB, then mul by 2
                mem.reg.V[0xF] = mem.reg.V[inst.nybbles.b] & 0x80 ? 1 : 0;
                mem.reg.V[inst.nybbles.b] *= 2;
            } break;
            default: break;
            }
        
        } break;
        case 0x9: { // SNE XY
            // 0x9xy0 : Skip next (2-byte) instruction if V[x] != V[y]
            if (mem.reg.V[inst.nybbles.b] != mem.reg.V[inst.nybbles.c]) {
                mem.reg.PC += 2;
            }
        } break;
        case 0xA: { // LD I
            // 0xAnnn : Load value to I register
            mem.reg.I = inst.opcode & 0x0FFF;
        } break;
        case 0xB: { // JP V0
            // 0xBnnn : Jump to addr with V[0] offset
            mem.reg.PC = mem.reg.V[0] + (inst.opcode & 0x0FFF);
        } break;
        case 0xC: { // RND
            // V[x] = rand & kk
            mem.reg.V[inst.nybbles.b] = randDist(rand) & inst.bytes.lo;
        } break;
        case 0xD: { // DRW
            // 0xDxyn : Read n bytes from memory starting from I
            //          Display as sprites on screen at co-ordinates V[x], V[y]
            //          Sprite is XOR-ed into screen memory
            //          If any screen pixels flip from 1 to 0, V[0xF] is set
			draw = true;
            uint16_t pixelCoord = mem.reg.V[inst.nybbles.b] + (mem.reg.V[inst.nybbles.c] * screenWidth);
            mem.reg.V[0xF] = 0;

            for (uint8_t vLine = 0; vLine < inst.nybbles.d; ++vLine) {
                uint8_t currentByte = mem.byte[mem.reg.I + vLine];

                for (uint8_t hLine = 0; hLine < 8; ++hLine) {
                    uint16_t currentPixel = pixelCoord + hLine + (vLine * screenWidth);

                    // Since we're XOR-ing, data only changes on 1-bits
                    if (currentByte & (0x80 >> hLine)) {
						if (mem.reg.V[0xF] == 0) {
							// We're detecting a flip from 1 to 0 if the
							// pre-flip state is 1, just copy it
							if (mem.screen[currentPixel / 8] & (0x80 >> (currentPixel % 8))) {
								mem.reg.V[0xF] = 1;
							}
                        }

                        // XOR bit
                        mem.screen[currentPixel / 8] ^= (0x80 >> (currentPixel % 8));
                    }
                }
            }
        } break;
        case 0xE: {
            if (inst.bytes.lo == 0x9E) { // SKP Vx
				if (mem.keys[inst.nybbles.b]) {
					mem.reg.PC += 2;
				}
            } else if (inst.bytes.lo == 0xA1) { // SKNP Vx
				if (!mem.keys[inst.nybbles.b]) {
					mem.reg.PC += 2;
				}
            }
        } break;
        case 0xF: {
            // More nested switches yayy!
            switch (inst.bytes.lo) {
            case 0x07: { // LD Vx, DT
                mem.reg.V[inst.nybbles.b] = mem.reg.DT;
            } break;
            case 0x0A: { // LD Vx, K
				keyWait = true;
				keyReg = inst.nybbles.b;
            } break;
            case 0x15: { // LD DT, Vx
                mem.reg.DT = mem.reg.V[inst.nybbles.b];
            } break;
            case 0x18: { // LD ST, Vx
                mem.reg.ST = mem.reg.V[inst.nybbles.b];
            } break;
            case 0x1E: { // ADD I, Vx
                mem.reg.I += mem.reg.V[inst.nybbles.b];
            } break;
            case 0x29: { // LD F, Vx
                // Set I to address of font character x
                // NOTE: font addr - mem addr = font addr as offset
                mem.reg.I = &mem.font[mem.reg.V[inst.nybbles.b] * fontHeight] - mem.byte;
            } break;
            case 0x33: { // LD B, Vx
                // Store BCD of V[x] into I, I+1, I+2
                mem.byte[mem.reg.I]   =  mem.reg.V[inst.nybbles.b] / 100;
                mem.byte[mem.reg.I+1] = (mem.reg.V[inst.nybbles.b] / 10) % 10;
                mem.byte[mem.reg.I+2] =  mem.reg.V[inst.nybbles.b] % 10;
            } break;
            case 0x55: { // LD [I], Vx
                // Store registers V[0] to V[x] in memory starting at I
                for (int i = 0; i <= inst.nybbles.b; ++i) {
                    mem.byte[mem.reg.I+i] = mem.reg.V[i];
                }
            } break;
            case 0x65: { // LD Vx, [I]
                // Read into registers V[0] to V[x] from memory starting at I
                for (int i = 0; i <= inst.nybbles.b; ++i) {
                    mem.reg.V[i] = mem.byte[mem.reg.I+i];
                }
            } break;
            default: break;
            }
        } break;
        default: break;
        }
    }

    void CPU::LoadProgram(uint8_t* bufIn, uint16_t bufLen)
    {
        if (bufIn != NULL && bufLen < memorySize - programStart) {
            memcpy(&mem.byte[programStart], bufIn, bufLen);
        }
    }

	void CPU::GetScreen(uint8_t* bufOut, uint16_t bufLen)
	{
		if (bufOut != NULL && bufLen >= screenSizeBytes) {
			memcpy(bufOut, mem.screen, screenSizeBytes);
		}
	}

	void CPU::SetKeyDown(uint8_t key)
	{
		if (keyWait) {
			keyWait = false;
			mem.reg.V[keyReg] = key;
		}

		mem.keys[key] = 1;
	}
}
