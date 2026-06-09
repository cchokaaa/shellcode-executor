/**
 * @file    loader.c
 * @brief   跨平台 Shellcode 加载器实现 / Cross-platform shellcode loader implementation
 *
 * 支持 Windows (VirtualAlloc/VirtualProtect) 和 POSIX (mmap/mprotect) 平台，
 * 提供统一的 API 接口。
 * Supports both Windows and POSIX platforms with a unified API.
 *
 * 执行流程 / Execution Flow:
 *   1. 分配具有适当权限的内存 / Allocate memory with permissions
 *   2. 将 shellcode 字节拷贝到内存中 / Copy shellcode bytes into memory
 *   3. 调整内存保护属性(如需要) / Adjust protections if needed
 *   4. 通过函数指针调用执行 / Execute via function pointer call
 *   5. (可选)执行后清零并释放 / (Optional) Clear and release after execution
 */

#include "loader/loader.h"
#include <string.h>
#include <stdio.h>

/* ═══════════════════════════════════════════════
 * Windows 平台实现 / Platform: Windows
 * 使用 VirtualAlloc / VirtualProtect / VirtualFree
 * ═══════════════════════════════════════════════ */
#if SCLOADER_PLATFORM_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>

scl_status_t scl_os_alloc_vm(size_t size, scl_mem_prot_t prot, void **out_ptr)
{
    DWORD win_prot = 0;

    /* 将 scl_mem_prot_t 转换为 Windows 内存保护常量 */
    if (prot & SCL_MEM_EXECUTE) {
        win_prot = (prot & SCL_MEM_WRITE) ? PAGE_EXECUTE_READWRITE
                                          : PAGE_EXECUTE_READ;
    } else if (prot & SCL_MEM_READ) {
        win_prot = (prot & SCL_MEM_WRITE) ? PAGE_READWRITE
                                          : PAGE_READONLY;
    } else {
        win_prot = PAGE_NOACCESS;
    }

    void *ptr = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, win_prot);
    if (!ptr) {
        return SCL_ERR_ALLOC_FAILED;
    }

    *out_ptr = ptr;
    return SCL_OK;
}

scl_status_t scl_os_protect_vm(void *ptr, size_t size, scl_mem_prot_t prot)
{
    DWORD win_prot = 0;

    if (prot & SCL_MEM_EXECUTE) {
        win_prot = (prot & SCL_MEM_WRITE) ? PAGE_EXECUTE_READWRITE
                                          : PAGE_EXECUTE_READ;
    } else if (prot & SCL_MEM_READ) {
        win_prot = (prot & SCL_MEM_WRITE) ? PAGE_READWRITE : PAGE_READONLY;
    } else {
        win_prot = PAGE_NOACCESS;
    }

    DWORD old_prot;
    if (!VirtualProtect(ptr, size, win_prot, &old_prot)) {
        return SCL_ERR_PROTECT_FAILED;
    }

    return SCL_OK;
}

scl_status_t scl_os_free_vm(void *ptr, size_t size)
{
    (void)size; /* Windows 释放整个分配 / Windows frees the entire allocation */
    if (!VirtualFree(ptr, 0, MEM_RELEASE)) {
        return SCL_ERR_ALLOC_FAILED;
    }
    return SCL_OK;
}

scl_status_t scl_os_exec_direct(scl_shellcode_fn fn)
{
    fn();
    return SCL_OK;
}

/* ═══════════════════════════════════════════════
 * POSIX 平台实现 / Platform: POSIX (Linux / macOS)
 * 使用 mmap / mprotect / munmap
 * ═══════════════════════════════════════════════ */
#elif SCLOADER_PLATFORM_LINUX || SCLOADER_PLATFORM_MACOS
    #include <sys/mman.h>
    #include <unistd.h>
    #include <errno.h>

/**
 * 将 scl_mem_prot_t 标志转换为 POSIX 保护标志
 * Convert scl_mem_prot_t flags to POSIX protection flags
 */
static int prot_to_posix(scl_mem_prot_t prot)
{
    int posix_prot = 0;

    if (prot & SCL_MEM_READ)   posix_prot |= PROT_READ;
    if (prot & SCL_MEM_WRITE)  posix_prot |= PROT_WRITE;
    if (prot & SCL_MEM_EXECUTE) posix_prot |= PROT_EXEC;

    return posix_prot;
}

scl_status_t scl_os_alloc_vm(size_t size, scl_mem_prot_t prot, void **out_ptr)
{
    int posix_prot = prot_to_posix(prot);
    int flags = MAP_PRIVATE | MAP_ANONYMOUS;

#if SCLOADER_PLATFORM_MACOS
    /* macOS 上没有 JIT entitlement 时 MAP_JIT 会失败 */
    /* 所以我们先分配 RW（不加 EXEC），之后在 scl_execute 中用 mprotect 加上 EXEC */
    /* MAP_JIT requires JIT entitlement; without it, fall back to RW + mprotect later */
    posix_prot &= ~PROT_EXEC;
#endif

    void *ptr = mmap(NULL, size, posix_prot, flags, -1, 0);
    if (ptr == MAP_FAILED) {
        return SCL_ERR_ALLOC_FAILED;
    }

    *out_ptr = ptr;
    return SCL_OK;
}

