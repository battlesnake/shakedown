all_sources := $(shell find -name '*.c' -type f)

sources := $(filter-out ./test/%, $(all_sources))
objects := $(sources:%.c=%.o)

test_sources := $(filter-out ./main.c, $(all_sources))
test_objects := $(test_sources:%.c=%.o)

outdir := bin

program := $(outdir)/program

test_runner := $(outdir)/test

$(shell mkdir -p -- $(outdir))

O ?= g

CC := gcc
LD := gcc
CFLAGS = -Wall -Wextra -Wno-unused-parameter -Werror -O$(O) -g -ffunction-sections -fdata-sections -std=gnu11 -I$(CURDIR) -MMD -MP -MF $@.d -c
LDFLAGS = -Wall -Wextra -Werror -O$(O) -g -Wl,--gc-sections

.PHONY: all
all: $(program) $(test_runner)

.PHONY: build
build: $(program)

.PHONY: test
test: $(test_runner)
	./$(test_runner) $(tests)

.PHONY: clean
clean:
	rm -rf $(outdir)
	find -name '*.o' -type f -delete
	find -name '*.d' -type f -delete

$(program): $(objects)
	$(LD) $(LDFLAGS) -o $@ $^

$(test_runner): $(test_objects)
	$(LD) $(LDFLAGS) -o $@ $^

$(objects): %.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

$(test_objects): test/%.o: test/%.c
	$(CC) $(CFLAGS) -DTEST_MAIN=main -o $@ $<

-include $(shell find -name '*.d' -type f)
