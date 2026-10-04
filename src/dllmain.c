#include <windows.h>

HMODULE hModuleDll;

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID /*lpReserved*/) {
  switch (dwReason) {
    case DLL_PROCESS_ATTACH:
      hModuleDll = hModule;
      DisableThreadLibraryCalls(hModule);
      break;
    case DLL_PROCESS_DETACH:
      break;
  }
  return TRUE;
}
