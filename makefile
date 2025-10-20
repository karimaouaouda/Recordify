# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2
DEBUG_FLAGS = -g -DDEBUG
TEST_FLAGS = -lcppunit
RM = rm -f
INCLUDE_DIR = include
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
TEST_DIR = tests
TEST_OBJ_DIR = obj/tests
TEST_BIN_DIR = bin/tests

# Modules - add new modules here
MODULES = core screen_handler audio_handler video_handler file_manager ui config utils

#screenshot rule
screenshot:
	clean 
	build
	echo "every things is ready"

# Program name
TARGET = $(BIN_DIR)/recordify.exe
TEST_TARGET = $(TEST_BIN_DIR)/test_runner.exe

# Find all source files recursively in all modules
SOURCES = $(foreach module,$(MODULES),$(wildcard $(SRC_DIR)/$(module)/*.cpp))
HEADERS = $(foreach module,$(MODULES),$(wildcard $(INCLUDE_DIR)/$(module)/*.h))

# Generate object files with module directory structure
OBJECTS = $(foreach src,$(SOURCES),$(OBJ_DIR)/$(patsubst $(SRC_DIR)/%.cpp,%.o,$(src)))

# Test files - recursively find test files in all module test directories
TEST_SOURCES = $(foreach module,$(MODULES),$(wildcard $(TEST_DIR)/$(module)/*.cpp))
TEST_HEADERS = $(foreach module,$(MODULES),$(wildcard $(TEST_DIR)/$(module)/*.h))
TEST_OBJECTS = $(foreach src,$(TEST_SOURCES),$(TEST_OBJ_DIR)/$(patsubst $(TEST_DIR)/%.cpp,%.o,$(src)))

# Source objects without main.cpp for linking with tests
LIB_SOURCES = $(filter-out $(SRC_DIR)/core/main.cpp, $(SOURCES))
LIB_OBJECTS = $(foreach src,$(LIB_SOURCES),$(OBJ_DIR)/$(patsubst $(SRC_DIR)/%.cpp,%.o,$(src)))

# Include paths for all modules
MODULE_INCLUDES = $(foreach module,$(MODULES),-I$(INCLUDE_DIR)/$(module))
ALL_INCLUDES = -I$(INCLUDE_DIR) $(MODULE_INCLUDES)

# Default target
all: directories $(TARGET)
	@echo "=== Build completed successfully ==="

# Create necessary directories
directories:
	@echo "=== Creating directories ==="
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(TEST_OBJ_DIR)
	@mkdir -p $(TEST_BIN_DIR)
	@mkdir -p $(TEST_DIR)
	@for module in $(MODULES); do mkdir -p $(OBJ_DIR)/$$module; done
	@for module in $(MODULES); do mkdir -p $(TEST_OBJ_DIR)/$$module; done
	@for module in $(MODULES); do mkdir -p $(TEST_DIR)/$$module; done
	@echo "Directories created/verified"

# Compile source files to object files (handles subdirectories)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	@echo "=== Compiling $< to $@ ==="
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
	$(CXX) $(CXXFLAGS) $(ALL_INCLUDES) -c $< -o $@
	@echo "Successfully compiled $<"

# Compile test files to object files (handles subdirectories)
$(TEST_OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp $(HEADERS) $(TEST_HEADERS)
	@echo "=== Compiling test $< to $@ ==="
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
	$(CXX) $(CXXFLAGS) $(ALL_INCLUDES) -I$(TEST_DIR) -c $< -o $@
	@echo "Successfully compiled test $<"

# Link object files to create the main executable
$(TARGET): $(OBJECTS)
	@echo "=== Linking objects to create executable ==="
	@echo "Objects to link: $(OBJECTS)"
	$(CXX) $(OBJECTS) -o $@
	@echo "Successfully created executable: $@"

# Build unit tests
test: directories $(TEST_TARGET)
	@echo "=== Unit test build completed ==="

# Link test objects with library objects to create test executable
$(TEST_TARGET): $(TEST_OBJECTS) $(LIB_OBJECTS)
	@echo "=== Linking test objects to create test executable ==="
	@echo "Test objects: $(TEST_OBJECTS)"
	@echo "Library objects: $(LIB_OBJECTS)"
	$(CXX) $(TEST_OBJECTS) $(LIB_OBJECTS) $(TEST_FLAGS) -o $@
	@echo "Successfully created test executable: $@"

# Run unit tests
test-run: $(TEST_TARGET)
	@echo "=== Running unit tests ==="
	$(TEST_TARGET)
	@echo "=== Unit tests completed ==="

# Run tests with verbose output
test-verbose: $(TEST_TARGET)
	@echo "=== Running unit tests (verbose) ==="
	$(TEST_TARGET) -v
	@echo "=== Verbose unit tests completed ==="

# Build individual modules
build-module-%: directories
	@echo "=== Building module: $* ==="
	@$(MAKE) $(foreach src,$(wildcard $(SRC_DIR)/$*/*.cpp),$(OBJ_DIR)/$(patsubst $(SRC_DIR)/%.cpp,%.o,$(src)))
	@echo "=== Module $* build completed ==="

