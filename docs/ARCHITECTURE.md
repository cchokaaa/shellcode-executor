# 架构设计 / Architecture

## 系统设计 / System Design

Shellcode Executor 遵循分层架构设计，各模块职责清晰、边界明确。
The Shellcode Executor follows a layered architecture with clear module boundaries.

### 第1层：应用层 / Layer 1: Application Layer

| 组件 / Component | 文件 / File | 说明 / Description |
|-----------------|-------------|-------------------|
| **CLI 命令行** | `src/main.c` | 基于 getopt 的参数解析与主逻辑 / Command-line interface with argument parsing |
| **示例程序** | `examples/` | API 使用参考实现 / Reference implementations demonstrating API usage |
| **辅助脚本** | `scripts/` | Python 编码工具与构建脚本 / Python encoding utility & build script |

### 第2层：核心库 / Layer 2: Core Library (`scloader`)

| 模块 / Module | 文件 / File | 职责 / Responsibility |
|--------------|-------------|----------------------|
| **Loader** 加载器 | `src/loader/` | 可执行内存管理与 Shellcode 执行 / Memory management and shellcode execution |
| **Encoder** 编码器 | `src/encoder/` | 数据变换（XOR、Base64）/ Data transformation (XOR, Base64) |
| **Utils** 工具 | `src/utils/` | Hex Dump、格式转换、调试工具 / Debugging and formatting utilities |
| **Remote** 远程 | `src/remote/` | 基于网络的 Payload 获取 / Network-based payload retrieval |
| **Crypto** 加密 | `src/crypto/` | AES-256-CBC 加解密 / AES encryption/decryption (optional) |

### 第3层：平台抽象层 / Layer 3: Platform Abstraction

| 平台 / Platform | API | 关键函数 / Key Functions |
|----------------|-----|-------------------------|
| **Windows** | Win32 API | `VirtualAlloc`, `VirtualProtect`, `VirtualFree` |
| **Linux** | POSIX | `mmap`, `mprotect`, `munmap` |
| **macOS** | POSIX + JIT | `mmap`, `pthread_jit_write_protect_np`, `mprotect` |

### 跨平台适配策略 / Cross-Platform Adaptation Strategy

```c
// 示例：加载器根据平台宏自动选择实现
// Example: Loader automatically selects implementation based on platform macros

#if defined(_WIN32) || defined(_WIN64)
    // Windows: VirtualAlloc 分配 RWX 内存
    ptr = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
#elif defined(__APPLE__)
    // macOS: mmap + MAP_JIT + 特殊写保护处理
    ptr = mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC,
               MAP_PRIVATE | MAP_ANONYMOUS | MAP_JIT, -1, 0);
    pthread_jit_write_protect_np(0);  // 允许写入 JIT 区域
#else
    // Linux: 标准 mmap RWX
    ptr = mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC,
               MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif
```

## 执行流程 / Execution Flow

```ascii
输入 / Input (文件file / 十六进制hex / 远程remote)
       │
       ▼
  ┌─────────────────────┐
  │ 解码器 / Decoder     │ ←── 可选 Optional: XOR / Base64 / AES
  │ 将编码数据恢复为原始   │     Restore encoded data to raw bytes
  └─────────────────────┘
       │
       ▼
  ┌─────────────────────┐
  │ 加载器.分配可执行内存  │ → mmap / VirtualAlloc
  │ Loader.alloc_exec    │    分配 RWX 内存页
  └─────────────────────┘
       │
       ▼
  ┌─────────────────────┐
  │ 加载器.写入 / Write   │ → memcpy
  │ 将 Shellcode 复制     │    拷贝到分配的内存
  │ 到目标内存            │
  └─────────────────────┘
       │
       ▼
  ┌─────────────────────┐
  │ 加载器.保护 / Protect │ → mprotect / VirtualProtect (RX)
  │ 移除写权限，保留执行权限│    Remove write, keep execute
  └─────────────────────┘
       │
       ▼
  ┌─────────────────────┐
  │ 加载器.执行 / Execute │ → 函数指针调用
  │ 跳转到 Shellcode 入口 │    Jump to shellcode entry point
  └─────────────────────┘
       │
       ▼
  ┌─────────────────────┐
  │ 加载器.释放 / Free    │ → munmap / VirtualFree
  │ 清零后释放（可选）     │    Clear & release (optional)
  └─────────────────────┘
```

