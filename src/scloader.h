/**
 * @file    scloader.h
 * @brief   Shellcode Executor 主头文件 / Master Header
 * @author  Shellcode Executor Team
 * @license MIT
 *
 * ═══════════════════════════════════════════════════════════════
 *  Shellcode Executor 框架 / Framework
 *  跨平台 Shellcode 加载、编码与执行 / Cross-platform loading, encoding & execution
 * ═══════════════════════════════════════════════════════════════
 *
 * 架构总览 / Architecture Overview:
 * ┌─────────────────────────────────────────────────────┐
 * │                    User Application / 用户应用        │
 * ├─────────────────────────────────────────────────────┤
 * │   loader/  → 内存分配与执行 / Memory alloc & exec    │
 * │   encoder/ → 编码/解码 / Encoding/decoding (XOR/B64) │
 * │   utils/   → Hex 转储与辅助 / Hex dump & helpers     │
 * │   remote/  → 远程拉取 / Network fetch                │
 * │   crypto/  → 加密操作 / Crypto (可选/optional)       │
 * └─────────────────────────────────────────────────────┘
 */

#ifndef SCLOADER_H
#define SCLOADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* ═══════════════════════════════════════════════
 * 版本信息 / Version Information
 * ═══════════════════════════════════════════════ */
#define SCLOADER_VERSION_MAJOR 2
#define SCLOADER_VERSION_MINOR 0
#define SCLOADER_VERSION_PATCH 2
#define SCLOADER_VERSION_STRING "2.0.2"

/* ═══════════════════════════════════════════════
 * 平台检测 / Platform Detection
 * 自动识别当前编译目标平台
 * Auto-detect the target compilation platform
 * ═══════════════════════════════════════════════ */
#if defined(_WIN32) || defined(_WIN64)
    #define SCLOADER_PLATFORM_WINDOWS 1
#elif defined(__APPLE__) || defined(__MACH__)
    #define SCLOADER_PLATFORM_MACOS 1
#elif defined(__linux__) || defined(__unix__)
    #define SCLOADER_PLATFORM_LINUX 1
#else
    #error "不支持的平台 / Unsupported platform"
#endif

/* ═══════════════════════════════════════════════
 * 导出宏定义 / Export Macros
 * 用于控制 DLL/共享库符号的可见性
 * Controls DLL/shared library symbol visibility
 * ═══════════════════════════════════════════════ */
#if defined(_MSC_VER)
    #define SCL_EXPORT __declspec(dllexport)
    #define SCL_IMPORT __declspec(dllimport)
#elif defined(__GNUC__) || defined(__clang__)
    #define SCL_EXPORT __attribute__((visibility("default")))
    #define SCL_IMPORT
#else
    #define SCL_EXPORT
    #define SCL_IMPORT
#endif

#ifdef SCLOADER_BUILD
    #define SCL_API SCL_EXPORT
#else
    #define SCL_API SCL_IMPORT
#endif

/* ═══════════════════════════════════════════════
 * 内存保护标志 / Memory Protection Flags
 * 与操作系统页权限对应 (rwx)
 * Mapped to OS page permission bits (rwx)
 * ═══════════════════════════════════════════════ */
typedef enum {
    SCL_MEM_NONE      = 0,      /**< 无权限 / No access */
    SCL_MEM_READ      = 1 << 0, /**< 读权限 / Read permission */
    SCL_MEM_WRITE     = 1 << 1, /**< 写权限 / Write permission */
    SCL_MEM_EXECUTE   = 1 << 2, /**< 执行权限 / Execute permission */
    SCL_MEM_RW        = SCL_MEM_READ | SCL_MEM_WRITE,
    SCL_MEM_RWX       = SCL_MEM_READ | SCL_MEM_WRITE | SCL_MEM_EXECUTE,
    SCL_MEM_RX        = SCL_MEM_READ | SCL_MEM_EXECUTE,
} scl_mem_prot_t;

/* ═══════════════════════════════════════════════
 * 加载方式 / Loading Methods
 * 不同 shellcode 执行策略
 * Different shellcode execution strategies
 * ═══════════════════════════════════════════════ */
typedef enum {
    SCL_LOAD_DIRECT,       /**< 直接分配+拷贝+执行 / Direct alloc+copy+execute */
    SCL_LOAD_THREAD,       /**< 在新线程中执行 / Execute in a new thread */
    SCL_LOAD_FIBER,        /**< 通过 Windows Fiber 执行 (仅Win) / via Windows fibers */
    SCL_LOAD_APC,          /**< 通过 APC 注入执行 (仅Win) / via APC injection */
} scl_load_method_t;

