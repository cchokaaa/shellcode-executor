/**
 * @file    crypto.c
 * @brief   Shellcode AES-256-CBC 加解密实现 / AES-256-CBC crypto implementation
 *
 * 需要 WITH_ENCRYPTION=1 和 OpenSSL 开发库。
 * 当加密功能关闭时，这些函数返回 SCL_ERR_UNSUPPORTED_PLATFORM。
 * Requires WITH_ENCRYPTION=1 and OpenSSL dev libraries.
 * When disabled, returns SCL_ERR_UNSUPPORTED_PLATFORM.
 */

#include "crypto/crypto.h"
#include <stdlib.h>
#include <string.h>

#ifdef WITH_ENCRYPTION
    #include <openssl/evp.h>
    #include <openssl/rand.h>

scl_status_t scl_aes_encrypt(const unsigned char *input, size_t input_size,
                              const unsigned char key[32],
                              const unsigned char iv[16],
                              unsigned char **output, size_t *output_size)
{
    if (!input || !output || !output_size || !key || !iv) {
        return SCL_ERR_INVALID_ARG;
    }

    /* 计算 PKCS7 填充后的密文长度 */
    /* Calculate ciphertext length with PKCS7 padding */
    size_t ciph_len = input_size + 16; /* AES 块大小 / AES block size */
    unsigned char *ciphertext = (unsigned char *)malloc(ciph_len);
    if (!ciphertext) {
        return SCL_ERR_ALLOC_FAILED;
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        free(ciphertext);
        return SCL_ERR_ALLOC_FAILED;
    }

    int len = 0;
    int ciphertext_len = 0;

    /* 初始化 AES-256-CBC 加密 / Init AES-256-CBC encryption */
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(ciphertext);
        return SCL_ERR_ENCODE_FAILED;
    }

    /* 加密数据 / Encrypt data */
    if (EVP_EncryptUpdate(ctx, ciphertext, &len, input, (int)input_size) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(ciphertext);
        return SCL_ERR_ENCODE_FAILED;
    }
    ciphertext_len = len;

    /* 完成加密（处理填充） / Finalize encryption (handle padding) */
    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(ciphertext);
        return SCL_ERR_ENCODE_FAILED;
    }
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    /* 裁剪到实际大小 / Trim to actual size */
    unsigned char *result = (unsigned char *)realloc(ciphertext, (size_t)ciphertext_len);
    if (!result) {
        *output = ciphertext;
    } else {
        *output = result;
    }
    *output_size = (size_t)ciphertext_len;

    return SCL_OK;
}

scl_status_t scl_aes_decrypt(const unsigned char *input, size_t input_size,
                              const unsigned char key[32],
                              const unsigned char iv[16],
                              unsigned char **output, size_t *output_size)
{
    if (!input || !output || !output_size || !key || !iv) {
        return SCL_ERR_INVALID_ARG;
    }

    unsigned char *plaintext = (unsigned char *)malloc(input_size);
    if (!plaintext) {
        return SCL_ERR_ALLOC_FAILED;
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        free(plaintext);
        return SCL_ERR_ALLOC_FAILED;
    }

    int len = 0;
    int plaintext_len = 0;

    /* 初始化 AES-256-CBC 解密 / Init AES-256-CBC decryption */
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(plaintext);
        return SCL_ERR_DECODE_FAILED;
    }

    /* 解密数据 / Decrypt data */
    if (EVP_DecryptUpdate(ctx, plaintext, &len, input, (int)input_size) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(plaintext);
        return SCL_ERR_DECODE_FAILED;
    }
    plaintext_len = len;

    /* 完成解密 / Finalize decryption */
    if (EVP_DecryptFinal_ex(ctx, plaintext + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(plaintext);
        return SCL_ERR_DECODE_FAILED;
    }
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    unsigned char *result = (unsigned char *)realloc(plaintext, (size_t)plaintext_len);
    if (!result) {
        *output = plaintext;
    } else {
        *output = result;
    }
    *output_size = (size_t)plaintext_len;

    return SCL_OK;
}

#else /* 未启用加密 / Encryption disabled */

scl_status_t scl_aes_encrypt(const unsigned char *input, size_t input_size,
                              const unsigned char key[32],
                              const unsigned char iv[16],
                              unsigned char **output, size_t *output_size)
{
    (void)input;
    (void)input_size;
    (void)key;
    (void)iv;
    (void)output;
    (void)output_size;
    return SCL_ERR_UNSUPPORTED_PLATFORM;
}

scl_status_t scl_aes_decrypt(const unsigned char *input, size_t input_size,
                              const unsigned char key[32],
                              const unsigned char iv[16],
                              unsigned char **output, size_t *output_size)
{
    (void)input;
    (void)input_size;
    (void)key;
    (void)iv;
    (void)output;
    (void)output_size;
    return SCL_ERR_UNSUPPORTED_PLATFORM;
}

#endif /* WITH_ENCRYPTION */