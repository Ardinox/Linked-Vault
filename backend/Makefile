CC = gcc

CFLAGS = -Wall -Iinclude -Ivendor -D_CRT_RAND_S -Dfseeko=fseek

LDFLAGS = -lws2_32 -lpthread

TARGET = LinkedVault.exe

SRC = $(wildcard src/*.c) $(wildcard vendor/*.c)

OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	@echo Linking $(TARGET)...
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)
	@echo [OK] Build Successful!

%.o: %.c
	@echo Compiling $<...
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo Cleaning up...
	-del /Q /S src\*.o vendor\*.o *.exe