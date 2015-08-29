# Makefile for the MonoBridge Project

# Top-level declares
CC := gcc -xc -c
CXX := g++ -xc++ --std=c++11 -c
LD := g++
CS := mcs

DEBUG_FLAG := -g -Wall
CS_DEBUG_FLAG := -debug
MONO_LIB := mono-2

MONO_CFLAGS := `pkg-config --cflags $(MONO_LIB)`
MONO_LDFLAGS := `pkg-config --libs $(MONO_LIB)`

BOOST_INSTALL_PATH := /opt/local/

BOOST_FLAGS := -I $(BOOST_INSTALL_PATH)include/ -L $(BOOST_INSTALL_PATH)lib/
BOOST_LIBS := -lboost_system-mt -lboost_filesystem-mt -lboost_regex-mt

CFLAGS := $(DEBUG_FLAG) $(MONO_CFLAGS)
CXXFLAGS := $(DEBUG_FLAG) $(MONO_CFLAGS) $(BOOST_FLAGS)
LDFLAGS := $(MONO_LDFLAGS) $(BOOST_LIBS)


# Files
# This section specifies all the input/output files

OUT_FILE := MonoBridge-RunTest.app

SRCS :=
OBJS :=

SRCS += MonoBridge.cc \
		MonoBridge-TestApp.cc

OBJS := $(SRCS:%.cc=%.o)


CS_SRCS :=
CS_SRCS += lib/FileIOLib.cs \
			lib/Logger.cs

LIB_FILES := $(CS_SRCS:%.cs=%.dll)
LIB_FILES_DBG := $(addsuffix .mdb, $(LIB_FILES))

# Targets

.PHONY: all
all: $(OUT_FILE) $(LIB_FILES)

.PHONY: bin
bin: $(OUT_FILE)

.PHONY: libs
libs: $(LIB_FILES)

$(OUT_FILE): $(OBJS)
	$(LD) -o $(OUT_FILE) $(OBJS) $(LDFLAGS)

.PHONY: clean
clean:
	rm -fr $(OBJS) $(OUT_FILE) $(LIB_FILES) $(LIB_FILES_DBG)

# Rules
%.dll : %.cs
	$(CS) -target:library $(CS_DEBUG_FLAG) -out:$@ $<

# end of file
