CC     = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -Isrc
SRC    = src/main.c src/snake.c src/render.c src/input.c src/game.c
TARGET = snake.exe

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	del /f $(TARGET) 2>nul || true
