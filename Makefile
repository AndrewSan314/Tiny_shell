# Makefile for MSH Shell - Modular Version
# Hỗ trợ Windows với MinGW

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I./include
TARGET = lsh.exe
SRCDIR = src

# Danh sách các source files
SOURCES = $(SRCDIR)/main.c \
          $(SRCDIR)/core.c \
          $(SRCDIR)/process_manager.c \
          $(SRCDIR)/builtins.c \
          $(SRCDIR)/launcher.c

# Tạo danh sách object files
OBJECTS = $(SOURCES:.c=.o)

# Phân công cho từng người:
# - main.c, core.c, core.h         : Person 1
# - process_manager.c/.h           : Person 2
# - builtins.c/.h                  : Person 3
# - launcher.c/.h                  : Person 4

#============================================================
# BUILD TARGETS
#============================================================

.PHONY: all clean run

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

#============================================================
# DEPENDENCIES - Thứ tự compile quan trọng
#============================================================

$(SRCDIR)/main.o: $(SRCDIR)/main.c $(SRCDIR)/core.h $(SRCDIR)/process_manager.h
$(SRCDIR)/core.o: $(SRCDIR)/core.c $(SRCDIR)/core.h $(SRCDIR)/builtins.h $(SRCDIR)/launcher.h $(SRCDIR)/process_manager.h
$(SRCDIR)/process_manager.o: $(SRCDIR)/process_manager.c $(SRCDIR)/process_manager.h include/common.h
$(SRCDIR)/builtins.o: $(SRCDIR)/builtins.c $(SRCDIR)/builtins.h $(SRCDIR)/process_manager.h include/common.h
$(SRCDIR)/launcher.o: $(SRCDIR)/launcher.c $(SRCDIR)/launcher.h $(SRCDIR)/process_manager.h include/common.h