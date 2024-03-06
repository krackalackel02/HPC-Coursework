# build directory for *.d and *.o files
BUILD_DIR = ./build
# bin directory for all built executables
BIN_DIR = ./bin
# entry point for all source files
CODE_DIR = ./code
# output directory for any generated text files
OUTPUT_DIR = ./output
# include directory for location of headers
INCDIRS = ./code/include
# compiler
CC = g++
# optimisation
OPT = -O0
# Dependancy uto generation
DEPFLAGS = -MP -MD
# Complete C flags used in compilation
CFLAGS = -Wall -Wextra -g $(foreach d,$(INCDIRS),-I$(d)) $(OPT) $(DEPFLAGS)
# libraries utilised by main "solver"
MAIN_LIBS = blas boost_program_options
# append -l to each
MAIN_LIB_FLAGS = $(foreach d,$(MAIN_LIBS),-l$(d))
# libaries used by test suite
TEST_LIBS = blas
# append -l to each
TEST_LIB_FLAGS = $(foreach d,$(TEST_LIBS),-l$(d))
# main solver file name.cpp
SOLVER_FILE = LidDrivenCavitySolver
# main solver final executable name
SOLVER_OUT_NAME = solver
# main test file name.cpp
TEST_FILE = test
# main test final executable name
TEST_OUT_NAME = unit-tests
# phony target to build all main files and any other desired "main" files
PHONY_BINARY = $(SOLVER_FILE)
# phony target to build all test files and any other desired "test" files
PHONY_TEST = $(TEST_FILE)
# combine all targets into the list of desired executables
EXECUTABLES = $(PHONY_BINARY) $(PHONY_TEST)

# shell script to find all cpp files in project
SRC_FILES = $(notdir $(shell find -name '*.cpp'))
# shell script to find exact files path of all executables cpp
EXECUTABLE_FILES = $(foreach exec,$(EXECUTABLES),$(shell find -type f -name "$(exec).cpp"))
# extract path name without cpp for target rules
TARGETS = $(basename $(notdir $(EXECUTABLE_FILES)))
# filter out any executables to form list of linked object files that will be used in link stge for each independant "main" file
CFILES  = $(filter-out $(notdir $(EXECUTABLE_FILES)),$(SRC_FILES))
# Append correct file extension
OBJ_FILES = $(CFILES:.cpp=.o)
DEP_FILES = $(SRC_FILES:.cpp=.d)

# include path for Makefile auto dependancy tracking
-include $(patsubst %, $(BUILD_DIR)/%,$(DEP_FILES))

# Vpath used for makefile auto file locating for build process
VPATH = $(BUILD_DIR) $(filter-out $(CODE_DIR) $(INCDIRS),$(shell find $(CODE_DIR) -type d))

# updated phony targets so Makefile knows these arent files
.PHONY: all \
		clean-all \
		clean-up \
		debug \
		docs \
		git-publish-docs \
		git-commit \
		git-push \
		git-log \
		default \
		solver \
		unittests \
		required_dir


# set default build to be for main solver executable
.DEFAULT_GOAL = default

# ensuring directory required for build process are instantiated
default: required_dir solver

# target to build all solver and test executables
all:required_dir $(TARGETS)

required_dir : 
	@mkdir  -p $(BUILD_DIR)
	@mkdir  -p $(BIN_DIR)
	@mkdir  -p $(OUTPUT_DIR)

# debugging script to check makefiles targets and rule setting
debug:
	@echo "\n-------------------------------"
	@echo "***DEBUG:***\n"
	@echo "EXECUTABLES: $(EXECUTABLES)"
	@echo "TARGETS: $(TARGETS)"
	@echo "CPP_FILES: $(SRC_FILES)"
	@echo "OBJ_FILES: $(OBJ_FILES)"
	@echo "DEP_FILES NAMES: $(DEP_FILES)"
	@echo "DEP_FILES Locations: $(patsubst %, $(BUILD_DIR)/%,$(DEP_FILES))"
	@echo "SOLVER_FILE: $(filter %$(SOLVER_FILE), $(TARGETS))"
	@echo "VPATH: $(VPATH)"
	@echo "-------------------------------"

# main "solver" executable
solver:$(SOLVER_FILE).o $(OBJ_FILES)
	@echo "\n-------------------------------"
	@echo "***MAIN EXE FILE BUILD:***\n"
	@echo "Combining \n\t $^ \n into executable \n\t $(BIN_DIR)/$(SOLVER_OUT_NAME)"
	$(CC) $(CFLAGS) $(patsubst %, $(BUILD_DIR)/%,$(notdir $^)) $(MAIN_LIB_FLAGS) -o $(BIN_DIR)/$(SOLVER_OUT_NAME)
	@echo "-------------------------------"

# main "test" executable
unittests:$(TEST_FILE).o $(OBJ_FILES)
	@echo "\n-------------------------------"
	@echo "***Test EXE FILE BUILD:***\n"
	@echo "Combining \n\t $^ \n into executable \n\t $(BIN_DIR)/$(TEST_OUT_NAME)"
	$(CC) $(CFLAGS) $(patsubst %, $(BUILD_DIR)/%,$(notdir $^)) $(TEST_LIB_FLAGS) -o $(BIN_DIR)/$(TEST_OUT_NAME)
	@echo "-------------------------------"

