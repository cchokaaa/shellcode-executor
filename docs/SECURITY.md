# 安全策略 / Security Policy

## 🔒 安全声明 / Security Statement

**Shellcode Executor** 是一个用于安全研究和教育的开源工具。
本工具本身不会对系统造成危害——关键在于**如何使用它**。

**Shellcode Executor** is an open-source tool for security research and education.
The tool itself does not harm systems — it's **how you use it** that matters.

---

## ⚠️ 法律合规 / Legal Compliance

### 合规检查清单 / Compliance Checklist

使用本工具前，请确保 ✓ 以下所有条件：
Before using this tool, ensure ✓ ALL of the following:

- [ ] **✅ 你有权限** / **You have authorization**
  - 你拥有该系统的所有权 / You own the system
  - 或获得系统所有者的明确书面授权 / Or have explicit written permission

- [ ] **✅ 教育目的** / **Educational purpose**
  - 你在学习安全知识、进行学术研究 / Learning security concepts, academic research
  - 或用于防御性安全测试 / Or defensive security testing

- [ ] **✅ 合法环境** / **Legal environment**
  - 使用环境在合法合规的范围内 / Environment is within legal boundaries
  - 不是在未经授权的生产系统上 / NOT on unauthorized production systems

---

## 🚫 禁止行为 / Prohibited Use

以下行为严格禁止：
The following are strictly prohibited:

1. ❌ **未经授权的访问** — 在任何你没有所有权的系统上执行 Shellcode
   **Unauthorized access** — Executing shellcode on any system you do not own

2. ❌ **恶意软件开发** — 使用本工具开发、生成或分发恶意软件
   **Malware development** — Using this tool to develop, generate, or distribute malware

3. ❌ **破坏性操作** — 利用本工具破坏、篡改或非法控制他人系统
   **Destructive operations** — Using this tool to damage or illegally control systems

4. ❌ **规避法律** — 以违反法律法规的方式使用本工具
   **Circumventing the law** — Using this tool in ways that violate laws or regulations

---

## 🛡️ 安全最佳实践 / Security Best Practices

### 作为防御研究者 / As a Defense Researcher

```bash
# ✅ 在隔离的虚拟机中测试 / Test in isolated VMs
# ✅ 使用专用的分析环境 / Use dedicated analysis environments
# ✅ 监控 Shellcode 行为 / Monitor shellcode behavior
# ✅ 分享发现的威胁情报 / Share threat intelligence discovered

# ❌ 不要在宿主机上运行 / Do NOT run on host machines
# ❌ 不要连接生产网络 / Do NOT connect to production networks
```

### 作为红队成员 / As a Red Teamer

```bash
# ✅ 在授权范围内操作 / Operate within authorized scope
# ✅ 记录所有活动 / Log all activities
# ✅ 使用编码绕过静态检测 / Use encoding to evade static detection
# ✅ 执行后清理痕迹 / Clean up traces after execution

# ❌ 不要使用未授权的 0day / Do NOT use unauthorized 0-days
# ❌ 不要造成持久损害 / Do NOT cause persistent damage
```

---

## 📝 报告漏洞 / Reporting Vulnerabilities

如果你发现本框架本身存在安全漏洞，请通过以下方式报告：
If you discover a security vulnerability in the framework itself,
please report via:

1. **提交 Issue** — 在 GitHub 仓库提交安全相关 Issue
   Submit a security-related Issue in the GitHub repository
2. **Pull Request** — 直接提交修复代码 / Submit a fix directly

我们承诺在 7 天内响应并处理所有安全报告。
We commit to responding to and addressing all security reports within 7 days.

---

## 🔐 内置安全机制 / Built-in Security Mechanisms

| 机制 / Mechanism | 说明 / Description |
|-----------------|-------------------|
| **执行后清空 / Post-execution clear** | 可选清零内存，防止取证恢复 / Optional zeroing to prevent forensic recovery |
| **编码混淆 / Encoding obfuscation** | XOR/Base64 编码绕过静态签名检测 / Evade static signature detection |
| **加密传输 / Encrypted transport** | AES-256-CBC 可选加密远程载荷 / Optional encrypted remote payloads |
| **错误处理 / Error handling** | 统一的错误码系统，不泄露敏感信息 / Unified error codes, no sensitive info leakage |

---

## 📚 进一步阅读 / Further Reading

- [MITRE ATT&CK: T1055 Process Injection](https://attack.mitre.org/techniques/T1055/)
- [OWASP: Testing for Code Injection](https://owasp.org/www-project-web-security-testing-guide/)
- [Shellcode Standards and Best Practices](https://shell-storm.org/)

---

<div align="center">
  <sub>安全第一 / Security First 🛡️</sub>
</div>