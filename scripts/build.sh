#!/usr/bin/env bash
# ═══════════════════════════════════════════════
# Shellcode Executor - 构建脚本 / Build Script
# ═══════════════════════════════════════════════
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_DIR}/build"

# 颜色定义 / Color definitions
COLOR_RESET="\033[0m"
COLOR_GREEN="\033[32m"
COLOR_YELLOW="\033[33m"
COLOR_CYAN="\033[36m"

info()  { echo -e "${COLOR_CYAN}[INFO]${COLOR_RESET}  $*"; }
ok()    { echo -e "${COLOR_GREEN}[OK]${COLOR_RESET}    $*"; }
warn()  { echo -e "${COLOR_YELLOW}[WARN]${COLOR_RESET}  $*"; }

usage() {
    cat <<EOF
Shellcode Executor - 构建脚本 / Build Script

用法 / Usage: $(basename "$0") [选项 / options]

选项 / Options:
  -r, --release      构建 Release 版本 / Build release (default)
  -d, --debug        构建 Debug 版本 / Build with debug symbols
  -t, --test         构建并运行测试 / Build and run tests
  -c, --clean        清理构建产物 / Clean build artifacts
  -a, --all          完整构建 + 测试 / Full build + test
  -h, --help         显示此帮助 / Show this help
EOF
    exit 0
}

# 默认配置 / Default configuration
BUILD_TYPE="Release"
DO_TEST=false
DO_CLEAN=false

while [[ $# -gt 0 ]]; do
    case "$1" in
        -r|--release)  BUILD_TYPE="Release" ;;
        -d|--debug)    BUILD_TYPE="Debug" ;;
        -t|--test)     DO_TEST=true ;;
        -c|--clean)    DO_CLEAN=true ;;
        -a|--all)      BUILD_TYPE="Debug"; DO_TEST=true ;;
        -h|--help)     usage ;;
        *)             echo "未知选项 / Unknown option: $1"; usage ;;
    esac
    shift
done

cd "$PROJECT_DIR"

# 清理模式 / Clean mode
if $DO_CLEAN; then
    info "清理构建产物 / Cleaning build artifacts..."
    rm -rf "$BUILD_DIR"
    ok "清理完成 / Clean complete."
    exit 0
fi

# 检查依赖 / Check dependencies
info "检查依赖 / Checking dependencies..."
command -v cmake >/dev/null 2>&1 || { warn "cmake 未找到 / not found"; exit 1; }
command -v make  >/dev/null 2>&1 || { warn "make 未找到 / not found";  exit 1; }
ok "所有依赖就绪 / All dependencies ready"

# CMake 配置 / CMake configuration
info "配置中 / Configuring (${BUILD_TYPE})..."
cmake -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DENABLE_TESTS=ON \
    -DENABLE_EXAMPLES=ON
ok "配置完成 / Configuration complete"

# 编译 / Build
info "编译中 / Building..."
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" -j "$(nproc 2>/dev/null || echo 4)"
ok "编译完成 / Build complete: ${BUILD_DIR}/scloader-cli"

# 测试模式 / Test mode
if $DO_TEST; then
    info "运行测试 / Running tests..."
    cd "$BUILD_DIR"
    ctest --output-on-failure -j "$(nproc 2>/dev/null || echo 4)"
    ok "全部测试通过 / All tests passed!"
fi

echo ""
echo "═══════════════════════════════════════"
echo "  构建成功 / Build Successful!"
echo "  二进制文件 / Binary: ${BUILD_DIR}/scloader-cli"
echo "═══════════════════════════════════════"