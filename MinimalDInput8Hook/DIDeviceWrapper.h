#pragma once
#include <dinput.h>
#include <wrl/client.h>
#include "DInput8.h"

// Обертка для IDirectInputDevice8
class DIDeviceWrapper : public IDirectInputDevice8A
{
private:
    Microsoft::WRL::ComPtr<IDirectInputDevice8A> m_pDevice;
    LONG m_refCount;

public:
    DIDeviceWrapper(IDirectInputDevice8A* pDevice);

    // IUnknown methods
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj) override;
    STDMETHOD_(ULONG, AddRef)() override;
    STDMETHOD_(ULONG, Release)() override;

    // IDirectInputDevice8A methods
    STDMETHOD(GetCapabilities)(LPDIDEVCAPS) override;
    STDMETHOD(EnumObjects)(LPDIENUMDEVICEOBJECTSCALLBACKA, LPVOID, DWORD) override;
    STDMETHOD(GetProperty)(REFGUID, LPDIPROPHEADER) override;
    STDMETHOD(SetProperty)(REFGUID, LPCDIPROPHEADER) override;
    STDMETHOD(Acquire)() override;
    STDMETHOD(Unacquire)() override;
    STDMETHOD(GetDeviceState)(DWORD, LPVOID) override;
    STDMETHOD(GetDeviceData)(DWORD, LPDIDEVICEOBJECTDATA, LPDWORD, DWORD) override;
    STDMETHOD(SetDataFormat)(LPCDIDATAFORMAT) override;
    STDMETHOD(SetEventNotification)(HANDLE) override;
    STDMETHOD(SetCooperativeLevel)(HWND, DWORD) override;
    STDMETHOD(GetObjectInfo)(LPDIDEVICEOBJECTINSTANCEA, DWORD, DWORD) override;
    STDMETHOD(GetDeviceInfo)(LPDIDEVICEINSTANCEA) override;
    STDMETHOD(RunControlPanel)(HWND, DWORD) override;
    STDMETHOD(Initialize)(HINSTANCE, DWORD, REFGUID) override;
    STDMETHOD(CreateEffect)(REFGUID, LPCDIEFFECT, LPDIRECTINPUTEFFECT*, LPUNKNOWN) override;
    STDMETHOD(EnumEffects)(LPDIENUMEFFECTSCALLBACKA, LPVOID, DWORD) override;
    STDMETHOD(GetEffectInfo)(LPDIEFFECTINFOA, REFGUID) override;
    STDMETHOD(GetForceFeedbackState)(LPDWORD) override;
    STDMETHOD(SendForceFeedbackCommand)(DWORD) override;
    STDMETHOD(EnumCreatedEffectObjects)(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK, LPVOID, DWORD) override;
    STDMETHOD(Escape)(LPDIEFFESCAPE) override;
    STDMETHOD(Poll)() override;
    STDMETHOD(SendData)(DWORD, LPCDIDEVICEOBJECTDATA, LPDWORD, DWORD) override;
    STDMETHOD(EnumEffectsInFile)(LPCSTR, LPDIENUMEFFECTSINFILECALLBACK, LPVOID, DWORD) override;
    STDMETHOD(WriteEffectToFile)(LPCSTR, DWORD, LPDIFILEEFFECT, DWORD) override;
};