## 返回码定义 / Status Codes

| 状态码 / Code | 名称 / Name | 说明 / Description |
|--------------|-------------|-------------------|
| `0` | `SCL_OK` | 成功 / Success |
| `-1` | `SCL_ERR_GENERAL` | 通用错误 / General error |
| `-2` | `SCL_ERR_INVALID_PARAM` | 无效参数 / Invalid parameter (NULL/zero) |
| `-3` | `SCL_ERR_MEMORY` | 内存分配失败 / Memory allocation failed |
| `-4` | `SCL_ERR_EXECUTION` | 执行异常 / Execution failed |
| `-5` | `SCL_ERR_IO` | 输入输出错误 / I/O error |
| `-6` | `SCL_ERR_NETWORK` | 网络通信错误 / Network error |
| `-7` | `SCL_ERR_UNSUPPORTED` | 不支持的编码类型 / Unsupported encoding |
| `-8` | `SCL_ERR_BUFFER_TOO_SMALL` | 缓冲区不足 / Buffer too small |

## 编译时控制流 / Compile-Time Control Flow

```ascii
                  ┌─────────────┐
                  │  CMakeLists  │
                  │  项目配置     │
                  └──────┬──────┘
                         │
         ┌───────────────┼───────────────┐
         ▼               ▼               ▼
   ┌──────────┐   ┌──────────┐   ┌──────────┐
   │ WITH_    │   │ ENABLE_  │   │ ENABLE_  │
   │ ENCRYPT  │   │ TESTS    │   │ EXAMPLES │
   └──────────┘   └──────────┘   └──────────┘
         │               │               │
         ▼               ▼               ▼
   ┌──────────┐   ┌──────────┐   ┌──────────┐
   │libcrypto │   │ 23 tests │   │ 3 examples│
   │(OpenSSL) │   │ 全部通过  │   │ 可独立编译│
   └──────────┘   └──────────┘   └──────────┘
```

---

## 设计决策记录 / Design Decision Records

### DDR-001：为何使用静态库而非动态库？
### DDR-001: Why static library instead of shared library?

**决策 / Decision**: 使用 `.a` 静态库分发核心功能。

**理由 / Rationale**:
1. 部署简单，无需处理动态链接依赖 / Single binary, no dependency hell
2. 体积影响小（核心库 < 50KB）/ Size impact minimal (< 50KB)
3. 适合安全工具的分发场景 / Suitable for security tool distribution

### DDR-002：为何默认不启用 OpenSSL 加密？
### DDR-002: Why is OpenSSL disabled by default?

**决策 / Decision**: `WITH_ENCRYPTION` 默认为 OFF。

**理由 / Rationale**:
1. 保持最小依赖 / Keep minimal dependencies for basic use cases
2. 大多数场景 XOR + Base64 已足够 / XOR + Base64 sufficient for most scenarios
3. 需要时可通过 `-DWITH_ENCRYPTION=ON` 启用 / Opt-in when needed

---

## 扩展指南 / Extension Guide

### 添加新的编码方式 / Adding a New Encoder

1. 在 `src/encoder/` 下创建实现文件 / Create implementation in `src/encoder/`
2. 在 `src/encoder/encoder.h` 中添加新的枚举值 / Add new enum to `encoder.h`
3. 在 `scloader.h` 中添加宏定义 / Add macro to `scloader.h`
4. 在 `scloader.h` 中声明编解码函数 / Declare encode/decode functions
5. 更新 `tests/test_encoder.c` 添加测试用例 / Add tests
6. 更新 `CMakeLists.txt` 添加源文件 / Add source file to CMake

### 添加新的加载平台 / Adding a New Platform

1. 在 `src/loader/loader.c` 中添加平台条件编译分支
2. 实现对应的 `scl_alloc_exec` 和 `scl_free_exec` 函数
3. 在 `CMakeLists.txt` 的平台检测部分添加新平台宏