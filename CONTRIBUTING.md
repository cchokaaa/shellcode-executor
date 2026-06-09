# 贡献指南 / Contributing Guide

欢迎你对 Shellcode Executor 做出贡献！无论是报告 Bug、提交新功能还是改进文档，我们都非常欢迎。
Welcome to Shellcode Executor! We appreciate contributions of all kinds —
bug reports, feature requests, documentation improvements, and code changes.

---

## 📋 准备工作 / Getting Started

### 开发环境 / Development Environment

```bash
# 安装必要工具 / Install required tools
sudo apt-get install build-essential cmake clang-format clang-tidy

# 克隆仓库 / Clone repository
git clone https://github.com/YOUR_USERNAME/shellcode-executor.git
cd shellcode-executor

# 完整构建验证 / Full build verification
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_TESTS=ON -DENABLE_EXAMPLES=ON
cmake --build build -j$(nproc)
ctest --output-on-failure
```

### 开发工作流 / Development Workflow

```bash
# 1. 创建特性分支 / Create feature branch
git checkout -b feature/your-feature-name

# 2. 编写代码并格式化 / Write code and format
make format

# 3. 运行静态分析 / Run static analysis
make analyze

# 4. 运行测试 / Run tests
make test

# 5. 提交并推送 / Commit and push
git add .
git commit -m 'feat: add your feature description'
git push origin feature/your-feature-name

# 6. 打开 Pull Request / Open a Pull Request
```

---

## 📏 编码规范 / Coding Standards

### C 语言规范 / C Language Rules

| 规范 / Rule | 要求 / Requirement |
|------------|-------------------|
| **标准 / Standard** | C11 (`-std=c11`) |
| **缩进 / Indentation** | 4 空格 / 4 spaces (不要用 Tab / No tabs) |
| **命名 / Naming** | `snake_case` 函数和变量 / functions & variables |
| **头文件 / Headers** | 函数前有 Doxygen 注释 / Doxygen comments before functions |
| **注释 / Comments** | **中英双语** / Bilingual (Chinese + English) |
| **行宽 / Line Width** | 不超过 100 列 / Maximum 100 columns |
| **错误处理 / Error Handling** | 全部使用 `scl_status_t` 返回码 |

### 注释规范 / Comment Standards

所有关键代码必须同时包含中文和英文注释：
All critical code must include both Chinese and English comments:

```c
/**
 * @brief 分配可执行内存 / Allocate executable memory
 *
 * 在目标平台上分配具有 RWX 权限的内存页。
 * Allocate a memory page with RWX permissions on the target platform.
 *
 * @param out_ptr [输出] 分配的内存指针 / [out] Allocated memory pointer
 * @param size    需要的字节数 / Required size in bytes
 * @return SCL_OK 成功 / Success, 或错误码 / or error code
 */
scl_status_t scl_alloc_exec(void **out_ptr, size_t size);
```

### 错误处理规范 / Error Handling Standards

```c
// ✅ 正确 / Correct: 检查每个可能的错误 / Check every possible error
scl_status_t s = scl_alloc_exec(&mem, size);
if (s != SCL_OK) {
    // 打印中英双语错误信息 / Print bilingual error message
    fprintf(stderr, "[!] 分配失败 / Allocation failed: %d\n", s);
    return s;
}

// ❌ 错误 / Wrong: 忽略错误码 / Ignoring error codes
scl_alloc_exec(&mem, size);  // 忽略返回值 / Ignoring return value!
```

---

## 🧪 测试规范 / Testing Standards

### 添加新的测试 / Adding New Tests

1. 在 `tests/` 目录下创建 `test_<module>.c`
2. 按照现有测试的格式编写测试用例
3. 在 `tests/CMakeLists.txt` 中添加新的测试目标
4. 运行 `make test` 确保全部通过

### 测试覆盖要求 / Coverage Requirements

```c
// 每个测试函数应该覆盖 / Each test should cover:
// ✅ 正常路径 / Happy path
// ✅ 边界条件 / Edge cases (empty input, zero size)
// ✅ 错误路径 / Error paths (NULL params, allocation failures)
// ✅ 往返一致性 / Round-trip consistency (encode → decode = original)
```

---

## 📝 提交信息规范 / Commit Message Convention

我们使用 [Conventional Commits](https://www.conventionalcommits.org/) 规范：
We follow the Conventional Commits specification:

```
<type>(<scope>): <description>

[optional body]
```

### 类型 / Types

| 类型 / Type | 说明 / Description |
|------------|-------------------|
| `feat` | 新功能 / New feature |
| `fix` | Bug 修复 / Bug fix |
| `docs` | 文档变更 / Documentation changes |
| `style` | 代码风格（不影响功能）/ Code style (no functional changes) |
| `refactor` | 重构 / Code refactoring |
| `test` | 测试相关 / Testing related |
| `chore` | 构建/工具链 / Build/tooling |

### 示例 / Examples

```
feat(loader): 添加 macOS MAP_JIT 支持 / Add macOS MAP_JIT support
fix(encoder): 修复 XOR 空密钥崩溃 / Fix XOR crash on empty key
docs(readme): 更新中英双语 README / Update bilingual README
test(loader): 添加 macOS 平台测试 / Add macOS platform tests
```

---

## 🔄 Pull Request 流程 / PR Process

1. **描述变更 / Describe Changes**: 清晰说明修改内容和原因
2. **关联 Issue / Link Issues**: 如有对应 Issue 请关联
3. **通过 CI / Pass CI**: 确保 GitHub Actions 全部通过
4. **代码审查 / Code Review**: 至少 1 人审查后方可合并
5. **合并策略 / Merge Strategy**: 使用 Squash merge 保持历史整洁

---

## 🚀 发布流程 / Release Process

```bash
# 1. 更新版本号 / Update version
#    修改 CMakeLists.txt 中的 VERSION

# 2. 更新 CHANGELOG / Update changelog

# 3. 创建标签 / Create tag
git tag v2.0.2
git push origin v2.0.2

# 4. GitHub Actions 自动构建 Release
#    GitHub Actions builds the release automatically
```

---

## 📚 学习资源 / Learning Resources

- [C11 Standard (ISO/IEC 9899:2011)](https://en.wikipedia.org/wiki/C11_(C_standard_revision))
- [CMake Documentation](https://cmake.org/documentation/)
- [Shellcode 开发指南 / Shellcode Development Guide](https://shell-storm.org/shellcode/)
- [System V ABI - AMD64 调用约定 / Calling Convention](https://wiki.osdev.org/System_V_ABI)

---

<div align="center">
  <sub>感谢你的贡献 / Thank you for contributing! 🎉</sub>
</div>