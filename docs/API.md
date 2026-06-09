# API 参考 / API Reference

本文档详细描述了 `libscloader` 库的所有公开 API。
This document details all public APIs of the `libscloader` library.

---

## 📋 类型定义 / Type Definitions

### `scl_status_t` — 状态码 / Status Codes

```c
typedef enum {
    SCL_OK                 =  0,  // 成功 / Success
    SCL_ERR_GENERAL        = -1,  // 通用错误 / General error
    SCL_ERR_INVALID_PARAM  = -2,  // 无效参数 / Invalid parameter
    SCL_ERR_MEMORY         = -3,  // 内存分配失败 / Memory allocation failed
    SCL_ERR_EXECUTION      = -4,  // 执行异常 / Execution failed
    SCL_ERR_IO             = -5,  // IO 错误 / I/O error
    SCL_ERR_NETWORK        = -6,  // 网络错误 / Network error
    SCL_ERR_UNSUPPORTED    = -7,  // 不支持的编码类型 / Unsupported encoding
    SCL_ERR_BUFFER_TOO_SMALL = -8, // 缓冲区太小 / Buffer too small
} scl_status_t;
```

### `scl_enc_type_t` — 编码类型 / Encoding Type

```c
typedef enum {
    SCL_ENC_XOR_KEY = 1,   // XOR 密钥编码 / XOR key encoding
    SCL_ENC_BASE64  = 2,   // Base64 编码 / Base64 encoding
    SCL_ENC_HEX     = 3,   // 十六进制编码 / Hex encoding
} scl_enc_type_t;
```

### `scl_loader_config_t` — 加载器配置 / Loader Configuration

```c
typedef struct {
    void  *reserved;     // 保留字段(未来使用) / Reserved for future use
    size_t heap_size;    // 堆大小(0=默认) / Heap size (0 = default)
    bool   clear_after;  // 执行后是否清空内存 / Clear memory after execution
    int    timeout_ms;   // 远程获取超时(毫秒) / Remote fetch timeout (ms)
} scl_loader_config_t;

#define SCL_LOADER_CONFIG_DEFAULT { NULL, 0, true, 10000 }
```

---

## 🚀 加载器 API / Loader API

### `scl_alloc_exec`

```c
scl_status_t scl_alloc_exec(void **out_ptr, size_t size);
```

**中文**:
分配一块具有读、写、执行权限的内存页。这是执行 Shellcode 的第一步。

**English**:
Allocate a memory page with read, write, and execute permissions.
This is the first step in executing shellcode.

| 参数 / Param | 说明 / Description |
|-------------|-------------------|
| `out_ptr` | [输出] 指向分配内存的指针 / [out] Pointer to allocated memory |
| `size` | 需要的字节数 / Required size in bytes |

| 返回值 / Return | 说明 / Description |
|----------------|-------------------|
| `SCL_OK` | 分配成功 / Success |
| `SCL_ERR_INVALID_PARAM` | `out_ptr` 为 NULL 或 `size` 为 0 |
| `SCL_ERR_MEMORY` | 系统内存分配失败 / System allocation failed |

**示例 / Example**:
```c
void *mem = NULL;
scl_status_t s = scl_alloc_exec(&mem, 4096);
if (s == SCL_OK) {
    // 使用内存 / Use the memory
}
```

---

### `scl_write_shellcode`

```c
scl_status_t scl_write_shellcode(void *dst, const unsigned char *src, size_t size);
```

**中文**:
将 Shellcode 字节拷贝到已分配的可执行内存中。

**English**:
Copy shellcode bytes into previously allocated executable memory.

| 参数 / Param | 说明 / Description |
|-------------|-------------------|
| `dst` | 目标内存地址(由 `scl_alloc_exec` 分配) |
| `src` | 源 Shellcode 数据 / Source shellcode data |
| `size` | 要写入的字节数 / Number of bytes to write |

| 返回值 / Return | 说明 / Description |
|----------------|-------------------|
| `SCL_OK` | 写入成功 / Success |
| `SCL_ERR_INVALID_PARAM` | 任一参数为 NULL |

---

### `scl_exec_shellcode`

```c
scl_status_t scl_exec_shellcode(const void *code);
```

**中文**:
跳转到指定内存地址并开始执行 Shellcode。

**English**:
Jump to the specified memory address and begin executing shellcode.

| 参数 / Param | 说明 / Description |
|-------------|-------------------|
| `code` | 指向可执行内存的指针 / Pointer to executable memory |

| 返回值 / Return | 说明 / Description |
|----------------|-------------------|
| `SCL_OK` | Shellcode 执行完毕并正常返回 / Shellcode completed and returned |
| `SCL_ERR_INVALID_PARAM` | `code` 为 NULL |
| `SCL_ERR_EXECUTION` | 执行异常 / Execution failed |

