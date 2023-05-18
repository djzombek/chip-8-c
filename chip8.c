#include <stdio.h>
#include <stdint.h>
#include "chip8.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <SDL2/SDL.h>

SDL_Window* screen = NULL;
SDL_Renderer* screen_draw = NULL;

void initialize();
void emulatecycle();


void setkeys() {
    const uint8_t *keydown = SDL_GetKeyboardState(NULL);
    SDL_PumpEvents();
    for (int i = 0; i < 16; i++) {
        key[i] = keydown[keymap[i]];
    }
}



void createscreen() {

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        const char *error = SDL_GetError();
        printf("unable to start sdl {%s}", error);
        exit(-1);
    }
    printf("sdl started\n");
    screen = SDL_CreateWindow("chip-8 emu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 64 * 15, 32 * 15, SDL_WINDOW_RESIZABLE);
    if (!screen) {
        printf("screen unable to start\n");
        exit(-1);
    }
    printf("screen started\n");
    screen_draw = SDL_CreateRenderer(screen, -1, 0);
    if (!screen_draw) {
        printf("screen unable to be drawn\n");
        exit(-1);
    }
    printf("render created\n");
}

void updatescreen() {
    
    SDL_SetRenderDrawColor(screen_draw, 128, 128, 128, 255);
    SDL_RenderClear(screen_draw);
    SDL_SetRenderDrawColor(screen_draw, 0, 0, 0, 255);
    int32_t width = 0;
    int32_t height = 0;
    SDL_GetWindowSize(screen, &width, &height);
    int32_t scalew = width/64;
    int32_t scaleh = height/32;
    for(int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if(gfx[x][y]) {
                SDL_Rect object;
                object.x = x *scalew;
                object.y = y * scaleh;
                object.w = scalew;
                object.h = scaleh;
                SDL_RenderFillRect(screen_draw, &object);
                //printf("creating object\n");
            }
        }
    }
    
    SDL_RenderPresent(screen_draw);
    printf("screen rendered\n");

}

void initialize() {
    pc = 0x200;
    opcode = 0;
    I = 0;
    sp = 0;
    dtimer = 60;
    stimer = 60;
    memset(stack, 0, sizeof(stack));
    memset(memory, 0, sizeof(memory));
    memset(gfx, 0, sizeof(gfx));
    memset(key, 0, sizeof(key));
    memset(V, 0, sizeof(V));
    for (int i = 0; i < 80; ++i) {
        memory[i] = chip8_fontset[i];
    }
    
}

void load_rom(char *filename) { 
    FILE *rom = fopen(filename, "rb\n");
    size_t size = 0;
    if (rom == NULL) {
        printf("error opening file\n\n");
        exit(-1);
    }
    fseek(rom, 0, SEEK_END);
    size = ftell(rom);
    fseek(rom, 0, SEEK_SET);
    size_t result = fread(memory + 512, size, 1, rom);
    if (result != 1) {
        printf("couldn't read rom\n");
        exit(-1);
    }
    fclose(rom);
    rom = NULL;
}

