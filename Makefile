INCDIRS = . ./include
CC = g++
OPT = -O0
DEPFLAGS = -MP -MD
CFLAGS = -Wall -Wextra -g $(foreach d,$(INCDIRS),-I$(d)) $(OPT) $(DEPFLAGS)

PHONY_BINARY = LidDrivenCavitySolver
PHONY_TEST =
EXECUTABLES = $(PHONY_BINARY) $(PHONY_TEST)


SRC_FILES = $(shell find -name '*.cpp')
EXECUTABLE_FILES = $(foreach exec,$(EXECUTABLES),$(shell find -type f -name "$(exec).cpp"))
TARGETS = $(basename $(EXECUTABLE_FILES))
CFILES  = $(filter-out $(EXECUTABLE_FILES),$(SRC_FILES))
OBJ_FILES = $(CFILES:.cpp=.o)
DEP_FILES = $(SRC_FILES:.cpp=.d)

-include $(DEP_FILES)

.PHONY: all clean clean-up debug
.DEFAULT_GOAL := all

all: $(TARGETS) clean-up

debug:
	@echo "EXECUTABLES: $(EXECUTABLES)"
	@echo "TARGETS: $(TARGETS)"
	@echo "SRC_FILES: $(SRC_FILES)"
	@echo "OBJ_FILES: $(OBJ_FILES)"
	@echo "DEP_FILES: $(DEP_FILES)"

$(TARGETS):%:%.o $(OBJ_FILES)
	@echo "Combining final objects into executable $@"
	$(CC) -o bin/$(notdir $@) $(patsubst %, ./build/%,$(notdir $^)) -lblas -lboost_program_options


%.o: %.cpp
	@echo "Compiling $< into $@"
	$(CC) $(CFLAGS) -c $< -o ./build/$(notdir $@)

BUILT_OBJ_FILES = $(patsubst %,./build/%,$(notdir $(SRC_FILES:.cpp=.o)))
BUILT_DEP_FILES = $(patsubst %,./build/%,$(notdir $(DEP_FILES)))
BUILT_BIN_FILES = $(patsubst %,./bin/%, $(notdir $(TARGETS)))
BUILT_TXT_FILES = ./output/*.txt
clean:
	@echo "Removing All except source files"
	@echo "OBJ_FILES: $(BUILT_OBJ_FILES)"
	@echo "DEP_FILES: $(BUILT_DEP_FILES)"
	@echo "EXE_FILES: $(BUILT_BIN_FILES)"
	@echo "TXT_FILES: $(BUILT_TXT_FILES)"
	rm -f  $(BUILT_OBJ_FILES) $(BUILT_DEP_FILES) $(BUILT_BIN_FILES) $(BUILT_TXT_FILES)

clean-up:
	@echo "Cleaning up and Removing all build files"
	@echo "OBJ_FILES: $(BUILT_OBJ_FILES)"
	@echo "DEP_FILES: $(BUILT_DEP_FILES)"
	rm -f $(BUILT_OBJ_FILES) $(BUILT_DEP_FILES) 