> ⚠️ **注意 / Note**: 如果 Shellcode 没有正常返回（如执行了 `exit()`），此函数不会返回。
> If the shellcode does not return normally (e.g., calls `exit()`), this function will not return.

---

### `scl_free_exec`

```c
scl_status_t scl_free_exec(void *ptr, size_t size);
```

**中文**:
释放由 `scl_alloc_exec` 分配的可执行内存。如果 `clear_after` 为 true，释放前会先用 0 填充。

**English**:
Free executable memory allocated by `scl_alloc_exec`. If `clear_after` is
enabled in config, the memory is zeroed before release.

| 参数 / Param | 说明 / Description |
|-------------|-------------------|
| `ptr` | 要释放的内存指针 / Memory pointer to free |
| `size` | 内存大小 / Memory size |

| 返回值 / Return | 说明 / Description |
|----------------|-------------------|
| `SCL_OK` | 释放成功 / Success |
| `SCL_ERR_INVALID_PARAM` | `ptr` 为 NULL（安全处理）|

---

### `scl_load_and_exec`

```c
scl_status_t scl_load_and_exec(const unsigned char *shellcode, size_t size,
                                const scl_loader_config_t *config);
```

**中文**:
一站式函数：分配内存 → 写入数据 → 执行 → 释放。覆盖了完整的 Shellcode 加载执行生命周期。

**English**:
All-in-one function: allocate → write → execute → free.
Covers the complete shellcode loading and execution lifecycle.

| 参数 / Param | 说明 / Description |
|-------------|-------------------|
| `shellcode` | Shellcode 数据 / Shellcode data |
| `size` | Shellcode 大小 / Shellcode size |
| `config` | 配置（传 NULL 使用默认值）/ Config (NULL = default) |

| 返回值 / Return | 说明 / Description |
|----------------|-------------------|
| `SCL_OK` | 执行成功 / Success |
| 其他 / Other | 参见各子函数返回值 / See sub-function return values |

**示例 / Example**:
```c
unsigned char shellcode[] = { 0xCC, 0xC3 };
scl_loader_config_t config = SCL_LOADER_CONFIG_DEFAULT;
config.clear_after = false;

scl_status_t s = scl_load_and_exec(shellcode, sizeof(shellcode), &config);
if (s == SCL_OK) {
    printf("Shellcode 执行成功 / Executed successfully!\n");
}
```

---

## 🔐 编码器 API / Encoder API

### `scl_encode`

```c
scl_status_t scl_encode(scl_enc_type_t type,
                         const unsigned char *data, size_t data_size,
                         const unsigned char *key, size_t key_size,
                         unsigned char **out, size_t *out_size);
```

**中文**:
对数据进行编码（XOR / Base64 / Hex）。编码结果通过 `out` 指针分配并返回（调用者负责 `free`）。

**English**:
Encode data using the specified method (XOR / Base64 / Hex).
The encoded output is allocated and returned via `out` (caller must `free`).

| 参数 / Param | 说明 / Description |
|-------------|-------------------|
| `type` | 编码类型 / Encoding type (`SCL_ENC_XOR_KEY`, `SCL_ENC_BASE64`, `SCL_ENC_HEX`) |
| `data` | 原始输入数据 / Raw input data |
| `data_size` | 输入数据大小 / Input data size |
| `key` | 密钥（仅 XOR）/ Key (XOR only, NULL=默认 / default) |
| `key_size` | 密钥大小 / Key size (XOR only, 0=默认 / default) |
| `out` | [输出] 编码后的数据 / [out] Encoded output |
| `out_size` | [输出] 输出数据大小 / [out] Output size |

| 返回值 / Return | 说明 / Description |
|----------------|-------------------|
| `SCL_OK` | 编码成功 / Success |
| `SCL_ERR_INVALID_PARAM` | 无效参数 / Invalid params |
| `SCL_ERR_UNSUPPORTED` | 不支持的编码类型 / Unsupported type |

---

### `scl_decode`

```c
scl_status_t scl_decode(scl_enc_type_t type,
                         const unsigned char *data, size_t data_size,
                         const unsigned char *key, size_t key_size,
                         unsigned char **out, size_t *out_size);
```

**中文**:
解码已编码的数据（XOR / Base64 / Hex）。输出通过 `out` 分配并返回。

**English**:
Decode previously encoded data (XOR / Base64 / Hex).
Output is allocated and returned via `out`.

参数与 `scl_encode` 相同 / Parameters are identical to `scl_encode`。

---

## 🌐 远程加载 API / Remote Fetch API

### `scl_remote_fetch`

```c
scl_status_t scl_remote_fetch(const char *url,
                               unsigned char **out, size_t *out_size);
```

**中文**:
从指定的 URL（HTTP GET）获取 Shellcode 数据。

