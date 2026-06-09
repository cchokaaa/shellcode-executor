/**
 * @file    memory_info.c
 * @brief   内存区域信息展示 / Memory region information display
 *
 * 显示当前进程的内存布局信息，包括页大小、内存用量等
 * Displays process memory layout info: page size, memory usage, etc.
 *
 * 跨平台支持 / Cross-platform support:
 * - Linux/macOS: sysconf() + /proc/self/status
 * - Windows:    GetSystemInfo() + GlobalMemoryStatusEx()
 */

#include "utils/utils.h"
#include <stdio.h>
#include <string.h>

/* ==================== 平台检测 / Platform Detection ==================== */
#if defined(_WIN32) || defined(_WIN64)
#  define SCL_PLATFORM_WIN 1
#else
#  include <unistd.h>  /* POSIX: sysconf() */
#endif

#if defined(SCL_PLATFORM_WIN)
#  include <windows.h>
#endif

void scl_print_memory_info(const void *addr)
{
    (void)addr; /* 保留供后续 /proc/self/maps 解析用 / Reserved for future use */

    printf("═══════════════════════════════════════════════\n");
    printf("  Shellcode Executor - 内存信息 / Memory Info\n");
    printf("═══════════════════════════════════════════════\n");

/* ---------- 页大小 + 物理内存信息 / Page size & physical memory ---------- */
#if defined(SCL_PLATFORM_WIN)
    {
        SYSTEM_INFO sys_info;
        MEMORYSTATUSEX mem_stat;

        GetSystemInfo(&sys_info);
        printf("  页大小 / Page size:        %lu bytes\n",
               sys_info.dwPageSize);

        mem_stat.dwLength = sizeof(mem_stat);
        if (GlobalMemoryStatusEx(&mem_stat)) {
            printf("  总物理内存 / Total RAM:   %llu MB\n",
                   mem_stat.ullTotalPhys / (1024 * 1024));
            printf("  可用物理内存 / Avail RAM: %llu MB\n",
                   mem_stat.ullAvailPhys / (1024 * 1024));
        }
    }
#else   /* POSIX (Linux / macOS / ...) */
    {
        long page_size = sysconf(_SC_PAGESIZE);
        if (page_size > 0)
            printf("  页大小 / Page size:        %ld bytes\n", page_size);

#if defined(_SC_PHYS_PAGES)
        long total_pages = sysconf(_SC_PHYS_PAGES);
        if (total_pages > 0) {
            printf("  总物理页 / Total pages:    %ld\n", total_pages);
            printf("  总物理内存 / Total RAM:    %.1f MB\n",
                   (double)total_pages * (double)page_size / (1024.0 * 1024.0));
        }
#endif

#if defined(_SC_AVPHYS_PAGES)
        long avail_pages = sysconf(_SC_AVPHYS_PAGES);
        if (avail_pages > 0)
            printf("  可用物理页 / Avail pages:  %ld\n", avail_pages);
#endif
    }
#endif  /* SCL_PLATFORM_WIN */

/* ---------- Linux: 从 /proc/self/status 读取进程内存用量 ---------- */
/* Linux-only: read process memory stats from /proc/self/status       */
#if defined(__linux__) || defined(__linux)
    {
        FILE *fp = fopen("/proc/self/status", "r");
        if (fp) {
            char line[256];
            while (fgets(line, sizeof(line), fp)) {
                if (strncmp(line, "VmPeak:", 7) == 0 ||
                    strncmp(line, "VmSize:", 7) == 0 ||
                    strncmp(line, "VmRSS:",  6) == 0 ||
                    strncmp(line, "VmData:", 7) == 0 ||
                    strncmp(line, "VmStk:",  6) == 0 ||
                    strncmp(line, "VmExe:",  6) == 0) {
                    /* 去掉换行符 / Remove trailing newline */
                    size_t len = strlen(line);
                    if (len > 0 && line[len-1] == '\n')
                        line[len-1] = '\0';
                    printf("  %s\n", line);
                }
            }
            fclose(fp);
        }
    }
#endif  /* __linux__ */

    printf("═══════════════════════════════════════════════\n\n");
}