DIRS     := . ../utils
INCLUDE  := $(addsuffix /include,$(DIRS))
SRC      := src
PROJECTS := . ast
WARNINGS := all extra pedantic format=2 format-overflow=2 init-self \
	ignored-qualifiers switch-enum strict-aliasing=3 \
	alloca array-bounds shadow pointer-arith uninitialized \
	aggregate-return strict-prototypes redundant-decls \
	parentheses unreachable-code missing-field-initializers \
	unused
CFLAGS   := -std=c99 -ggdb3 -fPIC $(addprefix -I,$(INCLUDE)) $(addprefix -W,$(WARNINGS))
LIBS     := :output/utils.a
LDFLAGS  := $(addprefix -l,$(LIBS)) $(addprefix -L,$(DIRS))
LTO      ?= 0
DEBUG    ?= 1
CC       := $$HOME/gcc-installs/usr/local/bin/gcc
BIN      := main

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

SOURCES = $(foreach PROJ,$(PROJECTS),$(wildcard $(SRC)/$(PROJ)/*.c))
OBJECTS = $(patsubst $(SRC)/%.c,$(OUTPUT)/%.o,$(SOURCES))
DEPS    = $(patsubst $(SRC)/%.c,$(OUTPUT)/%.d,$(SOURCES))
OUTDIRS = output $(OUTPUT) $(foreach PROJ,$(PROJECTS),$(OUTPUT)/$(PROJ))

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