**English**:
Fetch shellcode data from a specified URL via HTTP GET.

| 参数 / Param | 说明 / Description |
|-------------|-------------------|
| `url` | 完整的 HTTP URL / Full HTTP URL (e.g., `http://example.com/payload.bin`) |
| `out` | [输出] 获取的数据 / [out] Fetched data |
| `out_size` | [输出] 数据大小 / [out] Data size |

| 返回值 / Return | 说明 / Description |
|----------------|-------------------|
| `SCL_OK` | 获取成功 / Success |
| `SCL_ERR_INVALID_PARAM` | 无效参数 / Invalid params |
| `SCL_ERR_NETWORK` | 网络连接失败 / Network connection failed |

---

## 🛠️ 工具函数 API / Utility API

### `scl_hex_to_bytes`

```c
scl_status_t scl_hex_to_bytes(const char *hex, unsigned char *out,
                               size_t out_capacity, size_t *written);
```

**中文**:
将十六进制字符串转换为字节数组。支持空格分隔。
例如: `"CC AB 90"` → `{0xCC, 0xAB, 0x90}`

**English**:
Convert a hex string to a byte array. Supports space separation.
Example: `"CC AB 90"` → `{0xCC, 0xAB, 0x90}`

| 参数 / Param | 说明 / Description |
|-------------|-------------------|
| `hex` | 十六进制输入字符串 / Hex input string |
| `out` | 输出缓冲区 / Output buffer |
| `out_capacity` | 输出缓冲区容量 / Output buffer capacity |
| `written` | [输出] 实际写入的字节数 / [out] Bytes actually written |

| 返回值 / Return | 说明 / Description |
|----------------|-------------------|
| `SCL_OK` | 转换成功 / Success |
| `SCL_ERR_INVALID_PARAM` | 包含非法字符或奇数长度 / Invalid chars or odd length |
| `SCL_ERR_BUFFER_TOO_SMALL` | 输出缓冲区不足 / Output buffer insufficient |

---

### `scl_bytes_to_hex`

```c
scl_status_t scl_bytes_to_hex(const unsigned char *bytes, size_t size,
                               char *out, size_t out_capacity, size_t *written);
```

**中文**:
将字节数组转换为大写十六进制字符串。
例如: `{0xCC, 0xAB}` → `"CCAB"`

**English**:
Convert a byte array to an uppercase hex string.
Example: `{0xCC, 0xAB}` → `"CCAB"`

---

### `scl_hex_dump`

```c
void scl_hex_dump(const void *data, size_t size, const char *label);
```

**中文**:
以经典的 Hex + ASCII 格式打印内存内容。用于调试和可视化。

**English**:
Print memory content in classic Hex + ASCII format. Useful for debugging.

**示例输出 / Example Output**:
```
═══ [shellcode] ═══
0x00000000  31 C0 50 68 2F 2F 73 68  68 2F 62 69 6E 89 E3 50  1Ph//shh/binP
0x00000010  53 89 E1 99 B0 0B CD 80                           S̀
```

---

### `scl_memory_info`

```c
void scl_memory_info(void);
```

**中文**:
打印当前进程的内存映射信息（在 Linux 上解析 `/proc/self/maps`）。

**English**:
Print the current process's memory mapping information
(parses `/proc/self/maps` on Linux).

**示例输出 / Example Output**:
```
╔══════════════════════════════════════════════╗
║       进程内存信息 / Process Memory Info     ║
╚══════════════════════════════════════════════╝
PID: 12345
Total VM: 2.5 GB
Stack: 132 KB
Heap: 64 KB
─────────────────────────────────────────────
Address          Perms  Offset    Path
0x400000-0x401000 r-xp   0x0000    /usr/bin/...
0x401000-0x402000 r--p   0x0000    /usr/bin/...
```

---

## 📊 函数调用关系图 / Function Call Graph

```ascii
scl_load_and_exec()              ← 一站式入口 / All-in-one entry
├── scl_alloc_exec()             ← 分配可执行内存 / Alloc RWX memory
├── scl_write_shellcode()        ← 写入数据 / Write data
├── scl_exec_shellcode()         ← 跳转执行 / Jump and execute
└── scl_free_exec()              ← 释放内存 / Free memory

scl_encode()                     ← 编码入口 / Encoding entry
├── scl_xor_encode()             ← XOR 编码
└── scl_base64_encode()          ← Base64 编码

scl_decode()                     ← 解码入口 / Decoding entry
├── scl_xor_decode()             ← XOR 解码
└── scl_base64_decode()          ← Base64 解码

scl_remote_fetch()               ← 远程获取 / Remote fetch
├── http_get()                   ← HTTP GET 请求
└── tcp_recv()                   ← 原始 TCP 接收
```