PROJECT := ttsflow

# silent mode, enable pretty build (comment to see full commands)
Q ?= @

BUILD_DIR := build
BUILD_DIR_LINK := $(BUILD_DIR)
ifeq ($(RELEASE_BUILD_DIR),)
	RELEASE_BUILD_DIR := .$(BUILD_DIR)_release
endif
ifeq ($(DEBUG_BUILD_DIR),)
	DEBUG_BUILD_DIR := .$(BUILD_DIR)_debug
endif

DEBUG ?= 0
ifeq ($(DEBUG), 1)
	BUILD_DIR := $(DEBUG_BUILD_DIR)
	OTHER_BUILD_DIR := $(RELEASE_BUILD_DIR)
else
	BUILD_DIR := $(RELEASE_BUILD_DIR)
	OTHER_BUILD_DIR := $(DEBUG_BUILD_DIR)
endif

# All of the directories containing code.
SRC_DIRS := $(shell find ttsflow -type d -exec bash -c "find {} -maxdepth 1 \
	\( -name '*.cc' \) | grep -q ." \; -print)

# The target shared library name
LIBRARY_NAME := $(PROJECT)
LIB_BUILD_DIR := $(BUILD_DIR)/lib
STATIC_NAME := $(LIB_BUILD_DIR)/lib$(LIBRARY_NAME).a
DYNAMIC_VERSION_MAJOR 		:= 1
DYNAMIC_VERSION_MINOR 		:= 0
DYNAMIC_VERSION_REVISION 	:= 0
DYNAMIC_NAME_SHORT := lib$(LIBRARY_NAME).so
DYNAMIC_VERSIONED_NAME_SHORT := $(DYNAMIC_NAME_SHORT).$(DYNAMIC_VERSION_MAJOR).$(DYNAMIC_VERSION_MINOR).$(DYNAMIC_VERSION_REVISION)
DYNAMIC_NAME := $(LIB_BUILD_DIR)/$(DYNAMIC_VERSIONED_NAME_SHORT)
COMMON_FLAGS += -DTTSFLOW_VERSION=$(DYNAMIC_VERSION_MAJOR).$(DYNAMIC_VERSION_MINOR).$(DYNAMIC_VERSION_REVISION)

##############################
# Get all source files
##############################

# CXX_SRCS are the source files excluding the test ones
CXX_SRCS := $(shell find ttsflow ! -name "*_test.cc" -name "*.cc")
# TEST_SRCS are the test source files
TEST_SRCS := $(shell find ttsflow/tests -name "*_test.cc")
# EXAMPLE_SRCS are the source files for the example binaries
EXAMPLE_SRCS := $(shell find ttsflow/examples -name "*.cc")

# The objects corresponding to the source files
OBJS := $(addprefix $(BUILD_DIR)/, ${CXX_SRCS:.cc=.o})
TEST_BUILD_DIR := $(BUILD_DIR)/ttsflow/tests
TEST_OBJS := $(addprefix $(BUILD_DIR)/, ${TEST_SRCS:.cc=.o})
EXAMPLE_OBJS := $(addprefix $(BUILD_DIR)/, ${EXAMPLE_SRCS:.cc=.o})

# Output files for automatic dependency generation
DEPS := ${CXX_OBJS:.o=.d}

EXAMPLE_BINS := ${EXAMPLE_OBJS:.o=.bin}
# Put the test binaries in build/test for convenience.
TEST_BIN_DIR := $(BUILD_DIR)/test
TEST_BINS := $(addsuffix .testbin,$(addprefix $(TEST_BIN_DIR)/, \
		$(foreach obj,$(TEST_OBJS),$(basename $(notdir $(obj))))))

##############################
# Derive compiler warning dump locations
##############################

WARNS_EXT := warnings.txt
CXX_WARNS := $(addprefix $(BUILD_DIR)/, ${CXX_SRCS:.cc=.o.$(WARNS_EXT)})
EXAMPLE_WARNS := $(addprefix $(BUILD_DIR)/, ${EXAMPLE_SRCS:.cc=.o.$(WARNS_EXT)})
TEST_WARNS := $(addprefix $(BUILD_DIR)/, ${TEST_SRCS:.cc=.o.$(WARNS_EXT)})
ALL_WARNS := $(CXX_WARNS) $(EXAMPLE_WARNS) $(TEST_WARNS)

