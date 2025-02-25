#pragma once

#include <cstdio>

#if !defined(DIRECTINPUT_VERSION)
# define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>

#if CONFIG_MAX_AXIS_VALUES
// `c_dfDIJoystick2`
# pragma comment(lib, "dinput8.lib")
#endif

class WrappedDirectInputDevice8A : public IDirectInputDevice8A {
public:
  ~WrappedDirectInputDevice8A() = default;
  WrappedDirectInputDevice8A(const WrappedDirectInputDevice8A&) = delete;
  WrappedDirectInputDevice8A& operator=(const WrappedDirectInputDevice8A&) = delete;
  WrappedDirectInputDevice8A(WrappedDirectInputDevice8A&&) = delete;
  WrappedDirectInputDevice8A& operator=(WrappedDirectInputDevice8A&&) = delete;

  static WrappedDirectInputDevice8A* Create(IDirectInputDevice8A* original) {
    return new WrappedDirectInputDevice8A(original);
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
  // IDirectInputDevice8A interface.
  //

  HRESULT Acquire() override {
    return original_->Acquire();
  }

  HRESULT BuildActionMap(
    LPDIACTIONFORMAT lpdiaf,
    LPCTSTR lpszUserName,
    DWORD dwFlags
  ) override {
    return original_->BuildActionMap(lpdiaf, lpszUserName, dwFlags);
  }

  HRESULT CreateEffect(
    REFGUID rguid,
    LPCDIEFFECT lpeff,
    LPDIRECTINPUTEFFECT* ppdeff,
    LPUNKNOWN punkOuter
  ) override {
    return original_->CreateEffect(rguid, lpeff, ppdeff, punkOuter);
  }

  HRESULT EnumCreatedEffectObjects(
    LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback,
    LPVOID pvRef,
    DWORD fl
  ) override {
    return original_->EnumCreatedEffectObjects(lpCallback, pvRef, fl);
  }

  HRESULT EnumEffects(
    LPDIENUMEFFECTSCALLBACKA lpCallback,
    LPVOID pvRef,
    DWORD dwEffType
  ) override {
    return original_->EnumEffects(lpCallback, pvRef, dwEffType);
  }

  HRESULT EnumEffectsInFile(
    LPCSTR lpszFileName,
    LPDIENUMEFFECTSINFILECALLBACK pec,
    LPVOID pvRef,
    DWORD dwFlags
  ) override {
    return original_->EnumEffectsInFile(lpszFileName, pec, pvRef, dwFlags);
  }

  HRESULT EnumObjects(
    LPDIENUMDEVICEOBJECTSCALLBACKA lpCallback,
    LPVOID pvRef,
    DWORD dwFlags
  ) override {
    return original_->EnumObjects(lpCallback, pvRef, dwFlags);
  }

  HRESULT Escape(
    LPDIEFFESCAPE pesc
  ) override {
    return original_->Escape(pesc);
  }

  HRESULT GetCapabilities(
    LPDIDEVCAPS lpDIDevCaps
  ) override {
    return original_->GetCapabilities(lpDIDevCaps);
  }

  HRESULT GetDeviceData(
    DWORD cbObjectData,
    LPDIDEVICEOBJECTDATA rgdod,
    LPDWORD pdwInOut,
    DWORD dwFlags
  ) override {
    return original_->GetDeviceData(cbObjectData, rgdod, pdwInOut, dwFlags);
  }

  HRESULT GetDeviceInfo(
    LPDIDEVICEINSTANCE pdidi
  ) override {
    return original_->GetDeviceInfo(pdidi);
  }

  HRESULT GetDeviceState(
    DWORD cbData,
    LPVOID lpvData
  ) override {
#if CONFIG_MAX_AXIS_VALUES
    if (data_format_is_joystick2_ && cbData == sizeof(DIJOYSTATE2)) {
      HRESULT hr = original_->GetDeviceState(cbData, lpvData);
      if (FAILED(hr)) {
        return hr;
      }

      DIJOYSTATE2* dst_state = reinterpret_cast<DIJOYSTATE2*>(lpvData);

      auto GetAxisMaxValue = [device = original_](DWORD offset, LONG* pResult) -> HRESULT {
        DIPROPRANGE diprg {};
        diprg.diph.dwSize = sizeof(DIPROPRANGE);
        diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        diprg.diph.dwObj = offset;
        diprg.diph.dwHow = DIPH_BYOFFSET;
        diprg.lMin = 0;
        diprg.lMax = 0;

        HRESULT hr = device->GetProperty(DIPROP_RANGE, &diprg.diph);
        if (FAILED(hr)) {
          return hr;
        }

        *pResult = diprg.lMax;
        return S_OK;
      };

      GetAxisMaxValue(DIJOFS_X, &dst_state->lX);
      GetAxisMaxValue(DIJOFS_Y, &dst_state->lY);
      GetAxisMaxValue(DIJOFS_Z, &dst_state->lZ);
      GetAxisMaxValue(DIJOFS_RX, &dst_state->lRx);
      GetAxisMaxValue(DIJOFS_RY, &dst_state->lRy);
      GetAxisMaxValue(DIJOFS_RZ, &dst_state->lRz);
      GetAxisMaxValue(DIJOFS_SLIDER(0), &dst_state->rglSlider[0]);
      GetAxisMaxValue(DIJOFS_SLIDER(1), &dst_state->rglSlider[1]);

      return S_OK;
    }
#endif

    return original_->GetDeviceState(cbData, lpvData);
  }

  HRESULT GetEffectInfo(
    LPDIEFFECTINFOA pdei,
    REFGUID rguid
  ) override {
    return original_->GetEffectInfo(pdei, rguid);
  }

  HRESULT GetForceFeedbackState(
    LPDWORD pdwOut
  ) override {
    return original_->GetForceFeedbackState(pdwOut);
  }

  HRESULT GetImageInfo(
    LPDIDEVICEIMAGEINFOHEADER lpdiDevImageInfoHeader
  ) override {
    return original_->GetImageInfo(lpdiDevImageInfoHeader);
  }

  HRESULT GetObjectInfo(
    LPDIDEVICEOBJECTINSTANCE pdidoi,
    DWORD dwObj,
    DWORD dwHow
  ) override {
    return original_->GetObjectInfo(pdidoi, dwObj, dwHow);
  }

  HRESULT GetProperty(
    REFGUID rguidProp,
    LPDIPROPHEADER pdiph
  ) override {
#if CONFIG_RENAME_DEVICES
    if (&rguidProp == &DIPROP_PRODUCTNAME) {
      HRESULT hr = original_->GetProperty(rguidProp, pdiph);
      if (FAILED(hr)) {
        return hr;
      }

      DIPROPSTRING* pdipstr = reinterpret_cast<DIPROPSTRING*>(pdiph);

      char product_name[MAX_PATH];
      WideCharToMultiByte(CP_ACP, 0, pdipstr->wsz, -1, product_name, sizeof(product_name), nullptr, nullptr);

      char renamed_product_name[MAX_PATH];
      std::snprintf(renamed_product_name, sizeof(renamed_product_name), "RENAMED: %s", product_name);
      
      MultiByteToWideChar(CP_ACP, 0, renamed_product_name, -1, pdipstr->wsz, sizeof(pdipstr->wsz) / sizeof(WCHAR));

      return S_OK;
    }
#endif

    return original_->GetProperty(rguidProp, pdiph);
  }

  HRESULT Initialize(
    HINSTANCE hinst,
    DWORD dwVersion,
    REFGUID rguid
  ) override {
    return original_->Initialize(hinst, dwVersion, rguid);
  }

  HRESULT Poll() override {
    return original_->Poll();
  }

  HRESULT RunControlPanel(
    HWND hwndOwner,
    DWORD dwFlags
  ) override {
    return original_->RunControlPanel(hwndOwner, dwFlags);
  }

  HRESULT SendDeviceData(
    DWORD cbObjectData,
    LPCDIDEVICEOBJECTDATA rgdod,
    LPDWORD pdwInOut,
    DWORD fl
  ) override {
    return original_->SendDeviceData(cbObjectData, rgdod, pdwInOut, fl);
  }

  HRESULT SendForceFeedbackCommand(
    DWORD dwFlags
  ) override {
    return original_->SendForceFeedbackCommand(dwFlags);
  }

  HRESULT SetActionMap(
    LPDIACTIONFORMATA lpdiActionFormat,
    LPCTSTR lptszUserName,
    DWORD dwFlags
  ) override {
    return original_->SetActionMap(lpdiActionFormat, lptszUserName, dwFlags);
  }

  HRESULT SetCooperativeLevel(
    HWND hwnd,
    DWORD dwFlags
  ) override {
    return original_->SetCooperativeLevel(hwnd, dwFlags);
  }

  HRESULT SetDataFormat(
    LPCDIDATAFORMAT lpdf
  ) override {
#if CONFIG_MAX_AXIS_VALUES
    // See if the data format is `c_dfDIJoystick2`, which is the data format we support.
    if (lpdf->dwSize == c_dfDIJoystick2.dwSize &&
      lpdf->dwObjSize == c_dfDIJoystick2.dwObjSize &&
      lpdf->dwFlags == c_dfDIJoystick2.dwFlags &&
      lpdf->dwDataSize == c_dfDIJoystick2.dwDataSize &&
      lpdf->dwNumObjs == c_dfDIJoystick2.dwNumObjs)
    {
      bool equal = true;

      for (DWORD i = 0; i < lpdf->dwNumObjs; ++i) {
        DIOBJECTDATAFORMAT const& lhs = lpdf->rgodf[i];
        DIOBJECTDATAFORMAT const& rhs = c_dfDIJoystick2.rgodf[i];
        
        if ((lhs.pguid == nullptr) != (rhs.pguid == nullptr)) {
          equal = false;
          break;
        }

        if ((lhs.pguid != nullptr && *lhs.pguid != *rhs.pguid) ||
          lhs.dwOfs != rhs.dwOfs ||
          lhs.dwType != rhs.dwType ||
          lhs.dwFlags != rhs.dwFlags)
        {
          equal = false;
          break;
        }
      }

      data_format_is_joystick2_ = equal;
    }
#endif

    return original_->SetDataFormat(lpdf);
  }

  HRESULT SetEventNotification(
    HANDLE hEvent
  ) override {
    return original_->SetEventNotification(hEvent);
  }

  HRESULT SetProperty(
    REFGUID rguidProp,
    LPCDIPROPHEADER pdiph
  ) override {
    return original_->SetProperty(rguidProp, pdiph);
  }

  HRESULT Unacquire() override {
    return original_->Unacquire();
  }

  HRESULT WriteEffectToFile(
    LPCSTR lpszFileName,
    DWORD dwEntries,
    LPDIFILEEFFECT rgDiFileEft,
    DWORD dwFlags
  ) override {
    return original_->WriteEffectToFile(lpszFileName, dwEntries, rgDiFileEft, dwFlags);
  }

private:
  explicit WrappedDirectInputDevice8A(IDirectInputDevice8A* original) : original_(original) {}

  IDirectInputDevice8A* original_ = nullptr;;

#if CONFIG_MAX_AXIS_VALUES
  bool data_format_is_joystick2_ = false;
#endif
};
