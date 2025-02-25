#pragma once

#include <cstdio>

#if !defined(DIRECTINPUT_VERSION)
# define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>

#include "wrapped_directinputdevice8.h"

class WrappedDirectInput8A : public IDirectInput8A {
public:
  ~WrappedDirectInput8A() = default;
  WrappedDirectInput8A(const WrappedDirectInput8A&) = delete;
  WrappedDirectInput8A& operator=(const WrappedDirectInput8A&) = delete;
  WrappedDirectInput8A(WrappedDirectInput8A&&) = delete;
  WrappedDirectInput8A& operator=(WrappedDirectInput8A&&) = delete;

  static WrappedDirectInput8A* Create(IDirectInput8A* original) {
    return new WrappedDirectInput8A(original);
  }

  //
  // IUnknown interface.
  //

  HRESULT QueryInterface(
    REFIID riid,
    void** ppvObject
  ) override {
    return original_->QueryInterface(riid, ppvObject);
  }

  ULONG AddRef() override {
    return original_->AddRef();
  }

  ULONG Release() override {
    ULONG const result = original_->Release();
    if (result == 0) {
      delete this;
    }
    return result;
  }

  //
  // IDirectInput8A interface.
  //

  HRESULT ConfigureDevices(
    LPDICONFIGUREDEVICESCALLBACK lpdiCallback,
    LPDICONFIGUREDEVICESPARAMS lpdiCDParams,
    DWORD dwFlags,
    LPVOID pvRefData
  ) override {
    return original_->ConfigureDevices(lpdiCallback, lpdiCDParams, dwFlags, pvRefData);
  }

  HRESULT CreateDevice(
    REFGUID rguid,
    LPDIRECTINPUTDEVICE8A* lplpDirectInputDevice,
    LPUNKNOWN pUnkOuter
  ) override {
    MessageBoxA(NULL, "WrappedDirectInput8A::CreateDevice", "direct_input_hook_test", MB_OK);

    // Call the original method to actually create the device.
    LPDIRECTINPUTDEVICE8A original_device = nullptr;
    HRESULT hr = original_->CreateDevice(rguid, &original_device, pUnkOuter);

    if (SUCCEEDED(hr)) {
      // Wrap the original device in our custom implementation.
      *lplpDirectInputDevice = WrappedDirectInputDevice8A::Create(original_device);
    }
    return hr;
  }

  HRESULT EnumDevices(
    DWORD dwDevType,
    LPDIENUMDEVICESCALLBACKA lpCallback,
    LPVOID pvRef,
    DWORD dwFlags
  ) override {
#if CONFIG_RENAME_DEVICES
    using FpCallback = BOOL(*)(LPCDIDEVICEINSTANCEA, LPVOID);

    struct Capture final {
      FpCallback original_callback = nullptr;
      LPVOID original_pvRef = nullptr;
    };

    auto DIEnumDevicesCallback = [](LPCDIDEVICEINSTANCEA lpddi, LPVOID pvRef) -> BOOL {
      Capture const& capture = *reinterpret_cast<Capture* const>(pvRef);

      char renamed_product_name[MAX_PATH];
      std::snprintf(renamed_product_name, sizeof(renamed_product_name), "RENAMED: %s", lpddi->tszProductName);

      DIDEVICEINSTANCE custom_instance = *lpddi;
      strcpy_s(custom_instance.tszProductName, renamed_product_name);

      return capture.original_callback(&custom_instance, capture.original_pvRef);
    };

    Capture capture = {
      .original_callback = lpCallback,
      .original_pvRef = pvRef,
    };

    return original_->EnumDevices(dwDevType, DIEnumDevicesCallback, &capture, dwFlags);
#endif

    return original_->EnumDevices(dwDevType, lpCallback, pvRef, dwFlags);
  }

  HRESULT EnumDevicesBySemantics(
    LPCSTR ptszUserName,
    LPDIACTIONFORMATA lpdiActionFormat,
    LPDIENUMDEVICESBYSEMANTICSCBA lpCallback,
    LPVOID pvRef,
    DWORD dwFlags
  ) override {
    return original_->EnumDevicesBySemantics(ptszUserName, lpdiActionFormat, lpCallback, pvRef, dwFlags);
  }

  HRESULT FindDevice(
    REFGUID rguidClass,
    LPCTSTR ptszName,
    LPGUID pguidInstance
  ) override {
    return original_->FindDevice(rguidClass, ptszName, pguidInstance);
  }

  HRESULT GetDeviceStatus(
    REFGUID rguidInstance
  ) override {
    return original_->GetDeviceStatus(rguidInstance);
  }

  HRESULT Initialize(
    HINSTANCE hinst,
    DWORD dwVersion
  ) override {
    return original_->Initialize(hinst, dwVersion);
  }

  HRESULT RunControlPanel(
    HWND hwndOwner,
    DWORD dwFlags
  ) override {
    return original_->RunControlPanel(hwndOwner, dwFlags);
  }

private:
  explicit WrappedDirectInput8A(IDirectInput8A* original) : original_(original) {}

  IDirectInput8A* original_ = nullptr;;
};
