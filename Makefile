DIRS := . ../utils
INCLUDE := $(addsuffix /include,$(DIRS))
SRC := src
WARNINGS := all extra pedantic format=2 format-overflow=2 init-self \
	ignored-qualifiers switch-enum strict-aliasing=3 \
	alloca array-bounds shadow pointer-arith uninitialized \
	aggregate-return strict-prototypes redundant-decls \
	parentheses unreachable-code missing-field-initializers \
	unused
CFLAGS := -std=c99 -ggdb3 -fPIC $(addprefix -I,$(INCLUDE)) $(addprefix -W,$(WARNINGS))
LIBS := :output/utils.a
LDFLAGS := $(addprefix -l,$(LIBS)) $(addprefix -L,$(DIRS))
LTO ?= 0
DEBUG ?= 1
CC := $$HOME/gcc-installs/usr/local/bin/gcc
BINS := tests
BIN ?= tests

ifeq ($(DEBUG), 1)
	OUTPUT := output/debug
	CFLAGS += -fsanitize=address,leak,undefined -fanalyzer
else
	OUTPUT := output/release
	CFLAGS += -O3 -D NDEBUG
endif
ifeq ($(LTO), 1)
	CFLAGS += -flto -fwhole-program
endif

MAINS   = $(patsubst %,$(SRC)/%.c,$(filter-out $(BIN),$(BINS)))
SOURCES = $(filter-out $(MAINS),$(wildcard $(SRC)/*.c))
OBJECTS = $(patsubst $(SRC)/%.c,$(OUTPUT)/%.o,$(SOURCES))
DEPS = $(patsubst $(SRC)/%.c,$(OUTPUT)/%.d,$(SOURCES))

$(OUTPUT)/$(BIN): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

run: $(OUTPUT)/$(BIN)
	./$<

debug: $(OUTPUT)/$(BIN)
	gdb $<

-include $(DEPS)

$(OBJECTS): $(OUTPUT)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -MMD -c -o $@ $<

.PHONY: clean

clean:
	-rm $(OUTPUT)/*

