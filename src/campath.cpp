#include <windows.h>
#include <shlobj.h>

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

#include "includes/htmodloader.h"
#include "custompath/campath.h"

extern "C" HMODULE hModuleDll;

namespace campath {
namespace {

using u8  = std::uint8_t;
using u32 = std::uint32_t;

constexpr u32 RVA_PATH_FUNC = 0x1EC82C0;
constexpr u32 RVA_MGR_GLOBAL = 0x47265B0;

constexpr const char* TARGET_FORMAT_MARKER = "ThatGameCompany";
constexpr const char* SKY_SUBDIR           = "Sky";
constexpr const char* FALLBACK_ENV_VAR     = "USERPROFILE";
constexpr const char* FALLBACK_PATH        = "C:\\Users\\Public\\Sky";

constexpr std::size_t PATH_BUF_SIZE = 0x800;
constexpr std::size_t PATH_OFFSETS[] = { 0x1E6E, 0x266E };

using PathFunc = void* (*)(void*, void*);

u8*           gBase    = nullptr;
HTAsmFunction gHook{};
PathFunc      gOrig    = nullptr;
std::string   gCustomPath;
bool          gComInit = false;
volatile long gPatched = 0;

void ensureCom() noexcept {
  if (gComInit) return;
  if (SUCCEEDED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
    gComInit = true;
}

void resolveCustomPath() noexcept {
  if (!gCustomPath.empty()) return;
  ensureCom();

  WCHAR* pics = nullptr;
  if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Pictures, 0, nullptr, &pics)) && pics) {
    int len = WideCharToMultiByte(CP_UTF8, 0, pics, -1, nullptr, 0, nullptr, nullptr);
    if (len > 0) {
      gCustomPath.resize((std::size_t)len - 1);
      WideCharToMultiByte(CP_UTF8, 0, pics, -1, &gCustomPath[0], len, nullptr, nullptr);
      gCustomPath += '\\';
      gCustomPath += SKY_SUBDIR;
    }
    CoTaskMemFree(pics);
  }

  if (gCustomPath.empty()) {
    char env[512]{};
    if (GetEnvironmentVariableA(FALLBACK_ENV_VAR, env, sizeof(env)) > 0)
      gCustomPath = std::string(env) + "\\Pictures\\Sky";
  }

  if (gCustomPath.empty())
    gCustomPath = FALLBACK_PATH;
}

void patchBuffer(char* buf, std::size_t size) noexcept {
  char* marker = std::strstr(buf, TARGET_FORMAT_MARKER);
  if (!marker) return;

  const char* tail = marker + std::strlen(TARGET_FORMAT_MARKER);
  while (*tail == '\\' || *tail == '/') ++tail;

  char newPath[PATH_BUF_SIZE];
  int n = std::snprintf(newPath, sizeof(newPath), "%s\\%s", gCustomPath.c_str(), tail);
  if (n <= 0 || (std::size_t)n >= size) return;

  std::memcpy(buf, newPath, (std::size_t)n + 1);
  SHCreateDirectoryExA(nullptr, newPath, nullptr);
  ++gPatched;
}

void patchAll(void* self) noexcept;
void* detourPathFunc(void* self, void* a2);

}

bool init() noexcept {
  HTGameStatus st{};
  HTGetGameStatus(&st);
  gBase = reinterpret_cast<u8*>(st.baseAddr);
  if (!gBase) return false;

  gHook.fn     = gBase + RVA_PATH_FUNC;
  gHook.detour = reinterpret_cast<void*>(detourPathFunc);

  if (HTAsmHookCreate(hModuleDll, &gHook) != HT_SUCCESS) return false;
  gOrig = reinterpret_cast<PathFunc>(gHook.origin);
  if (!gOrig) return false;

  HTAsmHookEnable(hModuleDll, gHook.fn);
  resolveCustomPath();

  void* mgr = *reinterpret_cast<void**>(gBase + RVA_MGR_GLOBAL);
  if (mgr) patchAll(mgr);

  return true;
}

void getCustomPath(char* out, std::size_t size) noexcept {
  if (!out || size == 0) return;
  resolveCustomPath();
  std::snprintf(out, size, "%s", gCustomPath.c_str());
}

void shutdown() noexcept {
  if (gHook.fn && gOrig) {
    HTAsmHookDisable(hModuleDll, gHook.fn);
    gOrig = nullptr;
  }
  if (gComInit) {
    CoUninitialize();
    gComInit = false;
  }
}

namespace {

void patchAll(void* self) noexcept {
  resolveCustomPath();
  if (!self || gCustomPath.empty()) return;
  char* base = reinterpret_cast<char*>(self);
  for (std::size_t off : PATH_OFFSETS)
    patchBuffer(base + off, PATH_BUF_SIZE);
}

void* detourPathFunc(void* self, void* a2) {
  void* result = gOrig(self, a2);
  patchAll(self);
  return result;
}

}
}
