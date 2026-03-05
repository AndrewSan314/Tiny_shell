# Makefile for MSH Shell v2.0 - Modular Version

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I./include
TARGET = msh.exe
SRCDIR = src

# Source files
SOURCES = $(SRCDIR)/main.c \
          $(SRCDIR)/core.c \
          $(SRCDIR)/colors.c \
          $(SRCDIR)/history.c \
          $(SRCDIR)/readline.c \
          $(SRCDIR)/alias.c \
          $(SRCDIR)/pipe_redirect.c \
          $(SRCDIR)/utils.c \
          $(SRCDIR)/config.c \
          $(SRCDIR)/hacker.c \
          $(SRCDIR)/process_manager.c \
          $(SRCDIR)/builtins.c \
          $(SRCDIR)/launcher.c

# Object files
OBJECTS = $(SOURCES:.c=.o)

#============================================================
# BUILD TARGETS
#============================================================

.PHONY: all clean run test

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@echo Linking...
	$(CC) $(OBJECTS) -o $(TARGET)
	@echo Build successful!

%.o: %.c
	@echo Compiling $<...
	$(CC) $(CFLAGS) -c $< -o $@

#============================================================
# UTILITY TARGETS
#============================================================

clean:
ifeq ($(OS),Windows_NT)
	@if exist $(SRCDIR)\*.o del /Q $(SRCDIR)\*.o
	@if exist $(TARGET) del /Q $(TARGET)
	@echo Cleaned!
else
	rm -f $(OBJECTS) $(TARGET)
	@echo Cleaned!
endif

run: $(TARGET)
	./$(TARGET)

test: $(TARGET)
	powershell -ExecutionPolicy Bypass -File tests/test_lsh.ps1

#============================================================
# DEPENDENCIES
#============================================================

$(SRCDIR)/main.o: $(SRCDIR)/main.c $(SRCDIR)/core.h $(SRCDIR)/process_manager.h $(SRCDIR)/colors.h
$(SRCDIR)/core.o: $(SRCDIR)/core.c $(SRCDIR)/core.h $(SRCDIR)/builtins.h $(SRCDIR)/launcher.h $(SRCDIR)/process_manager.h $(SRCDIR)/colors.h
$(SRCDIR)/colors.o: $(SRCDIR)/colors.c $(SRCDIR)/colors.h include/common.h
$(SRCDIR)/process_manager.o: $(SRCDIR)/process_manager.c $(SRCDIR)/process_manager.h $(SRCDIR)/colors.h include/common.h
$(SRCDIR)/builtins.o: $(SRCDIR)/builtins.c $(SRCDIR)/builtins.h $(SRCDIR)/process_manager.h $(SRCDIR)/colors.h include/common.h
$(SRCDIR)/launcher.o: $(SRCDIR)/launcher.c $(SRCDIR)/launcher.h $(SRCDIR)/process_manager.h $(SRCDIR)/colors.h include/common.h