EMPTY_WARN_REPORT := $(BUILD_DIR)/.$(WARNS_EXT)
NONEMPTY_WARN_REPORT := $(BUILD_DIR)/$(WARNS_EXT)

##############################
# Derive include and lib directories
##############################

INCLUDE_DIRS += ./ ./third_party/eigen3 \
				./third_party/World/tools \
				./third_party/World/src \
				./third_party/protobuf/include \
				./third_party/tensorflow \
				./third_party/tensorflow/bazel-genfiles

LIBRARIES += protobuf tensorflow_cc world

##############################
# Set build directories
##############################
ALL_BUILD_DIRS := $(sort $(BUILD_DIR) $(addprefix $(BUILD_DIR)/, $(SRC_DIRS)) \
	$(LIB_BUILD_DIR) $(TEST_BUILD_DIR) $(TEST_BIN_DIR))

##############################
# Configure build
##############################

# Determine platform
UNAME := $(shell uname -s)
ifeq ($(UNAME), Linux)
	LINUX := 1
else ifeq ($(UNAME), Darwin)
	OSX := 1
endif

# Linux
ifeq ($(LINUX), 1)
	CXX := /usr/bin/g++ -std=c++11
	GCCVERSION := $(shell $(CXX) -dumpversion | cut -f1,2 -d.)
	# older versions of gcc are too dumb to build boost with -Wuninitalized
	ifeq ($(shell echo | awk '{exit $(GCCVERSION) < 4.6;}'), 1)
		WARNINGS += -Wno-uninitialized
	endif
	VERSIONFLAGS += -Wl,-soname,$(DYNAMIC_VERSIONED_NAME_SHORT) -Wl,-rpath,$(ORIGIN)/../lib
endif

# OS X:
# clang++ instead of g++
# libstdc++ for NVCC compatibility on OS X >= 10.9 with CUDA < 7.0
ifeq ($(OSX), 1)
	CXX := /usr/bin/clang++
	# we need to explicitly ask for the rpath to be obeyed
	ORIGIN := @loader_path
	VERSIONFLAGS += -Wl,-install_name,@rpath/$(DYNAMIC_VERSIONED_NAME_SHORT) -Wl,-rpath,$(ORIGIN)/../../build/lib
else
	ORIGIN := \$$ORIGIN
endif

# Custom compiler
ifdef CUSTOM_CXX
	CXX := $(CUSTOM_CXX)
endif

# Static linking
ifneq (,$(findstring clang++,$(CXX)))
	STATIC_LINK_COMMAND := -Wl,-force_load $(STATIC_NAME)
else ifneq (,$(findstring g++,$(CXX)))
	STATIC_LINK_COMMAND := -Wl,--whole-archive $(STATIC_NAME) -Wl,--no-whole-archive
else
  # The following line must not be indented with a tab, since we are not inside a target
  $(error Cannot static link with the $(CXX) compiler)
endif

# Debugging
ifeq ($(DEBUG), 1)
	COMMON_FLAGS += -DDEBUG -g -O0
else
	COMMON_FLAGS += -DNDEBUG -O2
endif

LIBRARY_DIRS += $(LIB_BUILD_DIR) \
				./third_party/protobuf/lib \
				./third_party/tensorflow/bazel-bin/tensorflow \
				./third_party/World/build

# Automatic dependency generation
CXXFLAGS += -MMD -MP

# Complete build flags.
COMMON_FLAGS += $(foreach includedir,$(INCLUDE_DIRS),-I$(includedir))
CXXFLAGS += -pthread -fPIC $(COMMON_FLAGS) $(WARNINGS)
LINKFLAGS += -pthread -fPIC $(COMMON_FLAGS) $(WARNINGS)
LDFLAGS += $(foreach librarydir,$(LIBRARY_DIRS),-L$(librarydir)) $(PKG_CONFIG) \
		$(foreach library,$(LIBRARIES),-l$(library))

# Set the sub-targets of the 'everything' target.
EVERYTHING_TARGETS := all test warn

##############################
# Define build targets
##############################

.PHONY: all lib test clean examples warn everything

all: lib examples

