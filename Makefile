# build directory for *.d and *.o files
BUILD_DIR = build
# bin directory for all built executables  - write "." for root 
BIN_DIR = bin
# entry point for all source files
CODE_DIR = code
# output directory for any generated text files
OUTPUT_DIR = output
# include directory for location of headers
INC_DIR = code/include
# compiler
CC = OMPI_CXX=g++-10 mpicxx -fopenmp
ifndef NP
NP = 4
endif

ifndef NT
NT = 4
endif
# runner
RUN = mpirun --bind-to none
# optimisation
OPT =
# Dependancy uto generation
DEPFLAGS = -MMD
# Complete C flags used in compilation
CFLAGS = -Wall -g $(foreach d,$(INC_DIR),-I$(d)) $(OPT) $(DEPFLAGS)
# Complete L flags used in compilation
LFLAGS = -Wall -g
# libraries utilised by main "solver"
MAIN_LIBS = blas boost_program_options boost_timer
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
TEST_FILE = UnitTest
# main test final executable name
TEST_OUT_NAME = unit-tests
# executable extension
OUT_EXE =
# phony target to build all main files and any other desired "main" files
PHONY_BINARY = $(SOLVER_FILE)
# phony target to build all test files and any other desired "test" files
PHONY_TEST = $(TEST_FILE)
# combine all targets into the list of desired executables
EXECUTABLES = $(PHONY_BINARY) $(PHONY_TEST)

# shell script to find all cpp files in project
# Find all .cpp files first
ALL_CPP_FILES = $(shell find . -wholename '*.cpp')
# Filter out .cpp files that contain "Test" anywhere in the filename
MAIN_SRC_FILES = $(notdir $(shell find $(CODE_DIR)/src -wholename '*.cpp'))
# Keep only .cpp files that contain "Test" anywhere in the filename
TEST_SRC_FILES = $(notdir $(shell find $(CODE_DIR)/test -wholename '*.cpp'))
# shell script to find exact files path of all executables cpp
EXECUTABLE_FILES = $(foreach exec,$(EXECUTABLES),$(shell find -type f -name "$(exec).cpp"))
# extract path name without cpp for target rules
TARGETS = $(basename $(notdir $(EXECUTABLE_FILES)))
# filter out any executables to form list of linked object files that will be used in link stge for each independant "main" file
MAIN_CFILES  = $(filter-out $(notdir $(EXECUTABLE_FILES)),$(MAIN_SRC_FILES))
# Append correct file extension
MAIN_OBJ_FILES =$(patsubst %, $(BUILD_DIR)/%, $(MAIN_CFILES:.cpp=.o))
# filter out any executables to form list of linked object files that will be used in link stge for each independant "main" file
TEST_CFILES  = $(filter-out $(notdir $(EXECUTABLE_FILES)),$(TEST_SRC_FILES))
# Append correct file extension
TEST_OBJ_FILES = $(patsubst %, $(BUILD_DIR)/%,$(TEST_CFILES:.cpp=.o))
DEP_FILES = $(MAIN_SRC_FILES:.cpp=.d) $(TEST_SRC_FILES:.cpp=.d)

# set default build to be for main solver executable
.DEFAULT_GOAL = default
# include path for Makefile auto dependancy tracking
-include $(patsubst %, $(BUILD_DIR)/%,$(DEP_FILES))

# Vpath used for makefile auto file locating for build process
VPATH = $(BUILD_DIR) $(shell find $(CODE_DIR) -type d)

# updated phony targets so Makefile knows these arent files
.PHONY: all \
		clean-all \
		clean-up \
		clean-output \
		debug \
		docs \
		git-publish-docs \
		git-commit \
		git-push \
		git-log \
		default \
		solver \
		unittests \
        required_dir \
        tree \
		analyze


# ensuring directory required for build process are instantiated
default: required_dir solver
analyze: required_dir solver
	export OMP_NUM_THREADS=$(shell echo 1);\
	collect -o $(OUTPUT_DIR)/data.er $(RUN) -np 1 $(BIN_DIR)/$(SOLVER_OUT_NAME)$(OUT_EXE)
# rule to build and run main solver with set threads and procs
runsolver: required_dir solver
	export OMP_NUM_THREADS=$(shell echo $(NT));\
	$(RUN) -np $(NP) $(BIN_DIR)/$(SOLVER_OUT_NAME)$(OUT_EXE);\
# rule to build and run unittests with set threads and procs
rununittests: required_dir unittests
	export OMP_NUM_THREADS=$(shell echo $(NT));\
	$(RUN) -np $(NP) $(BIN_DIR)/$(TEST_OUT_NAME)$(OUT_EXE);
dev: required_dir solver unittests
	export OMP_NUM_THREADS=$(shell echo $(NT));\
	$(RUN) -np $(NP) $(BIN_DIR)/$(TEST_OUT_NAME)$(OUT_EXE);\
	$(RUN) -np $(NP) $(BIN_DIR)/$(SOLVER_OUT_NAME)$(OUT_EXE);