# filters main "solver" from TARGETS as special output name and libs needed
$(filter %$(SOLVER_FILE), $(TARGETS)): %: %.o $(OBJ_FILES)
	@echo "\n-------------------------------"
	@echo "***MAIN EXE FILE BUILD:***\n"
	@echo "Combining \n\t $^ \n into executable \n\t $(BIN_DIR)/$(SOLVER_OUT_NAME)"
	$(CC) $(CFLAGS) $(patsubst %, $(BUILD_DIR)/%,$(notdir $^)) $(MAIN_LIB_FLAGS) -o $(BIN_DIR)/$(SOLVER_OUT_NAME)
	@echo "-------------------------------"

# filters main "test" from TARGETS as special output name and libs needed
$(filter %$(TEST_FILE), $(TARGETS)): %: %.o $(OBJ_FILES)
	@echo "\n-------------------------------"
	@echo "***Test EXE FILE BUILD:***\n"
	@echo "Combining \n\t $^ \n into executable \n\t $(BIN_DIR)/$(TEST_OUT_NAME)"
	$(CC) $(CFLAGS) $(patsubst %, $(BUILD_DIR)/%,$(notdir $^)) $(TEST_LIB_FLAGS) -o $(BIN_DIR)/$(TEST_OUT_NAME)
	@echo "-------------------------------"

# remaining "main" file targets to be built with no special output name
$(filter-out %$(SOLVER_FILE) %$(TEST_FILE), $(TARGETS)):  %: %.o $(OBJ_FILES)
	@echo "\n-------------------------------"
	@echo "***NON-MAIN EXE FILE BUILD:***\n"
	@echo "Combining \n\t $^ \n into executable \n\t $(BIN_DIR)/$(notdir $@)"
	$(CC) $(CFLAGS) $(patsubst %, $(BUILD_DIR)/%,$(notdir $^)) $(MAIN_LIB_FLAGS) -o $(BIN_DIR)/$(notdir $@)
	@echo "-------------------------------"

# script to catch all object files and compile into build directory
%.o: %.cpp
	@echo "\n-------------------------------"
	@echo "***OBJECT FILE BUILD:***\n"
	@echo "Compiling \n\t $^ \n into \n\t $@"
	$(CC) $(CFLAGS) -c $< -o $(BUILD_DIR)/$@
	@echo "-------------------------------"

# Checking what files have been built for cleanup
BUILT_OBJ_FILES = $(shell find -wholename '$(BUILD_DIR)/*.o')
BUILT_DEP_FILES = $(shell find -wholename '$(BUILD_DIR)/*.d')
BUILT_BIN_FILES = $(shell find -wholename '$(BIN_DIR)/*')
BUILT_TXT_FILES = $(shell find -wholename '$(OUTPUT_DIR)/*.txt')

# cleaning up all files including executables
clean-all:
	@echo "\n-------------------------------"
	@echo "***CLEAN ALL:***\n"
	@echo "Removing All except source files:"
	@echo "\tOBJ_FILES: $(BUILT_OBJ_FILES)"
	@echo "\tDEP_FILES: $(BUILT_DEP_FILES)"
	@echo "\tEXE_FILES: $(BUILT_BIN_FILES)"
	@echo "\tTXT_FILES: $(BUILT_TXT_FILES)"
	@if [ -n "$(BUILT_OBJ_FILES)$(BUILT_DEP_FILES)$(BUILT_BIN_FILES)$(BUILT_TXT_FILES)" ]; then \
		echo "Removing files..."; \
		rm -f  $(BUILT_OBJ_FILES) $(BUILT_DEP_FILES) $(BUILT_BIN_FILES) $(BUILT_TXT_FILES); \
	else \
		echo "No files to remove."; \
	fi
	@echo "-------------------------------"


# cleaning up all files excluding executables
clean-up:
	@echo "\n-------------------------------"
	@echo "***CLEAN UP:***\n"
	@echo "Cleaning up and Removing all build files:"
	@echo "\tOBJ_FILES: $(BUILT_OBJ_FILES)"
	@echo "\tDEP_FILES: $(BUILT_DEP_FILES)"
	@if [ -n "$(BUILT_OBJ_FILES)$(BUILT_DEP_FILES)" ]; then \
		echo "Removing files..."; \
		rm -f  $(BUILT_OBJ_FILES) $(BUILT_DEP_FILES) ; \
	else \
		echo "No files to remove."; \
	fi
	@echo "-------------------------------"


# # # The development process should involve publishing and then comiiting and pushing to main and then ghpages
# # # 		1. make
# # # 		2. make commit MESG="this is a test message"
# # # 		3. make push
# # # 		4. make publish-docs
doc:
	cd ./docs && doxygen
doc-open: doc
	xdg-open ./docs/html/index.html

git-commit:
ifndef MSG
	$(error MSG variable is not set. Usage: make git-commit MSG="Your commit message")
else
	git add .
	git commit -m "${MSG}"
endif
git-push:
	git push origin main
git-publish-docs:
	git subtree push --prefix docs/html origin gh-pages
git-log:
	git log --name-status > repository.log
	make git-commit MSG="Updated Log"
	make git-push
