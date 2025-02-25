
#include <Windows.h>

#if !defined(DIRECTINPUT_VERSION)
# define DIRECTINPUT_VERSION 0x0800
#endif
#include <initguid.h>
#include <dinput.h>

// Microsoft Detours: https://github.com/microsoft/Detours
#include <detours.h>

#include "wrapped_directinput8.h"

// Microsoft Detours requires an exported function with ordinal #1 to be present in the DLL.
// See also: `hook.def`.
void __declspec(dllexport) __stdcall DummyExportedFunction() {
  MessageBoxA(NULL, "DummyExportedFunction", "direct_input_hook_test", MB_OK);
}

using FpDirectInput8Create = HRESULT(*)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
using FpCoCreateInstance = HRESULT(*)(REFCLSID, LPUNKNOWN, DWORD, REFIID, LPVOID*);

// Function pointers to the original functions.
struct Functions {
  FpDirectInput8Create fpDirectInput8Create = nullptr;
  FpCoCreateInstance fpCoCreateInstance = nullptr;
};

static Functions g_target;
/// Function pointers to the original functions.
static Functions g_original;

HRESULT WINAPI Hooked_DirectInput8Create(
  HINSTANCE hinst,
  DWORD dwVersion,
  REFIID riidltf,
  LPVOID* ppvOut,
  LPUNKNOWN punkOuter
) {
  if (riidltf == IID_IDirectInput8A) {
    // Something like:
    //  DirectInput8Create(hinst, dwVersion, &IID_IDirectInput8A, ppvOut, punkOuter);

    // Call the original function to get the original IDirectInput8A object.
    LPDIRECTINPUT8A original_direct_input = nullptr;
    HRESULT result = g_original.fpDirectInput8Create(hinst, dwVersion, riidltf, reinterpret_cast<LPVOID*>(&original_direct_input), punkOuter);
    if (FAILED(result)) {
      return result;
    }

    // Wrap the original IDirectInput8A object.
    LPDIRECTINPUT8A wrapped_direct_input = WrappedDirectInput8A::Create(original_direct_input);

    // Return the wrapped object.
    *ppvOut = wrapped_direct_input;
    return S_OK;
  }
  else if (riidltf == IID_IDirectInput8W) {
    // Something like:
    //  DirectInput8Create(hinst, dwVersion, &IID_IDirectInput8W, ppvOut, punkOuter);
    MessageBoxA(NULL, "IID_IDirectInput8W: Not supported.", "direct_input_hook_test", MB_ICONEXCLAMATION | MB_OK);

    // Not supported.
  }

  // Call the original function.
  return g_original.fpDirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);
}

HRESULT WINAPI Hooked_CoCreateInstance(
  REFCLSID  rclsid,
  LPUNKNOWN pUnkOuter,
  DWORD     dwClsContext,
  REFIID    riid,
  LPVOID* ppv
) {
  if (rclsid == CLSID_DirectInput8) {
    if (riid == IID_IDirectInput8A) {
      // Something like:
      //  CoCreateInstance(&CLSID_DirectInput8, pUnkOuter, dwClsContext, &IID_IDirectInput8A, ppv);

      // Call the original function to get the original IDirectInput8A object.
      LPDIRECTINPUT8A original_direct_input = nullptr;
      HRESULT result = g_original.fpCoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, reinterpret_cast<LPVOID*>(&original_direct_input));
      if (FAILED(result)) {
        return result;
      }

      // Wrap the original IDirectInput8A object.
      LPDIRECTINPUT8A wrapped_direct_input = WrappedDirectInput8A::Create(original_direct_input);

      // Return the wrapped object.
      *ppv = wrapped_direct_input;
    }
    else if (riid == IID_IDirectInput8W) {
      // Something like:
      //  CoCreateInstance(&CLSID_DirectInput8, pUnkOuter, dwClsContext, &IID_IDirectInput8W, ppv);
      MessageBoxA(NULL, "CLSID_DirectInput8 && IID_IDirectInput8W: Not supported.", "direct_input_hook_test", MB_ICONEXCLAMATION | MB_OK);

      // Not supported.
    }
  }

  // Call the original function.
  return g_original.fpCoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
  HMODULE hModuleDinput8 = nullptr;
  HMODULE hModuleOle32 = nullptr;

  if (fdwReason == DLL_PROCESS_ATTACH) {
    MessageBoxA(NULL, "DLL_PROCESS_ATTACH", "direct_input_hook_test", MB_OK);

    // Get the original pointers to the functions we want to hook.
    {
      hModuleDinput8 = GetModuleHandleA("dinput8.dll");
      if (hModuleDinput8 == NULL) {
        MessageBoxA(NULL, "Failed to load original dinput8.dll", "direct_input_hook_test", MB_ICONERROR | MB_OK);
        return FALSE;
      }

      g_target.fpDirectInput8Create = (FpDirectInput8Create)GetProcAddress(hModuleDinput8, "DirectInput8Create");
    }
    {
      hModuleOle32 = GetModuleHandleA("ole32.dll");
      if (hModuleOle32 == NULL) {
        MessageBoxA(NULL, "Failed to load original ole32.dll", "direct_input_hook_test", MB_ICONERROR | MB_OK);
        return FALSE;
      }
      g_target.fpCoCreateInstance = (FpCoCreateInstance)GetProcAddress(hModuleOle32, "CoCreateInstance");
    }

    // Detour the original functions.
    {
      DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());
      DetourAttachEx(
        reinterpret_cast<PVOID*>(&g_target.fpDirectInput8Create),
        Hooked_DirectInput8Create,
        reinterpret_cast<PDETOUR_TRAMPOLINE*>(&g_original.fpDirectInput8Create), // Receive the function pointer to call the original function.
        nullptr, nullptr
      );
      DetourAttachEx(
        reinterpret_cast<PVOID*>(&g_target.fpCoCreateInstance),
        Hooked_CoCreateInstance,
        reinterpret_cast<PDETOUR_TRAMPOLINE*>(&g_original.fpCoCreateInstance), // Receive the function pointer to call the original function.
        nullptr, nullptr
      );
      DetourTransactionCommit();

      DetourRestoreAfterWith();
    }
  }
  else if (fdwReason == DLL_PROCESS_DETACH) {
    MessageBoxA(NULL, "DLL_PROCESS_DETACH", "direct_input_hook_test", MB_OK);

    // Restore the original functions.
    {
      DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());
      DetourDetach(
        reinterpret_cast<PVOID*>(&g_target.fpDirectInput8Create),
        Hooked_DirectInput8Create
      );
      DetourDetach(
        reinterpret_cast<PVOID*>(&g_target.fpCoCreateInstance),
        Hooked_CoCreateInstance
      );
      DetourTransactionCommit();
    }

    if (hModuleDinput8 != nullptr) {
      FreeLibrary(hModuleDinput8);
    }
    if (hModuleOle32 != nullptr) {
      FreeLibrary(hModuleOle32);
    }
  }

  return TRUE;
}
