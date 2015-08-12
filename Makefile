# Makefile for the MonoBridge Project

# Top-level declares
CC := gcc -xc -c
CXX := g++ -xc++ -c
LD := g++
CS := mcs

DEBUG_FLAG := -g
CS_DEBUG_FLAG := -debug
MONO_LIB := mono-2

MONO_CFLAGS := `pkg-config --cflags $(MONO_LIB)`
MONO_LDFLAGS := `pkg-config --libs $(MONO_LIB)`

CFLAGS := $(DEBUG_FLAG) $(MONO_CFLAGS)
CXXFLAGS := $(DEBUG_FLAG) $(MONO_CFLAGS)
LDFLAGS := $(MONO_LDFLAGS)


# Files
# This section specifies all the input/output files

OUT_FILE := MonoBridge-RunTest.app

SRCS :=
OBJS :=

SRCS += MonoBridge.cc \
		MonoBridge-TestApp.cc

OBJS := $(SRCS:%.cc=%.o)


CS_SRCS :=
CS_SRCS += lib/libtest.cs

LIB_FILES := $(CS_SRCS:%.cs=%.dll)
LIB_FILES_DBG := $(addsuffix .mdb, $(LIB_FILES))

# Targets

.PHONEY: all
all: $(OUT_FILE) $(LIB_FILES)

$(OUT_FILE): $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o $(OUT_FILE)

.PHONEY: clean
clean:
	rm -fr $(OBJS) $(OUT_FILE) $(LIB_FILES) $(LIB_FILES_DBG)

# Rules
%.dll : %.cs
	$(CS) -target:library $(CS_DEBUG_FLAG) -out:$@ $<

# end of file
