# Makefile for LSH - Windows Compatible
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_CRT_SECURE_NO_WARNINGS -DWIN32_LEAN_AND_MEAN
TARGET = lsh.exe
SRCDIR = src
SOURCES = $(SRCDIR)/main.c
OBJECTS = $(SOURCES:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
ifeq ($(OS),Windows_NT)
	if exist $(SRCDIR)\*.o del /Q $(SRCDIR)\*.o
	if exist $(TARGET) del /Q $(TARGET)
else
	rm -f $(OBJECTS) $(TARGET)
endif

run: $(TARGET)
	./$(TARGET)