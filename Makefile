### Master makefile

MAKEFLAGS += -rR --output-sync=target
SHELL = bash
.SHELLFLAGS = -euo pipefail -c

.DELETE_ON_ERROR:
.SECONDARY:

w_flags = -Wall -Wextra -Wno-unused-parameter -Werror
c_flags = -ffunction-sections -fdata-sections
ldflags = -Wl,--gc-sections
asflags =
cflags =
cxxflags =


c_std = gnu11
cxx_std = c++14

defines =
include_dirs = $(shell find -type d -name 'include')
includes =
libs =

source_exclude =

ifeq ($(V),)
MAKEFLAGS += -s
endif

# Language of program (used to choose gcc/g++ for linking)
language ?= c

# Useful logging functions
define log_action
	@printf "  $$(tput bold)[%s]$$(tput sgr0)\t %s\n" "$(strip $(1))" "$(strip $(2))"
endef

define log_info
$(shell [ "$(V)" ] && printf " $$(tput bold)-$$(tput sgr0) $$(tput sitm)%s$$(tput sgr0)\n" "$(strip $(1))" >&2)
endef

define log_var
$(call log_info, $(1) = $($(strip $1)))
endef

# Include project makefile if one exists
-include Makefile.project

# Include metadata makefile if one exists
-include Makefile.metadata

project_name ?= program

# Optimisiation level
o := $(firstword $(o) $(O) g)

$(call log_var, o)

# Build directory
build_base ?= .build
build_profile ?= $(o)
build_dir ?= $(build_base)/$(build_profile)

$(call log_var, build_profile)
$(call log_var, build_dir)

# Output directory
outdir ?= bin

# Temporary directory
tmpdir ?= tmp

# Create directories as needed, update symlinks
$(shell ( \
	mkdir -p -- $(build_dir)/$(outdir); \
	mkdir -p -- $(build_dir)/$(tmpdir); \
	rm -f $(outdir) $(tmpdir); \
	ln -sf $(build_dir)/$(outdir) $(outdir); \
	ln -sf $(build_dir)/$(tmpdir) $(tmpdir); \
) >&2 )

# Output filenames
program ?= $(outdir)/$(project_name)

# Inputs
source_root ?= .
source_exclude += $(tmpdir) $(outdir)
collect_files = $(patsubst $(source_root)/%.$(strip $1), $(tmpdir)/%.$(strip $2), $(shell find $(source_root) -name '*.$(strip $1)' -type f $(source_exclude:%=-not -path './%/*' )))

$(call log_var, program)
$(call log_var, source_root)
$(call log_var, source_exclude)

# Objects
objects += $(call collect_files, s, os)
objects += $(call collect_files, c, o)
objects += $(call collect_files, cpp, oxx)

$(call log_var, objects)

-include Makefile.objects

# Toolchain configuration
CROSS_COMPILE ?= $(cross_compile)

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
c_flags += $(w_flags) -O$(o) -g -MMD -MP -MF $@.d -c
c_flags += $(addprefix -I,$(include_dirs))
c_flags += $(addprefix -i,$(includes))
c_flags += $(addprefix -D,$(defines))

cflags += $(c_flags) -std=$(c_std)

cxxflags += $(c_flags) -std=$(cxx_std)

ldflags += $(w_flags) -O$(o) -g
ldflags += $(addprefix -l,$(libs))

# Link-time optimisation enabled if optimisation level is above "debug"
ifneq ($(filter-out 0 g, $(O)),)
use_lto := $(firstword $(use_lto) y)
else
use_lto := $(firstword $(use_lto) n)
endif

# Note: LTO breaks the symbol redefinition trick used to generate test binary
ifeq ($(use_lto),y)
c_flags += -flto
ldflags += -flto
endif

.PHONY: all
all: $(program)

.PHONY: build
build: $(program)

.PHONY: clean
clean:: cleanlinks
	$(call log_action, RM, $(build_dir))
	rm -rf $(build_dir)

.PHONY: cleanlinks
cleanlinks::
	$(call log_action, RM, $(outdir))
	rm -rf $(outdir)
	$(call log_action, RM, $(tmpdir))
	rm -rf $(tmpdir)

.PHONY: cleanall
cleanall:: cleanlinks
	$(call log_action, RM, $(build_base))
	rm -rf $(build_base)

$(program): $(objects)
	@mkdir -p -- $(@D)
	$(call log_action, LD, $@)
	$(LD) $^ -o $@ $(ldflags)
	@size $@

$(filter %.o, $(objects)): $(tmpdir)/%.o: %.c
	@mkdir -p -- $(@D)
	$(call log_action, CC, $@)
	$(CC) $(cflags) -o $@ $<

$(filter %.oxx, $(objects)): $(tmpdir)/%.oxx: %.cpp
	@mkdir -p -- $(@D)
	$(call log_action, CXX, $@)
	$(CXX) $(cxxflags) -o $@ $<

$(filter %.os, $(objects)): $(tmpdir)/%.os: %.s
	@mkdir -p -- $(@D)
	$(call log_action, AS, $@)
	$(AS) $(asflags) -o $@ $<

# Include makefile for test runner, if available
-include Makefile.test

-include $(shell find $(tmpdir) -name '*.d' -type f 2>/dev/null)
