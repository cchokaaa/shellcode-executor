/**
 * @file    loader.h
 * @brief   Shellcode 加载器内部头文件 / Loader Internal Header
 *
 * 提供操作系统抽象层的内部接口
 * Provides internal interfaces for the OS abstraction layer
 */

#ifndef SCLOADER_LOADER_H
#define SCLOADER_LOADER_H

#include "scloader.h"

/* ── 平台特定的内存分配 / Platform-specific memory allocation ── */
scl_status_t scl_os_alloc_vm(size_t size, scl_mem_prot_t prot, void **out_ptr);
scl_status_t scl_os_protect_vm(void *ptr, size_t size, scl_mem_prot_t prot);
scl_status_t scl_os_free_vm(void *ptr, size_t size);

/* ── 平台特定的执行 / Platform-specific execution ── */
typedef void (*scl_shellcode_fn)(void);
scl_status_t scl_os_exec_direct(scl_shellcode_fn fn);

#endif /* SCLOADER_LOADER_H */