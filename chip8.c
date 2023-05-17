#include <stdio.h>
#include <stdint.h>
#include "chip8.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>



void initialize();
void emulatecycle();

int main(int argc, char **argv[]) {
    //setup_graphics();
    //setup_input();
    //initialize(mycpu);
    //loadgame(mycpu);

    /*emulation loop:
    for () {
       emulatecycle(mycpu);
       if (drawflag(mycpu)) {
           drawGraphics();
       } 
       setKeys(mycpu);
    }*/
    return 0;
}

void initialize() {
    pc = 0x200;
    opcode = 0;
    I = 0;
    sp = 0;
    dtimer = 0;
    stimer = 0;
    memset(stack, 0, 16);
    memset(memory, 0, 4096);
    memset(gfx, 0, 64*32);
    memset(key, 0, 16);
    memset(V, 0, 16);
    for (int i = 0; i < 80; ++i) {
        memory[i] = chip8_fontset[i];
    }
}

int load_rom(char *filename) { 
    FILE *rom = fopen(filename, "rb");
    fseek(rom, 0, SEEK_END);
    int size = ftell(rom);
    fseek(rom, 0, SEEK_SET);
    fread(memory+512, 1, sizeof(memory) - 512, rom);
    fclose(rom);
    return 0;
}

void emulatecycle() {
    opcode = memory[pc] << 8 | memory[pc + 1];
    uint16_t nnn = opcode & 0x0FFF;
    uint8_t nn = opcode & 0x00FF;
    uint8_t n = opcode & 0x000F;
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    srand(time(0));
    switch (opcode & 0xF000) {
        case 0x1000:
            pc = nnn;
            break;
        case 0x2000:
            stack[sp] = pc;
            sp += 1;
            pc = nnn;
            break;
        case 0x3000:
            pc += (V[x] == nn) ? 4 : 2;
            break;
        case 0x4000:
            pc += (V[x] != nn) ? 4 : 2;
            break;
        case 0x5000:
            pc += (V[x] == V[y]) ? 4 : 2;
            break;
        case 0x6000:
            V[x] = nn;
            pc += 2;
            break;
        case 0x7000:
            V[x] += nn;
            pc += 2;
            break;
        case 0x8000:
            switch (n) {
                case 0x0:
                    V[x] = V[y];
                    pc += 2;
                    break;
                case 0x1:
                    V[x] = (V[x] | V[y]);
                    pc += 2;
                    break;
                case 0x2:
                    V[x] = (V[x] & V[y]);
                    pc += 2;
                    break;
                case 0x3:
                    V[x] = (V[x] ^ V[y]);
                    pc += 2;
                    break;
                case 0x4:
                    uint16_t vf = V[x] + V[y];
                    V[x] += V[y];
                    V[15] = vf > 255 ? 1 : 0; 
                    pc += 2;
                    break;
                case 0x5:
                    uint16_t vf = V[x] >= V[y] ? 1 : 0;
                    V[x] -= V[y];
                    V[15] = vf;
                    pc += 2;
                    break;
                case 0x6:
                    uint16_t vf = V[y] & 0x1;
                    V[x] = V[y] >> 1;
                    V[15] = vf;
                    pc += 2;
                    break;
                case 0x7:
                    uint16_t vf = (V[y] >= V[x]) ? 1 : 0; 
                    V[x] = V[y] - V[x];
                    V[15] = vf;
                    pc += 2;
                    break;
                case 0xE:
                    uint16_t vf = (V[y] >> 7) & 0x1;
                    V[x] = V[y] << 1;
                    V[15] = vf;
                    pc += 2;
                    break;
            }   
        case 0x9000:
            pc += (V[x] != V[y]) ? 4 : 2;
            break;
        case 0xA000:
            I = nnn;
            pc += 2;
            break;
        case 0xB000:
            pc = V[0] + nnn;
            break;
        case 0xC000:
            V[x] = (rand() % 256) & nn;
            pc += 2;
            break;
    switch (opcode & 0x0FFF) {
        case 0x00E0: 
            memset(gfx, 0, 4096);
            df = 1;
            break;
        case 0x00EE:
            sp -= 1;
            pc = stack[sp];
            break;
    }
    }
}