scl_status_t scl_os_protect_vm(void *ptr, size_t size, scl_mem_prot_t prot)
{
    int posix_prot = prot_to_posix(prot);

    if (mprotect(ptr, size, posix_prot) != 0) {
        return SCL_ERR_PROTECT_FAILED;
    }

    return SCL_OK;
}

scl_status_t scl_os_free_vm(void *ptr, size_t size)
{
    if (munmap(ptr, size) != 0) {
        return SCL_ERR_ALLOC_FAILED;
    }
    return SCL_OK;
}

scl_status_t scl_os_exec_direct(scl_shellcode_fn fn)
{
    fn();
    return SCL_OK;
}

#else
    #error "不支持的平台 / Unsupported platform"
#endif /* 平台选择结束 / End of platform selection */

/* ═══════════════════════════════════════════════
 * 公共 API 实现 / Public API Implementation
 * ═══════════════════════════════════════════════ */

scl_status_t scl_alloc_exec(size_t size, scl_mem_prot_t prot, void **out_ptr)
{
    if (!out_ptr || size == 0) {
        return SCL_ERR_INVALID_ARG;
    }

    /* 确保包含执行权限 / Ensure executable permission is set */
    if (!(prot & SCL_MEM_EXECUTE)) {
        prot |= SCL_MEM_EXECUTE;
    }

    return scl_os_alloc_vm(size, prot, out_ptr);
}

scl_status_t scl_write_shellcode(void *dst, const unsigned char *src, size_t size)
{
    if (!dst || !src || size == 0) {
        return SCL_ERR_INVALID_ARG;
    }

    memcpy(dst, src, size);
    return SCL_OK;
}

scl_status_t scl_execute(void *ptr, size_t size, const scl_loader_config_t *config)
{
    if (!ptr || size == 0) {
        return SCL_ERR_INVALID_ARG;
    }

    scl_loader_config_t default_config = SCL_LOADER_CONFIG_DEFAULT;
    if (!config) {
        config = &default_config;
    }

    /* 确保内存可执行 / Ensure memory is executable */
    scl_status_t status;
    status = scl_os_protect_vm(ptr, size, SCL_MEM_RX);
    if (status != SCL_OK) {
        return status;
    }

    /* 通过 memcpy 将指针转为函数指针(符合 C 标准) */
    /* Use memcpy to convert ptr to function pointer (C standard compliant) */
    scl_shellcode_fn fn;
    memcpy(&fn, &ptr, sizeof(fn));

#if defined(__GNUC__) || defined(__clang__)
    /* 防止编译器优化移除调用 / Prevent compiler from optimizing away the call */
    __asm__ __volatile__("" : : "r"(fn) : "memory");
#endif

    status = scl_os_exec_direct(fn);

    /* 执行后可根据配置清理内存 / Memory cleanup after execution */
    if (config->clear_after) {
        /* 先改回可写权限，再清零 */
        /* Make memory writable before clearing */
        scl_os_protect_vm(ptr, size, SCL_MEM_RW);
        memset(ptr, 0, size);
    }

    return status;
}

scl_status_t scl_free_exec(void *ptr, size_t size, bool clear)
{
    if (!ptr) {
        return SCL_ERR_INVALID_ARG;
    }

    if (clear) {
        /* 确保可写后再清理 / Ensure writable before clearing */
        scl_os_protect_vm(ptr, size, SCL_MEM_RW);
        memset(ptr, 0, size);
    }

    return scl_os_free_vm(ptr, size);
}

scl_status_t scl_load_and_exec(const unsigned char *shellcode,
                                size_t size,
                                const scl_loader_config_t *config)
{
    if (!shellcode || size == 0) {
        return SCL_ERR_INVALID_ARG;
    }

    scl_loader_config_t default_config = SCL_LOADER_CONFIG_DEFAULT;
    if (!config) {
        config = &default_config;
    }

    /* 步骤1: 分配可执行内存 / Step 1: Allocate executable memory */
    void *exec_mem = NULL;
    scl_status_t status;

    status = scl_alloc_exec(size, config->initial_prot, &exec_mem);
    if (status != SCL_OK) {
        return status;
    }

    /* 步骤2: 拷贝 shellcode / Step 2: Copy shellcode */
    status = scl_write_shellcode(exec_mem, shellcode, size);
    if (status != SCL_OK) {
        scl_os_free_vm(exec_mem, size);
        return status;
    }

    /* 步骤3: 执行 / Step 3: Execute */
    status = scl_execute(exec_mem, size, config);

    /* 步骤4: 清理 / Step 4: Cleanup */
    scl_status_t free_status = scl_free_exec(exec_mem, size, config->clear_after);

    return (status == SCL_OK) ? free_status : status;
}