/* ═══════════════════════════════════════════════
 * 编码类型 / Encoding Types
 * 支持的 Shellcode 编码/混淆方式
 * Supported shellcode encoding/obfuscation methods
 * ═══════════════════════════════════════════════ */
typedef enum {
    SCL_ENC_NONE   = 0,     /**< 不编码 / No encoding */
    SCL_ENC_XOR    = 1,     /**< 单字节 XOR / Single-byte XOR */
    SCL_ENC_XOR_KEY,        /**< 多字节密钥 XOR / Multi-byte key XOR */
    SCL_ENC_BASE64,         /**< Base64 编码 / Base64 encoding */
    SCL_ENC_HEX,            /**< 十六进制字符串 / Hex string encoding */
} scl_enc_type_t;

/* ═══════════════════════════════════════════════
 * 状态码 / Status & Error Codes
 * 所有 API 返回的状态/错误码
 * All API return status/error codes
 * ═══════════════════════════════════════════════ */
typedef enum {
    SCL_OK                     =  0,  /**< 成功 / Success */
    SCL_ERR_NULL_PTR           = -1,  /**< 空指针 / Null pointer */
    SCL_ERR_INVALID_SIZE       = -2,  /**< 无效大小 / Invalid size */
    SCL_ERR_ALLOC_FAILED       = -3,  /**< 内存分配失败 / Memory alloc failed */
    SCL_ERR_PROTECT_FAILED     = -4,  /**< 内存保护设置失败 / Memory protect failed */
    SCL_ERR_COPY_FAILED        = -5,  /**< 内存拷贝失败 / Memory copy failed */
    SCL_ERR_EXEC_FAILED        = -6,  /**< 执行失败 / Execution failed */
    SCL_ERR_ENCODE_FAILED      = -7,  /**< 编码失败 / Encoding failed */
    SCL_ERR_DECODE_FAILED      = -8,  /**< 解码失败 / Decoding failed */
    SCL_ERR_NETWORK_FAILED     = -9,  /**< 网络操作失败 / Network operation failed */
    SCL_ERR_UNSUPPORTED_PLATFORM = -10, /**< 不支持的平台 / Unsupported platform */
    SCL_ERR_INVALID_ARG        = -11, /**< 无效参数 / Invalid argument */
    SCL_ERR_BUFFER_TOO_SMALL   = -12, /**< 缓冲区太小 / Buffer too small */
} scl_status_t;

/* ═══════════════════════════════════════════════
 * 内存区域信息 / Memory Region Info
 * 描述一段内存区域的属性
 * Describes the attributes of a memory region
 * ═══════════════════════════════════════════════ */
typedef struct {
    void    *addr;       /**< 区域基地址 / Base address */
    size_t   size;       /**< 区域大小(字节) / Size in bytes */
    bool     is_exec;    /**< 是否可执行 / Whether executable */
    bool     is_rw;      /**< 是否可读写 / Whether readable/writable */
} scl_mem_region_t;

/* ═══════════════════════════════════════════════
 * 加载器配置 / Loader Configuration
 * 控制 shellcode 加载和执行的参数
 * Controls shellcode loading and execution parameters
 * ═══════════════════════════════════════════════ */
typedef struct {
    scl_load_method_t method;       /**< 执行方式 / Execution method */
    scl_mem_prot_t    initial_prot; /**< 初始内存保护 / Initial memory protection */
    bool              clear_after;  /**< 执行后清零内存 / Zero memory after exec */
    bool              use_jump;     /**< 使用函数指针跳转 / Use func ptr jump */
} scl_loader_config_t;

/** 默认加载器配置 / Default loader configuration */
#define SCL_LOADER_CONFIG_DEFAULT { \
    .method        = SCL_LOAD_DIRECT, \
    .initial_prot  = SCL_MEM_RWX, \
    .clear_after   = true, \
    .use_jump      = true  \
}

/* ═══════════════════════════════════════════════
 * ─── 加载器 API / Loader API ────────────────────
 * ═══════════════════════════════════════════════ */

/**
 * @brief 在当前进程中执行 shellcode / Execute shellcode in the current process
 *
 * 分配可执行内存 → 拷贝 shellcode → 执行 → 清理
 * Allocates executable memory, copies the shellcode, executes, and cleans up
 *
 * @param shellcode    Shellcode 字节指针 / Pointer to shellcode bytes
 * @param size         Shellcode 大小(字节) / Size in bytes
 * @param config       加载器配置(NULL=使用默认) / Loader config (NULL = default)
 * @return scl_status_t 状态码 / Status code
 *
 * @code
 *   unsigned char sc[] = { 0x90, 0x90, 0xCC };
 *   scl_load_and_exec(sc, sizeof(sc), NULL);
 * @endcode
 */
