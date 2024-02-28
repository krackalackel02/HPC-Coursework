BUILD_DIR = ./build
BIN_DIR = ./bin
CODE_DIR = ./code
OUTPUT_DIR = ./output
INCDIRS = ./code/include
CC = g++
OPT = -O0
DEPFLAGS = -MP -MD
CFLAGS = -Wall -Wextra -g $(foreach d,$(INCDIRS),-I$(d)) $(OPT) $(DEPFLAGS)
LIBS = blas boost_program_options
LIB_FLAGS = $(foreach d,$(LIBS),-l$(d))
SOLVER_FILE = LidDrivenCavitySolver
SOLVER_OUT_NAME = solver
TEST_FILE = test
TEST_OUT_NAME = test
PHONY_BINARY = $(SOLVER_FILE)
PHONY_TEST = $(TEST_FILE)
EXECUTABLES = $(PHONY_BINARY) $(PHONY_TEST)


SRC_FILES = $(notdir $(shell find -name '*.cpp'))
EXECUTABLE_FILES = $(foreach exec,$(EXECUTABLES),$(shell find -type f -name "$(exec).cpp"))
TARGETS = $(basename $(notdir $(EXECUTABLE_FILES)))
CFILES  = $(filter-out $(notdir $(EXECUTABLE_FILES)),$(SRC_FILES))
OBJ_FILES = $(CFILES:.cpp=.o)
DEP_FILES = $(SRC_FILES:.cpp=.d)

-include $(patsubst %, $(BUILD_DIR)/%,$(DEP_FILES))

VPATH = $(BUILD_DIR) $(filter-out $(CODE_DIR) $(INCDIRS),$(shell find $(CODE_DIR) -type d))

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
		unittests


.DEFAULT_GOAL = default

default: $(BUILD_DIR) $(BIN_DIR) $(OUTPUT_DIR) solver

all:$(BUILD_DIR) $(BIN_DIR) $(OUTPUT_DIR) $(TARGETS)

$(BUILD_DIR):
	@mkdir  -p $(BUILD_DIR)

$(BIN_DIR):
	@mkdir  -p $(BIN_DIR)

$(OUTPUT_DIR):
	@mkdir  -p $(OUTPUT_DIR)

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


solver:$(SOLVER_FILE).o $(OBJ_FILES)
	@echo "\n-------------------------------"
	@echo "***MAIN EXE FILE BUILD:***\n"
	@echo "Combining \n\t $^ \n into executable \n\t $(BIN_DIR)/$(SOLVER_OUT_NAME)"
	$(CC) $(CFLAGS) -o $(BIN_DIR)/$(SOLVER_OUT_NAME) $(patsubst %, $(BUILD_DIR)/%,$(notdir $^)) $(LIB_FLAGS)
	@echo "-------------------------------"

unittests:$(TEST_FILE).o $(OBJ_FILES)
	@echo "\n-------------------------------"
	@echo "***Test EXE FILE BUILD:***\n"
	@echo "Combining \n\t $^ \n into executable \n\t $(BIN_DIR)/$(TEST_OUT_NAME)"
	$(CC) $(CFLAGS) -o $(BIN_DIR)/$(TEST_OUT_NAME) $(patsubst %, $(BUILD_DIR)/%,$(notdir $^)) $(LIB_FLAGS)
	@echo "-------------------------------"


$(filter %$(SOLVER_FILE), $(TARGETS)): %: %.o $(OBJ_FILES)
	@echo "\n-------------------------------"
	@echo "***MAIN EXE FILE BUILD:***\n"
	@echo "Combining \n\t $^ \n into executable \n\t $(BIN_DIR)/$(SOLVER_OUT_NAME)"
	$(CC) $(CFLAGS) -o $(BIN_DIR)/$(SOLVER_OUT_NAME) $(patsubst %, $(BUILD_DIR)/%,$(notdir $^)) $(LIB_FLAGS)
	@echo "-------------------------------"

$(filter %$(TEST_FILE), $(TARGETS)): %: %.o $(OBJ_FILES)
	@echo "\n-------------------------------"
	@echo "***Test EXE FILE BUILD:***\n"
	@echo "Combining \n\t $^ \n into executable \n\t $(BIN_DIR)/$(TEST_OUT_NAME)"
	$(CC) $(CFLAGS) -o $(BIN_DIR)/$(TEST_OUT_NAME) $(patsubst %, $(BUILD_DIR)/%,$(notdir $^)) $(LIB_FLAGS)
	@echo "-------------------------------"

$(filter-out %$(SOLVER_FILE) %$(TEST_FILE), $(TARGETS)):  %: %.o $(OBJ_FILES)
	@echo "\n-------------------------------"
	@echo "***NON-MAIN EXE FILE BUILD:***\n"
	@echo "Combining \n\t $^ \n into executable \n\t $(BIN_DIR)/$(notdir $@)"
	$(CC) $(CFLAGS) -o $(BIN_DIR)/$(notdir $@) $(patsubst %, $(BUILD_DIR)/%,$(notdir $^)) $(LIB_FLAGS)
	@echo "-------------------------------"

%.o: %.cpp
	@echo "\n-------------------------------"
	@echo "***OBJECT FILE BUILD:***\n"
	@echo "Compiling \n\t $^ \n into \n\t $@"
	$(CC) $(CFLAGS) -c $< -o $(BUILD_DIR)/$@
	@echo "-------------------------------"

BUILT_OBJ_FILES = $(shell find -wholename '$(BUILD_DIR)/*.o')
BUILT_DEP_FILES = $(shell find -wholename '$(BUILD_DIR)/*.d')
BUILT_BIN_FILES = $(shell find -wholename '$(BIN_DIR)/*')
BUILT_TXT_FILES = $(shell find -wholename '$(OUTPUT_DIR)/*.txt')

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
