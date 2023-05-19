CC = gcc
SRC = chip8.c
TARGET = chip8
CFLAGS = -std=c11 -pedantic -Wall -Wextra -flto -march=native -mtune=native -O2
LDFLAGS = -lSDL2 -lSDL2_mixer

$(TARGET):	$(SRC)
	$(CC) $(SRC) $(CFLAGS) $(LDFLAGS) -o $(TARGET)