SCL_API scl_status_t scl_load_and_exec(const unsigned char *shellcode,
                                       size_t size,
                                       const scl_loader_config_t *config);

/**
 * @brief 为 shellcode 分配可执行内存 / Allocate executable memory
 *
 * @param size         分配大小 / Size to allocate
 * @param prot         内存保护标志 / Memory protection flags
 * @param[out] out_ptr 接收分配内存的指针 / Receives pointer to allocated memory
 * @return scl_status_t 状态码 / Status code
 */
SCL_API scl_status_t scl_alloc_exec(size_t size,
                                    scl_mem_prot_t prot,
                                    void **out_ptr);

/**
 * @brief 将 shellcode 写入预分配的内存 / Write shellcode into pre-allocated memory
 *
 * @param dst          目标缓冲区(须可执行) / Destination buffer (must be executable)
 * @param src          Shellcode 源字节 / Source shellcode bytes
 * @param size         拷贝字节数 / Number of bytes to copy
 * @return scl_status_t 状态码 / Status code
 */
SCL_API scl_status_t scl_write_shellcode(void *dst,
                                         const unsigned char *src,
                                         size_t size);

/**
 * @brief 使用函数指针执行 shellcode / Execute shellcode via function pointer
 *
 * @param ptr          可执行内存中的 shellcode 指针 / Ptr to shellcode in exec memory
 * @param size         Shellcode 大小(用于清理) / Size (for cleanup if configured)
 * @param config       加载器配置 / Loader configuration
 * @return scl_status_t 状态码 / Status code
 */
SCL_API scl_status_t scl_execute(void *ptr,
                                 size_t size,
                                 const scl_loader_config_t *config);

/**
 * @brief 释放可执行内存(可选清零) / Free executable memory (optionally clear)
 *
 * @param ptr          待释放的内存指针 / Pointer to memory to free
 * @param size         内存区域大小 / Size of memory region
 * @param clear         是否释放前清零 / Whether to zero-fill before freeing
 * @return scl_status_t 状态码 / Status code
 */
SCL_API scl_status_t scl_free_exec(void *ptr, size_t size, bool clear);

/* ═══════════════════════════════════════════════
 * ─── 编码器 API / Encoder API ───────────────────
 * ═══════════════════════════════════════════════ */

/**
 * @brief 使用指定算法编码 shellcode / Encode shellcode with specified algorithm
 *
 * @param type         编码类型 / Encoding type
 * @param input        输入数据 / Input data
 * @param input_size   输入大小 / Input size
 * @param key          编码密钥(XOR用，可为NULL) / Encoding key (for XOR; can be NULL)
 * @param key_size     密钥大小 / Key size
 * @param[out] output  输出缓冲区(内部申请，调用者须free) / Output buffer (caller must free)
 * @param[out] output_size 输出大小 / Output size
 * @return scl_status_t 状态码 / Status code
 */
SCL_API scl_status_t scl_encode(scl_enc_type_t type,
                                const unsigned char *input,
                                size_t input_size,
                                const unsigned char *key,
                                size_t key_size,
                                unsigned char **output,
                                size_t *output_size);

/**
 * @brief 将 shellcode 解码回原始字节 / Decode shellcode back to original bytes
 *
 * @param type         编码类型 / Encoding type
 * @param input        编码后的输入数据 / Encoded input data
 * @param input_size   输入大小 / Input size
 * @param key          解码密钥(XOR用) / Decoding key (for XOR)
 * @param key_size     密钥大小 / Key size
 * @param[out] output  输出缓冲区(内部申请，调用者须free) / Output buffer (caller must free)
 * @param[out] output_size 输出大小 / Output size
 * @return scl_status_t 状态码 / Status code
 */
SCL_API scl_status_t scl_decode(scl_enc_type_t type,
                                const unsigned char *input,
                                size_t input_size,
                                const unsigned char *key,
                                size_t key_size,
                                unsigned char **output,
                                size_t *output_size);

/* ═══════════════════════════════════════════════
 * ─── 工具 API / Utility API ─────────────────────
 * ═══════════════════════════════════════════════ */

/**
 * @brief 将内存数据以 hex dump 格式打印到 stdout / Print a hex dump to stdout
 *
 * @param data   数据指针 / Pointer to data
 * @param size   数据大小 / Size of data
 * @param title  可选的标题(可为NULL) / Optional title (can be NULL)
 */
SCL_API void scl_hex_dump(const unsigned char *data,
                          size_t size,
                          const char *title);