# target to build all solver and test executables
all:required_dir $(TARGETS)

required_dir : 
	clear
	@echo "\n-------------------------------"
	@echo "***Making Required Directories:***\n"
	@mkdir  -p $(BUILD_DIR)
	@mkdir  -p $(BIN_DIR)
	@mkdir  -p $(OUTPUT_DIR)
	@echo "-------------------------------"

# debugging script to check makefiles targets and rule setting
debug:
	@echo "\n-------------------------------"
	@echo "***DEBUG:***\n"
	@echo "TEST: $(TEST)"
	@echo "EXECUTABLES: $(EXECUTABLES)"
	@echo "TARGETS: $(TARGETS)"
	@echo "MAIN_CPP_FILES: $(MAIN_SRC_FILES)"
	@echo "TEST_CPP_FILES: $(TEST_SRC_FILES)"
	@echo "MAIN_OBJ_FILES: $(MAIN_OBJ_FILES)"
	@echo "TEST_OBJ_FILES: $(TEST_OBJ_FILES)"
	@echo "DEP_FILES NAMES: $(DEP_FILES)"
	@echo "DEP_FILES Locations: $(patsubst %, $(BUILD_DIR)/%,$(DEP_FILES))"
	@echo "SOLVER_FILE: $(filter %$(SOLVER_FILE), $(TARGETS))"
	@echo "VPATH: $(VPATH)"
	@echo "-------------------------------"

# main "solver" executable
solver:required_dir $(BUILD_DIR)/$(SOLVER_FILE).o $(MAIN_OBJ_FILES)
	@echo "\n-------------------------------"
	@echo "***MAIN EXE FILE BUILD:***\n"
	@echo "Combining \n\t $(filter-out $<,$^) \n into executable \n\t $(BIN_DIR)/$(SOLVER_OUT_NAME)$(OUT_EXE)"
	$(CC) $(LFLAGS) $(patsubst %, $(BUILD_DIR)/%,$(notdir $(filter-out $<,$^))) $(MAIN_LIB_FLAGS) -o $(BIN_DIR)/$(SOLVER_OUT_NAME)$(OUT_EXE)
	@echo "-------------------------------\n"
	@echo "You can run the solver like \"mpiexec -np *process num* $(BIN_DIR)/$(SOLVER_OUT_NAME)$(OUT_EXE) --nt *num threads*\"\n"

# main "test" executable
unittests:required_dir $(BUILD_DIR)/$(TEST_FILE).o $(MAIN_OBJ_FILES) $(TEST_OBJ_FILES)
	@echo "\n-------------------------------"
	@echo "***Test EXE FILE BUILD:***\n"
	@echo "Combining \n\t $(filter-out $<,$^) \n into executable \n\t $(BIN_DIR)/$(TEST_OUT_NAME)$(OUT_EXE)"
	$(CC) $(LFLAGS) $(patsubst %, $(BUILD_DIR)/%,$(notdir $(filter-out $<,$^))) $(TEST_LIB_FLAGS) -o $(BIN_DIR)/$(TEST_OUT_NAME)$(OUT_EXE)
	@echo "-------------------------------\n"
	@echo "Make sure to set OMP_NUM_THREADS for omp testing and not default vals\n"
	@echo "You can run the solver like \"mpiexec -np *process num* $(BIN_DIR)/$(TEST_OUT_NAME)$(OUT_EXE)\"\n"

# filters main "solver" from TARGETS as special output name and libs needed
$(filter %$(SOLVER_FILE), $(TARGETS)): %: $(BUILD_DIR)/%.o $(MAIN_OBJ_FILES)
	@echo "\n-------------------------------"
	@echo "***MAIN EXE FILE BUILD:***\n"
	@echo "Combining \n\t $^ \n into executable \n\t $(BIN_DIR)/$(SOLVER_OUT_NAME)$(OUT_EXE)"
	$(CC) $(LFLAGS) $(patsubst %, $(BUILD_DIR)/%,$(notdir $^)) $(MAIN_LIB_FLAGS) -o $(BIN_DIR)/$(SOLVER_OUT_NAME)$(OUT_EXE)
	@echo "-------------------------------\n"
	@echo "You can run the solver like \"mpiexec -np *process num* $(BIN_DIR)/$(SOLVER_OUT_NAME)$(OUT_EXE) --nt *num threads*\"\n"