lib: $(STATIC_NAME) $(DYNAMIC_NAME)

everything: $(EVERYTHING_TARGETS)

test: $(TEST_BINS)

tools: $(TOOL_BINS) $(TOOL_BIN_LINKS)

examples: $(EXAMPLE_BINS)

warn: $(EMPTY_WARN_REPORT)

$(EMPTY_WARN_REPORT): $(ALL_WARNS) | $(BUILD_DIR)
	@ cat $(ALL_WARNS) > $@
	@ if [ -s "$@" ]; then \
		cat $@; \
		mv $@ $(NONEMPTY_WARN_REPORT); \
		echo "Compiler produced one or more warnings."; \
		exit 1; \
	  fi; \
	  $(RM) $(NONEMPTY_WARN_REPORT); \
	  echo "No compiler warnings!";

$(ALL_WARNS): %.o.$(WARNS_EXT) : %.o

$(BUILD_DIR_LINK): $(BUILD_DIR)/.linked

# Create a target ".linked" in this BUILD_DIR to tell Make that the "build" link
# is currently correct, then delete the one in the OTHER_BUILD_DIR in case it
# exists and $(DEBUG) is toggled later.
$(BUILD_DIR)/.linked:
	@ mkdir -p $(BUILD_DIR)
	@ $(RM) $(OTHER_BUILD_DIR)/.linked
	@ $(RM) -r $(BUILD_DIR_LINK)
	@ ln -s $(BUILD_DIR) $(BUILD_DIR_LINK)
	@ touch $@

$(ALL_BUILD_DIRS): | $(BUILD_DIR_LINK)
	@ mkdir -p $@

$(DYNAMIC_NAME): $(OBJS) | $(LIB_BUILD_DIR)
	@ echo LD -o $@
	$(Q)$(CXX) -shared -o $@ $(OBJS) $(VERSIONFLAGS) $(LINKFLAGS) $(LDFLAGS)
	@ cd $(BUILD_DIR)/lib; rm -f $(DYNAMIC_NAME_SHORT); ln -s $(DYNAMIC_VERSIONED_NAME_SHORT) $(DYNAMIC_NAME_SHORT)

$(STATIC_NAME): $(OBJS) | $(LIB_BUILD_DIR)
	@ echo AR -o $@
	$(Q)ar rcs $@ $(OBJS)

$(BUILD_DIR)/%.o: %.cc | $(ALL_BUILD_DIRS)
	@ echo CXX $<
	$(Q)$(CXX) $< $(CXXFLAGS) -c -o $@ 2> $@.$(WARNS_EXT) \
		|| (cat $@.$(WARNS_EXT); exit 1)
	@ cat $@.$(WARNS_EXT)

$(TEST_BINS): $(TEST_BIN_DIR)/%.testbin: $(TEST_BUILD_DIR)/%.o \
	| $(DYNAMIC_NAME) $(TEST_BIN_DIR)
	@ echo LD $<
	$(Q)$(CXX) $(TEST_MAIN_SRC) $< $(GTEST_OBJ) \
		-o $@ $(LINKFLAGS) $(LDFLAGS) -l$(LIBRARY_NAME) -Wl,-rpath,$(ORIGIN)/../lib:$(ORIGIN)/../../third_party/protobuf/lib:$(ORIGIN)/../../third_party/tensorflow/bazel-bin/tensorflow

$(EXAMPLE_BINS): %.bin : %.o | $(DYNAMIC_NAME)
	@ echo CXX/LD -o $@
	$(Q)$(CXX) $< -o $@ $(LINKFLAGS) -l$(LIBRARY_NAME) $(LDFLAGS) \
		-Wl,-rpath,$(ORIGIN)/../../lib:$(ORIGIN)/../../../third_party/protobuf/lib:$(ORIGIN)/../../../third_party/tensorflow/bazel-bin/tensorflow
	@ ln -s $(EXAMPLE_BINS) ttsflow_main

clean:
	@- $(RM) -rf $(ALL_BUILD_DIRS)
	@- $(RM) -rf $(OTHER_BUILD_DIR)
	@- $(RM) -rf $(BUILD_DIR_LINK)
	@- $(RM) -rf ttsflow_main

-include $(DEPS)
