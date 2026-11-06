CC = gcc
CFLAGS = -Wall -Wextra -O2 -Isrc
LDFLAGS = -lshell32
TARGET = build/suna.exe

# Find all source files
SRCS = src/main.c src/flags.c src/dispatch.c src/util.c $(wildcard src/subcommands/*.c)

# Map every src/folder/name.c to build/obj/name.o
OBJS = $(addprefix build/obj/, $(notdir $(SRCS:.c=.o)))

all: $(TARGET)

# Link the exe
$(TARGET): $(OBJS)
	@if not exist build mkdir build
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

# Compile files in the root src/ folder
build/obj/%.o: src/%.c
	@if not exist build\obj mkdir build\obj
	$(CC) $(CFLAGS) -c $< -o $@

# Compile files in the src/subcommands/ folder
build/obj/%.o: src/subcommands/%.c
	@if not exist build\obj mkdir build\obj
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@if exist build rmdir /s /q build
	@if exist src\*.o del src\*.o
	@if exist src\subcommands\*.o del src\subcommands\*.o

.PHONY: all clean