# filters main "test" from TARGETS as special output name and libs needed
$(filter %$(TEST_FILE), $(TARGETS)): %: $(BUILD_DIR)/%.o $(MAIN_OBJ_FILES) $(TEST_OBJ_FILES)
	@echo "\n-------------------------------"
	@echo "***Test EXE FILE BUILD:***\n"
	@echo "Combining \n\t $^ \n into executable \n\t $(BIN_DIR)/$(TEST_OUT_NAME)$(OUT_EXE)"
	$(CC) $(LFLAGS) $(patsubst %, $(BUILD_DIR)/%,$(notdir $^)) $(TEST_LIB_FLAGS) -o $(BIN_DIR)/$(TEST_OUT_NAME)$(OUT_EXE)
	@echo "-------------------------------\n"
	@echo "Make sure to set OMP_NUM_THREADS for omp testing and not default vals\n"
	@echo "You can run the solver like \"mpiexec -np *process num* $(BIN_DIR)/$(TEST_OUT_NAME)$(OUT_EXE)\"\n"

# remaining "main" file targets to be built with no special output name
$(filter-out %$(SOLVER_FILE) %$(TEST_FILE), $(TARGETS)):  %: $(BUILD_DIR)/%.o $(MAIN_OBJ_FILES)
	@echo "\n-------------------------------"
	@echo "***NON-MAIN EXE FILE BUILD:***\n"
	@echo "Combining \n\t $^ \n into executable \n\t $(BIN_DIR)/$(notdir $(OUT_EXE)$@)"
	$(CC) $(LFLAGS) $(patsubst %, $(BUILD_DIR)/%,$(notdir $^)) $(MAIN_LIB_FLAGS) -o $(BIN_DIR)/$(notdir $(OUT_EXE)$@)
	@echo "-------------------------------\n"

# Rebuilds any prebuilt object files based on triggers in change in dep. header file or associated cpp file or even fresh build
$(BUILD_DIR)/%.o: %.cpp
	@echo "\n-------------------------------"
	@echo "***OBJECT FILE BUILD:***\n"
	@echo "Compiling \n\t $< \n into \n\t $@"
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "-------------------------------"

# Checking what files have been built for cleanup
BUILT_OBJ_FILES = $(shell find $(BUILD_DIR) -type f -name '*.o')
BUILT_DEP_FILES = $(shell find $(BUILD_DIR) -type f -name '*.d')
BUILT_BIN_FILES = $(shell find $(BIN_DIR) -type f -name '*')
BUILT_OUTPUT_FILES = $(shell find $(OUTPUT_DIR) -type f -name '*')


# cleaning up all files including executables
clean-all:
	@echo "\n-------------------------------"
	@echo "***CLEAN ALL:***\n"
	@echo "Removing All except source files:"
	@echo "\tOBJ_FILES: $(BUILT_OBJ_FILES)"
	@echo "\tDEP_FILES: $(BUILT_DEP_FILES)"
	@echo "\tEXE_FILES: $(BUILT_BIN_FILES)"
	@echo "\tOUTPUT_FILES: $(BUILT_OUTPUT_FILES)"
	@if [ -n "$(BUILT_OBJ_FILES)$(BUILT_DEP_FILES)$(BUILT_BIN_FILES)$(BUILT_OUTPUT_FILES)" ]; then \
		echo "Removing files..."; \
		rm -f  $(BUILT_OBJ_FILES) $(BUILT_DEP_FILES) $(BUILT_BIN_FILES) $(BUILT_OUTPUT_FILES); \
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
	@if [ -n "$(BUILT_OBJ_FILES)$(BUILT_DEP_FILES)$(BUILT_OUTPUT_FILES)" ]; then \
		echo "Removing files..."; \
		rm -f  $(BUILT_OBJ_FILES) $(BUILT_DEP_FILES) $(BUILT_OUTPUT_FILES) ; \
	else \
		echo "No files to remove."; \
	fi
	@echo "-------------------------------"
# cleaning up all output files
clean-output:
	@echo "\n-------------------------------"
	@echo "***CLEAN UP:***\n"
	@echo "Cleaning up all output files:"
	@if [ -n "$(BUILT_OUTPUT_FILES)" ]; then \
		echo "Removing files..."; \
		rm -f   $(BUILT_OUTPUT_FILES) ; \
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
	git checkout main
	git add .
	git commit -m "${MSG}"
endif
git-push:
	git push origin main --tags
git-publish-docs:
    # Perform the subtree push
	git subtree push --prefix docs/html origin gh-pages


git-log:
	git log --name-status > repository.log
	make git-commit MSG="Updated Log"
	make git-push
tree:
ifndef T
	git log --graph --oneline --decorate --all
else
endif
ifeq ($(T),1)
	git log --graph --abbrev-commit --decorate --format=format:'%C(bold blue)%h%C(reset) - %C(bold green)(%ar)%C(reset) %C(white)%s%C(reset) %C(dim white)- %an%C(reset)%C(auto)%d%C(reset)' --all
endif
ifeq ($(T),2)
	git log --graph --abbrev-commit --decorate --format=format:'%C(bold blue)%h%C(reset) - %C(bold cyan)%aD%C(reset) %C(bold green)(%ar)%C(reset)%C(auto)%d%C(reset)%n''          %C(white)%s%C(reset) %C(dim white)- %an%C(reset)'
endif