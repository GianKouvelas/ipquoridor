# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -std=c11 $(shell sdl2-config --cflags)

# Linker flags — SDL2 + SDL2_ttf + math
LDFLAGS = $(shell sdl2-config --libs) -lSDL2_ttf -lm

# Executable name
TARGET = quoridor

# Source files
SRC = Main.c ai.c Playmove.c Playwall.c Insert_at_end.c Display_board.c Sorting_wall.c

# Object files
OBJ = $(SRC:.c=.o)

# Default rule
all: $(TARGET)

# Link
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

# Compile (each .c depends on header.h)
%.o: %.c header.h
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(OBJ) $(TARGET)