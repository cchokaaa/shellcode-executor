/**
 * @file    encoder.h
 * @brief   Shellcode 编码器内部头文件 / Encoder Internal Header
 */

#ifndef SCLOADER_ENCODER_H
#define SCLOADER_ENCODER_H

#include "scloader.h"

/* XOR 编码器/解码器 / XOR encoder/decoder */
scl_status_t scl_xor_encode(const unsigned char *input, size_t input_size,
                            const unsigned char *key, size_t key_size,
                            unsigned char **output, size_t *output_size);
scl_status_t scl_xor_decode(const unsigned char *input, size_t input_size,
                            const unsigned char *key, size_t key_size,
                            unsigned char **output, size_t *output_size);

/* Base64 编码器/解码器 / Base64 encoder/decoder */
scl_status_t scl_b64_encode(const unsigned char *input, size_t input_size,
                            unsigned char **output, size_t *output_size);
scl_status_t scl_b64_decode(const unsigned char *input, size_t input_size,
                            unsigned char **output, size_t *output_size);

#endif /* SCLOADER_ENCODER_H */