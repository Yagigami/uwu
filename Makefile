INCLUDE  := include
SRC      := src
WARNINGS := all extra format=2 format-overflow=2 init-self \
	ignored-qualifiers switch-enum strict-aliasing=3 \
	alloca array-bounds shadow pointer-arith uninitialized \
	aggregate-return strict-prototypes redundant-decls \
	parentheses unreachable-code missing-field-initializers \
	unused
CFLAGS   := -std=c99 -ggdb3 -fPIC $(addprefix -I,$(INCLUDE)) $(addprefix -W,$(WARNINGS))
LIBS     := m
LDFLAGS  := $(addprefix -l,$(LIBS)) $(addprefix -L,$(DIRS))
LTO      ?= 0
DEBUG    ?= 1
SAN      ?= address leak undefined
EMPTY    :=
SPACE    := $(EMPTY) $(EMPTY)
COMMA    := ,
CC       := $$HOME/gcc-installs/usr/local/bin/gcc
BIN      := main

ifeq ($(DEBUG), 1)
	OUTPUT := output/debug
	CFLAGS += -fanalyzer
else
	OUTPUT := output/release
	CFLAGS += -O3 -D NDEBUG
endif
ifneq ($(words $(SAN)),0)
	CFLAGS += -fsanitize=$(subst $(SPACE),$(COMMA),$(SAN))
endif
ifeq ($(LTO), 1)
	CFLAGS += -flto -fwhole-program
endif

SOURCES = $(shell find $(SRC) -type f -name "*.c")
OBJECTS = $(patsubst $(SRC)/%.c,$(OUTPUT)/%.o,$(SOURCES))
DEPS    = $(patsubst $(SRC)/%.c,$(OUTPUT)/%.d,$(SOURCES))
OUTDIRS = output $(OUTPUT) $(patsubst $(SRC)/%,$(OUTPUT)/%,$(shell find $(SRC) -type d))

$(OUTPUT)/$(BIN): $(OUTDIRS) $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) $(LDFLAGS)

-include $(DEPS)
$(OBJECTS): $(OUTPUT)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -MMD -c -o $@ $<

$(OUTDIRS):
	-mkdir $@

.PHONY: clean
clean:
	-rm -rf $(OUTPUT)

