
# ****************************************************************************
# *                                                                          *
# *  Intel Corporation                                                       *
# *  Copyright (c) 2024 Intel Corporation.                                   *
# *                                                                          *
# ****************************************************************************

# Project name
TARGET_PROJECT_NAME := "Xtensa LX7 MCTP/USB"
COUNT_INSTRUCTIONS ?= 0  # Default is to not run the Python script

# SpecialFX
COLOR_RESET=\033[0m
COLOR_GREEN=\033[32m
COLOR_YELLOW=\033[33m
COLOR_CYAN=\033[36m

# Check if either XTENSA_TOOLS or XTENSA_HOME is not defined
ifndef XTENSA_TOOLS
ifndef XTENSA_HOME
$(error XTENSA_TOOLS or XTENSA_HOME is not defined. The Xtensa SDK is required for the build.)
endif
endif

# Common include paths
INCLUDE_PATHS = -Ilibmctp -Isrc/include

# Compiler and flags
CC = xt-clang
AS = xtensa-elf-as
LD = xt-clang

# For all targets
COMMON_CFLAGS = -c -save-temps=obj -DHAVE_CONFIG_H $(INCLUDE_PATHS)
COMMON_LDFLAGS = -Wl,--gc-sections -lxos -lsim

# Target specific flags
DEBUG_CFLAGS = -ggdb3 -Wall -Werror -mlongcalls -ffunction-sections -O0 -DDEBUG
RELEASE_CFLAGS = -O3 -DNDEBUG

# Default build type
BUILD_TYPE ?= release
BUILD_DIR = build/$(BUILD_TYPE)
START_MESSAGE = "$(TARGET_PROJECT_NAME): \'$(BUILD_TYPE)\'."

# Simplified conditional checks
ifeq ($(BUILD_TYPE),debug)
    CFLAGS = $(COMMON_CFLAGS) $(DEBUG_CFLAGS)
else ifeq ($(BUILD_TYPE),release)
    CFLAGS = $(COMMON_CFLAGS) $(RELEASE_CFLAGS)
else
    $(error Invalid BUILD_TYPE specified: $(BUILD_TYPE))
endif

# Compiled sources
SRCS =	src/main.c \
		src/mctp_usb.c \
		src/hal/hal.c \
		src/hal/hal_alloc.c \
		src/hal/hal_msgq.c \
		src/test/cargs.c \
		src/test/cycles_eval.c \
		libmctp/core.c \
		libmctp/alloc.c \
		libmctp/log.c \
		libmctp/crc-16-ccitt.c
	
# Object files
OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS)) \
       $(patsubst %.S,$(BUILD_DIR)/%.o,$(filter %.S,$(SRCS)))

# Compiled output
TARGET = firmware.elf

# Default target
.PHONY: all
all: prebuild $(BUILD_DIR)/$(TARGET)

# Clean up before building
.PHONY: prebuild
prebuild:
	@echo -e "$(COLOR_GREEN)\n$(START_MESSAGE)$(COLOR_RESET)"

# Rule for building object files
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo -e "$(COLOR_YELLOW)Compiling:$(COLOR_RESET) $(COLOR_CYAN)$<$(COLOR_RESET)"
	@$(CC) $(CFLAGS) -o $@ $<

# Rule for building object files from assembly source files
$(BUILD_DIR)/%.o: %.S
	@mkdir -p $(dir $@)
	@echo -e "$(COLOR_YELLOW)Assembling:$(COLOR_RESET) $(COLOR_CYAN)$<$(COLOR_RESET)"
	@$(AS) $(ASFLAGS) -o $@ $<

# Linking rule
$(BUILD_DIR)/$(TARGET): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	@echo -e "$(COLOR_YELLOW)Linking:$(COLOR_RESET) $(COLOR_CYAN)$@$(COLOR_RESET)\n"
	@$(LD) $(COMMON_LDFLAGS) -o $@ $^
	@if [ $(COUNT_INSTRUCTIONS) -eq 1 ]; then \
		python3 resources/elf_inspect.py $@ $(BUILD_DIR); \
	fi

# Build type-specific targets
.PHONY: debug release
debug:
	@$(MAKE) -s -j$(nproc) BUILD_TYPE=debug
release:
	@$(MAKE) -s -j$(nproc) BUILD_TYPE=release

# Run target to execute the firmware in the emulator
.PHONY: run
run: release
	@echo -e "$(COLOR_YELLOW)Running Xtensa emulator for: $(COLOR_CYAN)$(BUILD_TYPE) $@$(COLOR_RESET)"
	@xt-run $(BUILD_DIR)/$(TARGET) $(ARGS)

# Clean up
.PHONY: clean
clean:
	@rm -rf build
	@echo -e "$(COLOR_CYAN)Cleaned up build files$(COLOR_RESET)"
