# EchoTrace build system.
#
# Targets:
#   make            build the CLI (release)
#   make lib        build the static analysis library
#   make tests      build and run the unit-test suite
#   make bench      build the benchmark harness
#   make clean      remove all generated artifacts
#   make tidy       remove only incremental objects (keep release bins)
#
# All build output lives under ./build and is never committed.

PROJECT   := echotrace
CXX       ?= g++
CXXSTD    := -std=c++20
WARN      := -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
WARN      += -Wnon-virtual-dtor -Wold-style-cast -Wcast-align
OPT       ?= -O2
INC       := -Iinclude
CPPFLAGS  := $(INC) -MMD -MP

BUILD     := build
BINDIR    := $(BUILD)/bin
OBJDIR    := $(BUILD)/obj
TESTDIR   := $(BUILD)/tests

# Parallel-safe: every .cpp is compiled independently.
LIB_SRCS  := $(wildcard src/*.cpp)
LIB_OBJS  := $(patsubst src/%.cpp,$(OBJDIR)/%.o,$(LIB_SRCS))
LIB       := $(BUILD)/lib$(PROJECT).a

CLI_SRCS  := src/cli/main.cpp
CLI_OBJ   := $(patsubst src/%.cpp,$(OBJDIR)/%.o,$(CLI_SRCS))
CLI_BIN   := $(BINDIR)/$(PROJECT)

TEST_SRCS := $(wildcard tests/*.cpp)
TEST_OBJS := $(patsubst tests/%.cpp,$(TESTDIR)/%.o,$(TEST_SRCS))
TEST_BIN  := $(BINDIR)/$(PROJECT)_tests

BENCH_BIN := $(BINDIR)/$(PROJECT)_bench

.PHONY: all lib cli tests test bench clean tidy run-help
.DEFAULT_GOAL := cli

all: cli tests

# --- library --------------------------------------------------------------
lib: $(LIB)

$(LIB): $(LIB_OBJS) | $(BUILD)
	@echo "[AR]      $@"
	@$(AR) rcs $@ $^

# --- CLI executable -------------------------------------------------------
cli: $(CLI_BIN)

$(CLI_BIN): $(CLI_OBJ) $(LIB) | $(BINDIR)
	@echo "[LINK]    $@"
	@$(CXX) $(CXXSTD) $(OPT) $< -L$(BUILD) -l$(PROJECT) -lpthread -o $@

# --- unit tests -----------------------------------------------------------
tests test: $(TEST_BIN)
	@echo "[RUN]     unit tests"
	@./$(TEST_BIN)

$(TEST_BIN): $(TEST_OBJS) $(LIB) | $(BINDIR)
	@echo "[LINK]    $@"
	@$(CXX) $(CXXSTD) $(OPT) $^ -L$(BUILD) -l$(PROJECT) -lpthread -o $@

# --- benchmark ------------------------------------------------------------
bench: $(BENCH_BIN)

$(BENCH_BIN): tests/benchmark.o $(LIB) | $(BINDIR)
	@echo "[LINK]    $@"
	@$(CXX) $(CXXSTD) $(OPT) $^ -L$(BUILD) -l$(PROJECT) -lpthread -o $@
	@./$(BENCH_BIN)

# --- generic compile rule -------------------------------------------------
$(OBJDIR)/%.o: src/%.cpp | $(OBJDIR)
	@echo "[CXX]     $<"
	@$(CXX) $(CXXSTD) $(WARN) $(OPT) $(CPPFLAGS) -c $< -o $@

$(OBJDIR)/cli/%.o: src/cli/%.cpp | $(OBJDIR)/cli
	@echo "[CXX]     $<"
	@$(CXX) $(CXXSTD) $(WARN) $(OPT) $(CPPFLAGS) -c $< -o $@

$(TESTDIR)/%.o: tests/%.cpp | $(TESTDIR)
	@echo "[CXX]     $<"
	@$(CXX) $(CXXSTD) $(WARN) $(OPT) $(CPPFLAGS) -c $< -o $@

# --- directories ----------------------------------------------------------
$(BUILD) $(BINDIR) $(OBJDIR) $(OBJDIR)/cli $(TESTDIR):
	@mkdir -p $@

# --- housekeeping ---------------------------------------------------------
clean:
	@rm -rf $(BUILD)

tidy:
	@rm -rf $(OBJDIR) $(TESTDIR)

run-help: cli
	@./$(CLI_BIN) --help

-include $(shell find $(BUILD) -name '*.d' 2>/dev/null)
