SHELL = bash

GCOV ?= gcov
LCOV ?= lcov
GENHTML ?= genhtml
DOXYGEN ?= doxygen

BUILD_DIR = ./build
SOURCE_DIR = ./src
INC_DIR = ./include
TEST_DIR = ./test


BUILD_DIR = ./build
LIB_DIR = $(BUILD_DIR)/lib
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin
COV_DIR = $(BUILD_DIR)/coverage
INC_DIR = ./include
SRC_DIR = ./src
TEST_DIR = ./test

MP_LIB = $(LIB_DIR)/mp.a
MP_LIB_SRC = $(shell find '$(SRC_DIR)/mp' -type f -name '*.c')
MP_LIB_OBJ = \
	$(patsubst $(SRC_DIR)/mp/%.c,$(OBJ_DIR)/mp/%.o,$(MP_LIB_SRC))

MP_TEST_SRC = $(shell find '$(TEST_DIR)/mp' -type f -name '*.c')
MP_TEST_OBJ = \
	$(patsubst $(TEST_DIR)/%.c,$(OBJ_DIR)/%.o,$(MP_TEST_SRC))
MP_TEST_BIN = \
	$(patsubst $(TEST_DIR)/%.c,$(BIN_DIR)/%,$(MP_TEST_SRC))

MP_CPPFLAGS = -MP -MD -I$(INC_DIR) -I$(BUILD_DIR)/include
MP_CFLAGS = \
	-Wall \
	-Wextra \
	-Wcast-align=strict \
	-Wstrict-overflow=5 \
	-Wwrite-strings \
	-Wcast-qual \
	-Wunreachable-code \
	-Wpointer-arith \
	-Warray-bounds \
	-Wno-sign-compare \
	-Wno-switch \
	-Wno-implicit-fallthrough
MP_LDFLAGS =
MP_LDLIBS =

ifneq ($(MP_DEBUG),)
MP_CPPFLAGS += -DJSON_DEBUG=1
MP_CFLAGS += -g3 -fsanitize=undefined -fsanitize=address
MP_LDFLAGS += -g3 -fsanitize=undefined -fsanitize=address
endif

ifneq ($(MP_OPTIMIZE),)
MP_CFLAGS += -O3
MP_LDFLAGS += -03 -flto
else
MP_CFLAGS += -O0
MP_LDFLAGS += -O0
endif

CPPFLAGS += $(MP_CPPFLAGS)
CFLAGS += $(MP_CFLAGS)
LDFLAGS += $(MP_LDFLAGS)
LDLIBS += $(MP_LDLIBS)

.PHONY: all lib test clean docs

all: lib $(MP_TEST_BIN) docs

lib: $(MP_LIB)

test: $(MP_TEST_BIN)

docs:
	@ $(DOXYGEN)

clean:
	@ $(RM) -r ./build

%.o:
	@ mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $(filter-out %.h,$^)

%.a:
	@ mkdir -p $(@D)
	$(AR) rcs $@ $^

$(MP_LIB): $(MP_LIB_OBJ)
$(MP_LIB_OBJ) : build/obj/%.o : $(SRC_DIR)/%.c
$(MP_TEST_OBJ) : build/obj/%.o : $(TEST_DIR)/%.c
# $(MP_TEST_BIN):
# 	@mkdir -p $(@D)
# 	$(CXX) $(LDFLAGS) $(LDLIBS) -o $@ $^

-include $(shell find build -name \*.d 2>/dev/null)
