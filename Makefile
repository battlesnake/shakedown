MAKEFLAGS += -rR

# Output directory
outdir := bin

# Output filenames
program := $(outdir)/program
test_runner := $(outdir)/test

# Optimisiation level
O ?= g

# Temporary directory
tmpdir := bin/tmp-$(O)

# Inputs
sources := $(patsubst ./%, %, $(shell find -name '*.c' -type f -not -path './$(tmpdir)/*'))

# Objects
objects := $(sources:%.c=$(tmpdir)/%.o)

# Toolchain configuration
CROSS_COMPILE ?=
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy

# Compiler/linker flags
CFLAGS = -Wall -Wextra -Wno-unused-parameter -Werror -O$(O) -g -ffunction-sections -fdata-sections -std=gnu11 -I$(CURDIR) -MMD -MP -MF $@.d -c
LDFLAGS = -Wall -Wextra -Werror -O$(O) -g -Wl,--gc-sections

# Link-time optimisation if optimisation level is above "debug"
ifneq ($(filter-out 0 g, $(O)),)
# Breaks symbol redefinition used ot generate test binary
CFLAGS += -flto
LDFLAGS += -flto
use_lto := y
else
use_lto := n
endif

# Do not build test runner if LTO is enabled
.PHONY: all
all: $(program)
ifeq ($(use_lto),n)
all: $(test_runner)
endif

.PHONY: build
build: $(program)

.PHONY: test
test: $(test_runner)
	./$(test_runner) $(tests)

.PHONY: clean
clean:
	rm -rf $(outdir)

$(program): $(objects)
	@mkdir -p -- $(@D)
	$(LD) $(LDFLAGS) -o $@ $^

ifeq ($(use_lto),n)
$(test_runner): $(tmpdir)/test_main.o $(filter-out $(tmpdir)/test/test.o $(tmpdir)/main.o, $(objects))
	$(LD) $(LDFLAGS) -o $@ $^

$(tmpdir)/test_main.o: $(tmpdir)/test/test.o
	$(OBJCOPY) --redefine-sym test_main=main $< $@
endif

$(objects): $(tmpdir)/%.o: %.c
	@mkdir -p -- $(@D)
	$(CC) $(CFLAGS) -o $@ $<

-include $(shell find $(tmpdir) -name '*.d' -type f 2>/dev/null)
