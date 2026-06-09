# ═══════════════════════════════════════════════
# Shellcode Executor - GNU Make 构建系统 / Build System
# ═══════════════════════════════════════════════
# 目标 / Targets:
#   all          - 构建 Release 版本 / Build release binary
#   debug        - 构建 Debug 版本 / Build with debug symbols
#   test         - 编译并运行单元测试 / Build and run unit tests
#   clean        - 清理构建产物 / Remove build artifacts
#   format       - 格式化代码 (clang-format) / Format source code
#   analyze      - 静态分析 (clang-tidy) / Run static analysis
#   coverage     - 代码覆盖率报告 / Generate code coverage report
#   docs         - 生成 Doxygen 文档 / Generate Doxygen documentation
# ═══════════════════════════════════════════════

# 编译器与参数 / Compiler & flags
CC       := gcc
CFLAGS   := -Wall -Wextra -Wpedantic -Wshadow -Wstrict-prototypes \
            -Wmissing-prototypes -Wconversion -Wformat=2 \
            -std=c11 -D_GNU_SOURCE
# 跨平台链接库 / Cross-platform link libraries
LDFLAGS  :=
ifneq ($(OS),Windows_NT)
    # POSIX 系统需要 libdl (dlopen/dlsym) / POSIX needs libdl
    LDFLAGS += -ldl
else
    # Windows 需要 Winsock2 (socket 相关) / Windows needs Winsock2
    LDFLAGS += -lws2_32
endif

# 目录结构 / Directory structure
SRC_DIR  := src
BLD_DIR  := build
LIB_DIR  := $(BLD_DIR)/lib
BIN_DIR  := $(BLD_DIR)/bin
OBJ_DIR  := $(BLD_DIR)/obj

# 源文件 / Source files
SRCS     := $(wildcard $(SRC_DIR)/**/*.c) $(wildcard $(SRC_DIR)/*.c)
OBJS     := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# 产物 / Build artifacts
TARGET   := $(BIN_DIR)/scloader-cli
LIBRARY  := $(LIB_DIR)/libscloader.a

# ═══════════════════════════════════════════════
# 默认目标 / Default Target
# ═══════════════════════════════════════════════
.PHONY: all
all: release

.PHONY: release
release: CFLAGS += -O2 -DNDEBUG
release: $(TARGET)

.PHONY: debug
debug: CFLAGS += -O0 -g3 -DDEBUG
debug: $(TARGET)

# ═══════════════════════════════════════════════
# 目标文件编译 / Object Compilation
# ═══════════════════════════════════════════════
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -c $< -o $@

# ═══════════════════════════════════════════════
# 静态库 / Static Library
# ═══════════════════════════════════════════════
$(LIBRARY): $(OBJS)
	@mkdir -p $(LIB_DIR)
	ar rcs $@ $^

# ═══════════════════════════════════════════════
# 可执行文件 / Executable
# ═══════════════════════════════════════════════
$(TARGET): $(LIBRARY)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $@ $^ $(LDFLAGS)

# ═══════════════════════════════════════════════
# 单元测试 / Unit Tests
# ═══════════════════════════════════════════════
TEST_DIR  := tests
TEST_SRCS := $(wildcard $(TEST_DIR)/*.c)
TEST_BINS := $(patsubst $(TEST_DIR)/%.c, $(BIN_DIR)/test_%, $(TEST_SRCS))

$(BIN_DIR)/test_%: $(TEST_DIR)/%.c $(LIBRARY)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $@ $^ $(LDFLAGS)

.PHONY: test
test: CFLAGS += -O0 -g3 -DDEBUG -DUNIT_TEST
test: $(TARGET) $(TEST_BINS)
	@echo "═══════════════════════════════════════"
	@echo "  运行单元测试 / Running Unit Tests..."
	@echo "═══════════════════════════════════════"
	@for test in $(TEST_BINS); do \
		echo "  [运行/RUN]  $$(basename $$test)"; \
		$$test || exit 1; \
		echo "  [通过/PASS] $$(basename $$test)"; \
		echo "─────────────────────────────────────"; \
	done
	@echo "  全部测试通过 / All tests passed! ✓"

# ═══════════════════════════════════════════════
# 清理 / Clean
# ═══════════════════════════════════════════════
.PHONY: clean
clean:
	rm -rf $(BLD_DIR)
	rm -rf coverage/
	rm -rf docs/html docs/latex
	@echo "  构建产物已清理 / Build artifacts cleaned."

.PHONY: distclean
distclean: clean
	rm -rf .cache compile_commands.json

# ═══════════════════════════════════════════════
# 代码格式化 / Code Formatting
# ═══════════════════════════════════════════════
.PHONY: format
format:
	@command -v clang-format >/dev/null 2>&1 || { \
		echo "clang-format 未找到 / not found."; exit 1; }
	clang-format -i $(shell find $(SRC_DIR) $(TEST_DIR) examples -name "*.c" -o -name "*.h" 2>/dev/null)
	@echo "  代码已格式化 / Source code formatted. ✓"

# ═══════════════════════════════════════════════
# 静态分析 / Static Analysis
# ═══════════════════════════════════════════════
.PHONY: analyze
analyze:
	@command -v clang-tidy >/dev/null 2>&1 || { \
		echo "clang-tidy 未找到 / not found."; exit 1; }
	@echo "运行静态分析 / Running static analysis..."
	@for src in $(SRCS); do \
		clang-tidy $$src -- -I$(SRC_DIR) -std=c11 -D_GNU_SOURCE || true; \
	done

# ═══════════════════════════════════════════════
# 代码覆盖率 / Code Coverage
# ═══════════════════════════════════════════════
.PHONY: coverage
coverage: CFLAGS += -O0 -g3 --coverage
coverage: LDFLAGS += --coverage
coverage: test
	@command -v gcovr >/dev/null 2>&1 || { \
		echo "gcovr 未找到 / not found. Install: pip install gcovr"; exit 1; }
	gcovr -r . --html --html-details -o coverage/report.html
	@echo "  覆盖率报告 / Coverage report: coverage/report.html"

# ═══════════════════════════════════════════════
# 文档生成 / Documentation
# ═══════════════════════════════════════════════
.PHONY: docs
docs:
	@command -v doxygen >/dev/null 2>&1 || { \
		echo "Doxygen 未找到 / not found."; exit 1; }
	doxygen Doxyfile 2>/dev/null || true
	@echo "  文档已生成 / Documentation generated in docs/html/"

# ═══════════════════════════════════════════════
# 帮助 / Help
# ═══════════════════════════════════════════════
.PHONY: help
help:
	@echo "Shellcode Executor - 构建系统 / Build System"
	@echo "═══════════════════════════════════════"
	@echo "  make            - 构建 Release 版本 / Build release"
	@echo "  make debug      - 构建 Debug 版本 / Build with debug"
	@echo "  make test       - 编译并测试 / Build and test"
	@echo "  make clean      - 清理 / Clean"
	@echo "  make format     - 格式化代码 / Format code"
	@echo "  make analyze    - 静态分析 / Static analysis"
	@echo "  make coverage   - 覆盖率报告 / Coverage report"
	@echo "  make docs       - 生成文档 / Generate docs"
	@echo "  make help       - 本帮助 / This help"

# ═══════════════════════════════════════════════
# 伪目标 / Phony Targets
# ═══════════════════════════════════════════════
.PHONY: all release debug test clean distclean
.PHONY: format analyze coverage docs help