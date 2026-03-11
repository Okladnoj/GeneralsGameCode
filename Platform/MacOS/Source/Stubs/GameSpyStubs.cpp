// GameSpyStubs.cpp — Linker stubs for Windows-only symbols on macOS
// Only contains stubs for symbols that have NO real implementation on macOS:
// Registry, Debug helpers, DX8 WebBrowser, DownloadManager, WorkerProcess.
//
// All GameSpy integration symbols are provided by real source files
// in Core/GameEngine/Source/GameNetwork/ (GameSpy/, LAN*, Transport, etc.)
// All GameSpy SDK C API symbols are provided by GameSpySDKStubs.c

#include "PreRTS.h"
#include <string>

#include "GameNetwork/DownloadManager.h"
#include "Common/WorkerProcess.h"
#include "DbgHelpGuard.h"
#include "registry.h"
#include "dx8webbrowser.h"

class IMEManagerInterface;

// ══════════════════════════════════════════════════════
//  NULL SINGLETONS (only those NOT defined in real code)
// ══════════════════════════════════════════════════════

DownloadManager *TheDownloadManager = nullptr;
IMEManagerInterface *TheIMEManager = nullptr;

char g_LastErrorDump[1] = {0};

IMEManagerInterface *CreateIMEManagerInterface(void) { return nullptr; }

// ══════════════════════════════════════════════════════
//  REGISTRY (std::string-based, from WWDownload)
// ══════════════════════════════════════════════════════

bool GetStringFromRegistry(std::string, std::string, std::string &) { return false; }
bool GetUnsignedIntFromRegistry(std::string, std::string, unsigned int &) { return false; }
bool SetStringInRegistry(std::string, std::string, std::string) { return false; }
bool SetUnsignedIntInRegistry(std::string, std::string, unsigned int) { return false; }

// ══════════════════════════════════════════════════════
//  STRING CONVERSION
// ══════════════════════════════════════════════════════

std::wstring MultiByteToWideCharSingleLine(const char *orig) {
  if (!orig) return L"";
  std::wstring r; while (*orig) r += (wchar_t)(unsigned char)*orig++; return r;
}
std::string WideCharStringToMultiByte(const wchar_t *orig) {
  if (!orig) return "";
  std::string r; while (*orig) r += (char)*orig++; return r;
}

// ══════════════════════════════════════════════════════
//  DEBUG STUBS
// ══════════════════════════════════════════════════════

void FillStackAddresses(void **, unsigned int, unsigned int) {}
void StackDumpFromAddresses(void **, unsigned int, void (*)(const char *)) {}
DbgHelpGuard::DbgHelpGuard() : m_needsUnload(false) {}
DbgHelpGuard::~DbgHelpGuard() {}
void DbgHelpGuard::activate() {}
void DbgHelpGuard::deactivate() {}

// ══════════════════════════════════════════════════════
//  DX8 WEB BROWSER
// ══════════════════════════════════════════════════════

bool DX8WebBrowser::Initialize(const char *, const char *, const char *, const char *) { return false; }
void DX8WebBrowser::Render(int) {}
void DX8WebBrowser::Update() {}
void DX8WebBrowser::Shutdown() {}

// ══════════════════════════════════════════════════════
//  DOWNLOAD MANAGER (WWDownload — Windows only)
// ══════════════════════════════════════════════════════

DownloadManager::DownloadManager() {}
DownloadManager::~DownloadManager() {}
HRESULT DownloadManager::update() { return S_OK; }
HRESULT DownloadManager::downloadFile(AsciiString, AsciiString, AsciiString, AsciiString, AsciiString, AsciiString, Bool) { return E_FAIL; }
HRESULT DownloadManager::downloadNextQueuedFile() { return E_FAIL; }
void DownloadManager::queueFileForDownload(AsciiString, AsciiString, AsciiString, AsciiString, AsciiString, AsciiString, Bool) {}
HRESULT DownloadManager::OnEnd() { return S_OK; }
HRESULT DownloadManager::OnError(Int) { return S_OK; }
HRESULT DownloadManager::OnProgressUpdate(Int, Int, Int, Int) { return S_OK; }
HRESULT DownloadManager::OnQueryResume() { return S_OK; }
HRESULT DownloadManager::OnStatusUpdate(Int) { return S_OK; }

// ══════════════════════════════════════════════════════
//  WORKER PROCESS (Windows CreateProcess)
// ══════════════════════════════════════════════════════

WorkerProcess::WorkerProcess() : m_isDone(false) {}
bool WorkerProcess::startProcess(UnicodeString) { return false; }
void WorkerProcess::update() {}
bool WorkerProcess::isRunning() const { return false; }
bool WorkerProcess::isDone() const { return true; }
DWORD WorkerProcess::getExitCode() const { return 0; }
AsciiString WorkerProcess::getStdOutput() const { return AsciiString::TheEmptyString; }

// ══════════════════════════════════════════════════════
//  REGISTRY CLASS (WWVegas)
// ══════════════════════════════════════════════════════

RegistryClass::RegistryClass(const char *, bool) {}
RegistryClass::~RegistryClass() {}
int RegistryClass::Get_Int(const char *, int defaultValue) { return defaultValue; }
char *RegistryClass::Get_String(const char *, char *dest, int maxLen, const char *defaultValue) {
  if (dest && maxLen > 0) {
    if (defaultValue) { strncpy(dest, defaultValue, maxLen - 1); dest[maxLen - 1] = '\0'; }
    else dest[0] = '\0';
  }
  return dest;
}
void RegistryClass::Set_Int(const char *, int) {}
void RegistryClass::Set_String(const char *, const char *) {}

// ══════════════════════════════════════════════════════
//  GAMESPY QR2 HOSTING STATUS (game-level, not SDK)
// ══════════════════════════════════════════════════════

int getQR2HostingStatus() { return 0; }