void emulatecycle() {
    //printf("emulating cycle\n\n");
    opcode = memory[pc] << 8 | memory[pc + 1];
    printf("{%.4x}", opcode);
    uint16_t nnn = (opcode & 0x0FFF);
    uint8_t nn = (opcode & 0x00FF);
    uint8_t n = (opcode & 0x000F);
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint16_t vf;
    const uint8_t *keydown = SDL_GetKeyboardState(NULL);
    srand(time(0));
    
    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode) {
                case 0x00E0: 
                    printf("0x00E0\n");
                    memset(gfx, 0, sizeof(gfx));
                    pc += 2;
                    break;
                case 0x00EE:
                    printf("0x00EE\n");
                    --sp;
                    pc = stack[sp];
                    pc += 2;
                    break;
                default:
                    printf("unknown opcode in 0x0000: {%.4x}\n", opcode);
                    exit(-1);
                    break;
            }
        break;
        case 0x1000:
            printf("0x1000\n");
            pc = (opcode & 0x0FFF);
            printf("nnn: {%.4x}", nnn);
            break;
        case 0x2000:
            printf("0x2000\n");
            stack[sp] = pc;
            sp += 1;
            pc = nnn;
            break;
        case 0x3000:
            printf("0x3000\n");
            pc += (V[x] == nn) ? 4 : 2;
            break;
        case 0x4000:
            printf("0x4000\n");
            pc += (V[x] != nn) ? 4 : 2;
            break;
        case 0x5000:
            printf("0x5000\n");
            pc += (V[x] == V[y]) ? 4 : 2;
            break;
        case 0x6000:
            printf("0x6000\n");
            V[x] = nn;
            pc += 2;
            break;
        case 0x7000:
            printf("0x7000\n");
            V[x] += nn;
            pc += 2;
            break;
        case 0x8000:
            switch (n) {
                case 0x0000:
                    printf("0x8000\n");
                    V[x] = V[y];
                    pc += 2;
                    break;
                case 0x0001:
                    printf("0x8001\n");
                    V[x] = (V[x] | V[y]);
                    pc += 2;
                    break;
                case 0x0002:
                    printf("0x8002\n");
                    V[x] = (V[x] & V[y]);
                    pc += 2;
                    break;
                case 0x0003:
                    printf("0x8003\n");
                    V[x] = (V[x] ^ V[y]);
                    pc += 2;
                    break;
                case 0x0004:
                    printf("0x8004\n");
                    vf = V[x] + V[y];
                    V[x] += V[y];
                    V[15] = vf > 255 ? 1 : 0; 
                    pc += 2;
                    break;
                case 0x0005:
                    printf("0x8005\n");
                    vf = V[x] >= V[y] ? 1 : 0;
                    V[x] -= V[y];
                    V[15] = vf;
                    pc += 2;
                    break;
                case 0x0006:
                    printf("0x8006\n");
                    vf = V[y] & 0x1;
                    V[x] = V[y] >> 1;
                    V[15] = vf;
                    pc += 2;
                    break;
                case 0x0007:
                    printf("0x8007\n");
                    vf = (V[y] >= V[x]) ? 1 : 0; 
                    V[x] = V[y] - V[x];
                    V[15] = vf;
                    pc += 2;
                    break;
                case 0x000E:
                    printf("0x800E\n");
                    vf = (V[y] >> 7) & 0x1;
                    V[x] = V[y] << 1;
                    V[15] = vf;
                    pc += 2;
                    break;
                default:
                    printf("unknown opcode in 8000: {%.4x}\n", opcode);
                    pc += 2;
                    break;
            }   
            break;
        case 0x9000:
            printf("0x9000\n");
            pc += (V[x] != V[y]) ? 4 : 2;
            break;
        case 0xA000:
            printf("0xA000\n");
            I = nnn;
            pc += 2;
            break;
        case 0xB000:
            printf("0xB000\n");
            pc = V[0] + nnn;
            break;
        case 0xC000:
            printf("0xC000\n");
            V[x] = (rand() % 256) & nn;
            pc += 2;
            break;
        case 0xD000:
            printf("0xD000\n");
            V[15] = 0;
            uint8_t height = n;
            uint8_t pixel = 0;

            for (uint16_t yLine = 0; yLine < height; yLine++) {
                pixel = memory[I + yLine];
                for (uint16_t xLine = 0; xLine < 8; xLine++) {
                    if ((pixel & (0x80 >> xLine)) != 0) {
                        if (gfx[(V[x] + xLine) % 64][(V[y] + yLine) % 32] == 1) {
                            V[15] = 1;
                        }
                        gfx[(V[x] + xLine) % 64][(V[y] + yLine) % 32] ^= 1;
                    }
                }
            }

            df = 1;
            pc += 2;
            break;
        case 0xE000:
            switch (opcode & 0x00FF) {
                case 0x009E:
                    printf("0xE09E\n");
                    pc += (key[V[x] & 0xF]) ? 4 : 2;
                    break;
                case 0x00A1:
                    printf("0xE0A1\n");
                    pc += (!key[V[x] & 0xF]) ? 4 : 2;
                    break;
                default:
                    printf("unknown opcode in E000: {%.4x}\n", opcode);
                    pc += 2;
                    break;
            }
            break;
        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x0007:
                    printf("0xF007\n");
                    V[x] = dtimer;
                    pc += 2;
                    break;
                case 0x000A:
                    printf("0xF00A\n");
                    keydown = SDL_GetKeyboardState(NULL);
                    SDL_Event(event);
                    SDL_WaitEvent(&event);
                    for (int i = 0; i < 16; i++) {
                        if (keydown[keymap[i]]) {
                            V[x] = i;
                            pc += 2;
                            break;
                        }
                    }
                    break;
                case 0x0015:
                    printf("0xF015\n");
                    dtimer = V[x];
                    pc += 2;
                    break;
                case 0x0018:
                    printf("0xF018\n");
                    stimer = V[x];
                    pc += 2;
                    break;
                case 0x001E:
                    printf("0xF01E\n");
                    I += V[x];
                    pc += 2;
                    break;
                case 0x0029:
                    printf("0xF029\n");
                    I = V[x] * 0x5;
                    pc += 2;
                    break;
                case 0x0033:
                    printf("0xF033\n");
                    memory[I]     = V[x] / 100;
                    memory[I + 1] = (V[x] / 10) % 10;
                    memory[I + 2] = (V[x] % 100) % 10;
                    pc += 2;
                    break;
                case 0x0055:
                    printf("0xF055\n");
                    for (int i = 0; i <= x; i++) {
                        memory[I + i] = V[i];
                    }
                    I += x + 1;
                    pc += 2;
                    break;
                case 0x0065:
                    printf("0xF065\n");
                    for (int i = 0; i <= x; ++i) {
                        V[i] = memory[I + i];
                    }
                    I += x + 1;
                    pc += 2;
                    break;
                default:
                    printf("unknown opcode in F000: {%.4x}\n", opcode);
                    pc += 2;
                    break;
                    
            }
    
    }
}

void closescreen() {
    SDL_DestroyRenderer(screen_draw);
    SDL_DestroyWindow(screen);
    SDL_Quit();
    screen = NULL;
    screen_draw = NULL;
}

int main(int argc, char *argv[]) {
    SDL_Event event;
    uint8_t run = 1;
    uint8_t clock = (uint8_t)(500/60);
    createscreen();
    initialize();
    load_rom(argv[1]);
    if (argc != 2) {
        printf("invalid number of args\n");
        exit(-1);
    }
    while(run) {
        for (int i = 0; i < clock; i++) {
            emulatecycle();
        }
        if (dtimer > 0) {
            dtimer--;
        }

        if (df) {
            updatescreen();
            df = 0;
        }
        
        setkeys();

        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) {
            run = 0;
        }
        SDL_Delay(30);
    
    }
    closescreen();
    return 0;
}