/**
 * @brief 打印内存区域信息 / Print memory region information
 *
 * @param addr  待查询地址(NULL=当前进程) / Address to query (NULL = current process)
 */
SCL_API void scl_print_memory_info(const void *addr);

/**
 * @brief 将十六进制字符串转换为原始字节 / Convert hex string to raw bytes
 *
 * @param hex          十六进制字符串(如 "9090CC") / Hex string (e.g. "9090CC")
 * @param[out] out     输出缓冲区 / Output buffer
 * @param out_size     输出缓冲区容量 / Output buffer capacity
 * @param[out] written 实际写入字节数 / Bytes written
 * @return scl_status_t 状态码 / Status code
 */
SCL_API scl_status_t scl_hex_to_bytes(const char *hex,
                                      unsigned char *out,
                                      size_t out_size,
                                      size_t *written);

/**
 * @brief 将原始字节转换为十六进制字符串 / Convert raw bytes to hex string
 *
 * @param data         原始字节 / Raw bytes
 * @param data_size    字节数 / Number of bytes
 * @param[out] out     输出字符串缓冲区 / Output string buffer
 * @param out_size     缓冲区容量(须 >= data_size*2+1) / Buffer capacity
 * @return scl_status_t 状态码 / Status code
 */
SCL_API scl_status_t scl_bytes_to_hex(const unsigned char *data,
                                      size_t data_size,
                                      char *out,
                                      size_t out_size);

/* ═══════════════════════════════════════════════
 * ─── 远程加载 API / Remote API ──────────────────
 * ═══════════════════════════════════════════════ */

/**
 * @brief 通过 HTTP GET 从远程 URL 拉取 shellcode / Fetch shellcode via HTTP GET
 *
 * @param url          待获取的 URL / URL to fetch from
 * @param[out] out     含 shellcode 的缓冲区(调用者须free) / Buffer with shellcode
 * @param[out] out_size 接收的字节数 / Number of bytes received
 * @return scl_status_t 状态码 / Status code
 */
SCL_API scl_status_t scl_remote_fetch(const char *url,
                                      unsigned char **out,
                                      size_t *out_size);

/**
 * @brief 从 TCP 服务器拉取 shellcode / Fetch shellcode from a TCP server
 *
 * @param host         主机名或IP / Hostname or IP
 * @param port         TCP 端口 / TCP port
 * @param buffer_size  接收缓冲区大小 / Receive buffer size
 * @param[out] out     含 shellcode 的缓冲区(调用者须free) / Buffer with shellcode
 * @param[out] out_size 接收的字节数 / Number of bytes received
 * @return scl_status_t 状态码 / Status code
 */
SCL_API scl_status_t scl_remote_fetch_tcp(const char *host,
                                          uint16_t port,
                                          size_t buffer_size,
                                          unsigned char **out,
                                          size_t *out_size);

/* ═══════════════════════════════════════════════
 * ─── 加密 API / Crypto API (可选,需 WITH_ENCRYPTION) ──
 * ═══════════════════════════════════════════════ */

/**
 * @brief 使用 AES-256-CBC 加密 shellcode / Encrypt shellcode using AES-256-CBC
 *
 * @param input        明文 shellcode / Plaintext shellcode
 * @param input_size   输入大小 / Input size
 * @param key          32 字节 AES 密钥 / 32-byte AES key
 * @param iv           16 字节初始向量 / 16-byte IV
 * @param[out] output  密文缓冲区(调用者须free) / Ciphertext (caller must free)
 * @param[out] output_size 密文大小 / Ciphertext size
 * @return scl_status_t 状态码 / Status code
 */
SCL_API scl_status_t scl_aes_encrypt(const unsigned char *input,
                                     size_t input_size,
                                     const unsigned char key[32],
                                     const unsigned char iv[16],
                                     unsigned char **output,
                                     size_t *output_size);

/**
 * @brief 使用 AES-256-CBC 解密 shellcode / Decrypt shellcode using AES-256-CBC
 *
 * @param input        密文 / Ciphertext
 * @param input_size   输入大小 / Input size
 * @param key          32 字节 AES 密钥 / 32-byte AES key
 * @param iv           16 字节初始向量 / 16-byte IV
 * @param[out] output  明文缓冲区(调用者须free) / Plaintext (caller must free)
 * @param[out] output_size 明文大小 / Plaintext size
 * @return scl_status_t 状态码 / Status code
 */
SCL_API scl_status_t scl_aes_decrypt(const unsigned char *input,
                                     size_t input_size,
                                     const unsigned char key[32],
                                     const unsigned char iv[16],
                                     unsigned char **output,
                                     size_t *output_size);

#ifdef __cplusplus
}
#endif

#endif /* SCLOADER_H */