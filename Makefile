# Use MSYS2 ucrt64 GCC. If gcc is already on your PATH, set CC=gcc.
CC     = C:/msys64/ucrt64/bin/gcc.exe
CFLAGS = -Wall -Wextra -std=c99 -O2 -Isrc
SRC    = src/main.c src/snake.c src/render.c src/input.c src/game.c
TARGET = snake.exe

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: all
	./$(TARGET)

clean:
	del /f $(TARGET) 2>nul || rm -f $(TARGET)