# Test individual modules
test-module-%: directories
	@echo "=== Testing module: $* ==="
	@$(MAKE) $(foreach src,$(wildcard $(TEST_DIR)/$*/*.cpp),$(TEST_OBJ_DIR)/$(patsubst $(TEST_DIR)/%.cpp,%.o,$(src)))
	@echo "=== Module $* tests compiled ==="

# Show module information
info-module-%:
	@echo "=== Module Information: $* ==="
	@echo "Source files: $(wildcard $(SRC_DIR)/$*/*.cpp)"
	@echo "Header files: $(wildcard $(INCLUDE_DIR)/$*/*.h)"
	@echo "Test files: $(wildcard $(TEST_DIR)/$*/*.cpp)"
	@echo "Object files: $(foreach src,$(wildcard $(SRC_DIR)/$*/*.cpp),$(OBJ_DIR)/$(patsubst $(SRC_DIR)/%.cpp,%.o,$(src)))"

# Debug build
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: directories $(TARGET)
	@echo "=== Debug build completed ==="

# Debug build for tests
test-debug: CXXFLAGS += $(DEBUG_FLAGS)
test-debug: directories $(TEST_TARGET)
	@echo "=== Debug test build completed ==="

# Clean all generated files
clean:
	@echo "=== Cleaning build artifacts ==="
	$(RM) -rf $(OBJ_DIR)
	$(RM) -f $(BIN_DIR)/*.exe
	$(RM) -rf $(TEST_OBJ_DIR)
	$(RM) -f $(TEST_BIN_DIR)/*.exe
	@echo "Cleanup completed"

# Clean only test artifacts
clean-test:
	@echo "=== Cleaning test artifacts ==="
	@for %%m in ($(MODULES)) do @if exist "$(TEST_OBJ_DIR)\%%m\*.o" del /Q "$(TEST_OBJ_DIR)\%%m\*.o"
	@if exist "$(TEST_BIN_DIR)\*.exe" del /Q "$(TEST_BIN_DIR)\*.exe"
	@echo "Test cleanup completed"

# Clean and rebuild
rebuild: clean all
	@echo "=== Rebuild completed ==="

# Install (copy to system directory or specified location)
install: $(TARGET)
	@echo "=== Installing $(TARGET) ==="
	@echo "Installation completed"

# Show help
help:
	@echo "Available targets:"
	@echo "  all             - Build the main executable (default)"
	@echo "  debug           - Build with debug flags"
	@echo "  test            - Build unit tests"
	@echo "  test-run        - Build and run unit tests"
	@echo "  test-verbose    - Build and run unit tests with verbose output"
	@echo "  test-debug      - Build unit tests with debug flags"
	@echo "  build-module-X  - Build specific module (e.g., build-module-core)"
	@echo "  test-module-X   - Build tests for specific module"
	@echo "  info-module-X   - Show information about specific module"
	@echo "  clean           - Remove all build artifacts"
	@echo "  clean-test      - Remove only test artifacts"
	@echo "  rebuild         - Clean and build"
	@echo "  install         - Install the executable"
	@echo "  help            - Show this help message"
	@echo ""
	@echo "Available modules: $(MODULES)"
	@echo ""
	@echo "Variables:"
	@echo "  CXX       = $(CXX)"
	@echo "  CXXFLAGS  = $(CXXFLAGS)"
	@echo "  TEST_FLAGS= $(TEST_FLAGS)"
	@echo "  TARGET    = $(TARGET)"
	@echo "  TEST_TARGET= $(TEST_TARGET)"
	@echo "  MODULES   = $(MODULES)"

# Run the program
run: $(TARGET)
	@echo "=== Running $(TARGET) ==="
	$(TARGET)

# Check for source files
check:
	@echo "=== Project structure check ==="
	@echo "Modules: $(MODULES)"
	@echo "All source files found: $(SOURCES)"
	@echo "All header files found: $(HEADERS)"
	@echo "All object files will be: $(OBJECTS)"
	@echo "Test source files found: $(TEST_SOURCES)"
	@echo "Test header files found: $(TEST_HEADERS)"
	@echo "Test object files will be: $(TEST_OBJECTS)"
	@echo "Library objects (without main): $(LIB_OBJECTS)"
	@echo ""
	@echo "=== Module breakdown ==="
	@for %%m in ($(MODULES)) do @echo Module %%m: $(wildcard $(SRC_DIR)/%%m/*.cpp)

# Phony targets
.PHONY: all clean clean-test rebuild debug test test-run test-verbose test-debug install help run check directories build-module-% test-module-% info-module-%


hani:
	g++ hani_screen_shot\full_screen.cpp -lgdi32 -o bin\app.exe

fusc:
	g++ hani_screen_shot\full_screen.cpp -lgdi32 -o bin\app.exe && bin\app.exe

runhani:
	bin\app.exe