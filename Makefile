# ****************************************************************************
# *                                                                          *
# *  Intel Corporation                                                       *
# *  Copyright (c) 2024 Intel Corporation.                                   *
# *                                                                          *
# ****************************************************************************

# Project name
TARGET_PROJECT_NAME := "Xtensa LX7 MCTP/USB"
COUNT_INSTRUCTIONS ?= 0  # Default is to not run the Python script
PUBLISH_CGI := 1
MAKEFLAGS += --no-print-directory # Suppress "Entering directory" messages

# SpecialFX
COLOR_RESET=\033[0m
COLOR_GREEN=\033[32m
COLOR_YELLOW=\033[33m
COLOR_CYAN=\033[36m

# Check if either XTENSA_TOOLS or XTENSA_HOME is not defined
ifndef XTENSA_TOOLS
ifndef XTENSA_HOME
ifndef XTENSA_CORE
$(error XTENSA_TOOLS, XTENSA_HOME or XTENSA_HOME are not defined. The Xtensa SDK is required for the build.)
endif
endif
endif

# Detect if the Makefile was invoked with 'make' instead of 'xt-make'
ifneq ($(MAKE),xt-make)
.PHONY: FORCE
FORCE:
	@$(MAKE) -f $(MAKEFILE_LIST) MAKE=xt-make $(MAKECMDGOALS)
	@exit 0
endif

# Common include paths
INCLUDE_PATHS = -Ilibmctp -Isrc/include -Isrc/include

# Compiler and flags
CC = xt-clang
AS = xt-as
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

 CFLAGS = $(COMMON_CFLAGS)

# Simplified conditional checks
ifeq ($(BUILD_TYPE),debug)
    CFLAGS += $(DEBUG_CFLAGS)
else ifeq ($(BUILD_TYPE),release)
    CFLAGS += $(RELEASE_CFLAGS)
else
    $(error Invalid BUILD_TYPE specified: $(BUILD_TYPE))
endif

# Compiled sources
SRCS =	src/main.c \
		src/hal/hal.c \
		src/hal/hal_alloc.c \
		src/hal/hal_msgq.c \
		src/hal/ncsi.c \
		src/hal/cargs.c \
		libmctp/core.c \
		libmctp/alloc.c \
		libmctp/log.c \
		libmctp/crc-16-ccitt.c \
		src/tests/test_launcher.c \
		src/tests/test_frag.c \
		src/tests/test_frag_memcpy.c \
		src/tests/test_defrag_mctplib.c \
		src/tests/test_msgq.c \
		src/tests/test_memcpy.c \
		src/tests/test_usless.c
	
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
	@echo -e "$(COLOR_YELLOW)Linking:$(COLOR_RESET) $(COLOR_CYAN)$@$(COLOR_RESET)"
	@$(LD) $(COMMON_LDFLAGS) -o $@ $^

# Post-build rule
.PHONY: post_build
post_build: $(BUILD_DIR)/$(TARGET)
ifeq ($(PUBLISH_CGI),1)
	@echo -e "$(COLOR_YELLOW)Copying to CGI path...$(COLOR_RESET)"
	@sudo cp $(BUILD_DIR)/$(TARGET) /var/www/cgi-bin/$(BUILD_TYPE)/firmware.elf >/dev/null 2>&1 || true
endif

	@if [ $(COUNT_INSTRUCTIONS) -eq 1 ]; then \
		echo -e "$(COLOR_YELLOW)Running instruction count script...$(COLOR_RESET)"; \
		python3 resources/elf_inspect.py $(BUILD_DIR)/$(TARGET) $(BUILD_DIR); \
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
	@xt-run --version
	@xt-run $(BUILD_DIR)/$(TARGET) -v
	@echo -e ""

# Default target includes post-build
.PHONY: all
all: $(BUILD_DIR)/$(TARGET) post_build

# Clean up
.PHONY: clean
clean:
	@rm -rf build
	@echo -e "$(COLOR_CYAN)Cleaned up build files$(COLOR_RESET)\n"
