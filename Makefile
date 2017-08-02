MAKEFLAGS += -rR
W_FLAGS =
C_FLAGS =
CFLAGS =
CXXFLAGS =
LDFLAGS =

ifeq ($(V),)
MAKEFLAGS += -s
endif

include Makefile.project

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
test_runner ?= $(outdir)/test

# Inputs
sources ?= $(patsubst ./%, %, $(shell find -name '*.c' -type f -not -path './$(tmpdir)/*' -not -path './$(outdir)/*'))

# Objects
objects ?= $(sources:%.c=$(tmpdir)/%.o)

# Toolchain configuration
language ?= c

CROSS_COMPILE ?=

CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++

ifeq ($(language),c)
LD = $(CROSS_COMPILE)gcc
endif

ifeq ($(language),c++)
LD = $(CROSS_COMPILE)g++
endif

OBJCOPY = $(CROSS_COMPILE)objcopy

# Compiler/linker flags
W_FLAGS ?= -Wall -Wextra -Wno-unused-parameter -Werror

C_FLAGS ?= -ffunction-sections -fdata-sections
C_FLAGS += $(W_FLAGS) -O$(O) -g -MMD -MP -MF $@.d -c

CFLAGS += $(C_FLAGS) -std=gnu11

CXXFLAGS += $(C_FLAGS) -std=gnu++14

LDFLAGS ?= -Wl,--gc-sections
LDFLAGS ?= $(W_FLAGS) -O$(O) -g

# Link-time optimisation if optimisation level is above "debug"
# Note: LTO breaks the symbol redefinition trick used to generate test binary
ifneq ($(filter-out 0 g, $(O)),)
ifneq ($(use_lto),n)
CFLAGS += -flto
LDFLAGS += -flto
use_lto := y
endif
else
use_lto := n
endif

.PHONY: all
all: $(program)

# Do not build test runner if LTO is enabled
ifeq ($(use_lto),n)
all: $(test_runner)
endif

define log_action
	@[ "$V" ] || printf "  [%s]\t %s\n" "$(strip $(1))" "$(strip $(2))"
endef

.PHONY: build
build: $(program)

.PHONY: test
test: $(test_runner)
	./$< $(tests)

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

ifeq ($(use_lto),n)
$(test_runner): $(tmpdir)/test_main.o $(filter-out $(tmpdir)/test/test.o $(tmpdir)/main.o, $(objects))
	@mkdir -p -- $(@D)
	$(call log_action, LD, $@)
	$(LD) $(LDFLAGS) -o $@ $^

$(tmpdir)/test_main.o: $(tmpdir)/test/test.o
	@mkdir -p -- $(@D)
	$(call log_action, OC, $@)
	$(OBJCOPY) --redefine-sym test_main=main $< $@
endif

$(filter %.o, $(objects)): $(tmpdir)/%.o: %.c
	@mkdir -p -- $(@D)
	$(call log_action, CC, $@)
	$(CC) $(CFLAGS) -o $@ $<

$(filter %.oxx, $(objects)): $(tmpdir)/%.oxx: %.cpp
	@mkdir -p -- $(@D)
	$(call log_action, CXX, $@)
	$(CXX) $(CXXFLAGS) -o $@ $<

-include $(shell find $(tmpdir) -name '*.d' -type f 2>/dev/null)
