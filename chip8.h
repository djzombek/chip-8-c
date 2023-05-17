#include <stdio.h>
#include <stdint.h>

struct cpu {
    uint16_t opcode;
    uint8_t memory[4096];
    uint8_t V[16];
    uint16_t i;
    uint16_t pc;
    uint8_t gfx[64*32];
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint16_t stack[16];
    uint16_t sp;
    uint8_t key[16];
};