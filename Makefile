# Define the compiler
CC = gcc

# Define the flags
CFLAGS = -O2

# Define the source files
SRC = memory_manager.c bitmap.c

# Define the object files
OBJ = memory_manager.o bitmap.o

# Define the target static library
TARGET_LIB = memory_manager.a

# Default target
all: $(TARGET_LIB)

# Rule to build the static library
$(TARGET_LIB): $(OBJ)
	@echo "Creating static library $(TARGET_LIB)..."
	ar rcs $(TARGET_LIB) $(OBJ)
	@echo "Static library $(TARGET_LIB) created successfully."

# Rule to build object files
%.o: %.c
	@echo "Compiling $< to create $@..."
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up generated files
clean:
	@echo "Cleaning up object files and static library..."
	rm -f $(OBJ) $(TARGET_LIB)
	@echo "Clean completed."

# Phony targets
.PHONY: all clean
