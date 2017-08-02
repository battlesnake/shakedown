### Master makefile

MAKEFLAGS += -rR --output-sync=target
SHELL = bash
.SHELLFLAGS = -euo pipefail -c

.DELETE_ON_ERROR:
.SECONDARY:

W_FLAGS =
C_FLAGS =
CFLAGS =
CXXFLAGS =
LDFLAGS =
ASFLAGS =

c_std = gnu11
cxx_std = c++14

defines =
include_dirs = $(shell find -type d -name 'include')
include =

ifeq ($(V),)
MAKEFLAGS += -s
endif

# Language of program (used to choose gcc/g++ for linking)
language ?= c

# Include project makefile if one exists
-include Makefile.project

# Include metadata makefile if one exists
-include Makefile.metadata

project_name ?= program

# Optimisiation level
O ?= g

# Build directory
buildbasedir ?= .build
builddir ?= $(buildbasedir)/$(O)

# Output directory
outdir ?= bin

# Temporary directory
tmpdir ?= tmp

# Create directories as needed, update symlinks
$(shell ( \
	mkdir -p -- $(builddir)/$(outdir); \
	mkdir -p -- $(builddir)/$(tmpdir); \
	rm -f $(outdir) $(tmpdir); \
	ln -sf $(builddir)/$(outdir) $(outdir); \
	ln -sf $(builddir)/$(tmpdir) $(tmpdir); \
) >&2 )

# Output filenames
program ?= $(outdir)/$(project_name)

# Inputs
collect_files = $(patsubst ./%.$(strip $1), $(tmpdir)/%.$(strip $2), $(shell find -name '*.$(strip $1)' -type f -not -path './$(tmpdir)/*' -not -path './$(outdir)/*'))

# Objects
objects += $(call collect_files, s, os)
objects += $(call collect_files, c, o)
objects += $(call collect_files, cpp, oxx)

-include Makefile.objects

# Toolchain configuration
CROSS_COMPILE ?=

CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
OBJCOPY = $(CROSS_COMPILE)objcopy
AS = $(CROSS_COMPILE)as
AR = $(CROSS_COMPILE)ar
GDB = $(CROSS_COMPILE)gdb

ifeq ($(language),c)
LD = $(CC)
endif

ifeq ($(language),c++)
LD = $(CXX)
endif

# Compiler/linker flags
W_FLAGS ?= -Wall -Wextra -Wno-unused-parameter -Werror

C_FLAGS ?= -ffunction-sections -fdata-sections
C_FLAGS += $(W_FLAGS) -O$(O) -g -MMD -MP -MF $@.d -c
C_FLAGS += $(addprefix -I,$(include_dirs))
C_FLAGS += $(addprefix -i,$(includes))
C_FLAGS += $(addprefix -D,$(defines))

CFLAGS += $(C_FLAGS) -std=$(c_std)

CXXFLAGS += $(C_FLAGS) -std=$(cxx_std)

LDFLAGS ?= -Wl,--gc-sections
LDFLAGS += $(W_FLAGS) -O$(O) -g

ASFLAGS ?=

# Link-time optimisation enabled if optimisation level is above "debug"
ifneq ($(filter-out 0 g, $(O)),)
use_lto ?= y
else
use_lto ?= n
endif

# Note: LTO breaks the symbol redefinition trick used to generate test binary
ifeq ($(use_lto),y)
C_FLAGS += -flto
LDFLAGS += -flto
endif

define log_action
	@[ "$V" ] || printf "  [%s]\t %s\n" "$(strip $(1))" "$(strip $(2))"
endef

.PHONY: all
all: $(program)

.PHONY: build
build: $(program)

.PHONY: clean
clean: cleanlinks
	$(call log_action, RM, $(builddir))
	rm -rf $(builddir)

.PHONY: cleanlinks
cleanlinks:
	$(call log_action, RM, $(outdir))
	rm -rf $(outdir)
	$(call log_action, RM, $(tmpdir))
	rm -rf $(tmpdir)

.PHONY: cleanall
cleanall: cleanlinks
	$(call log_action, RM, $(buildbasedir))
	rm -rf $(buildbasedir)

$(program): $(objects)
	@mkdir -p -- $(@D)
	$(call log_action, LD, $@)
	$(LD) $(LDFLAGS) -o $@ $^
	@size $@

$(filter %.o, $(objects)): $(tmpdir)/%.o: %.c
	@mkdir -p -- $(@D)
	$(call log_action, CC, $@)
	$(CC) $(CFLAGS) -o $@ $<

$(filter %.oxx, $(objects)): $(tmpdir)/%.oxx: %.cpp
	@mkdir -p -- $(@D)
	$(call log_action, CXX, $@)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(filter %.os, $(objects)): $(tmpdir)/%.os: %.s
	@mkdir -p -- $(@D)
	$(call log_action, AS, $@)
	$(AS) $(ASFLAGS) -o $@ $<

# Include makefile for test runner, if available
-include Makefile.test

-include $(shell find $(tmpdir) -name '*.d' -type f 2>/dev/null)
