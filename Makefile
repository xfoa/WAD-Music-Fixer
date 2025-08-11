# Makefile for WAD-Music-Fixer

# Compiler and flags
CC := gcc
CFLAGS := -Wall -Wextra -Wno-stringop-truncation -O2

# Source and build files
SRC := $(wildcard src/*.c)
OBJ := $(SRC:.c=.o)
TARGET := wad-music-fixer

# Default target
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean