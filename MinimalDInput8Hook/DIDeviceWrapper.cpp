#include "stdafx.h"
#include "DIDeviceWrapper.h"
#include "CustomHooks.h"

DIDeviceWrapper::DIDeviceWrapper(IDirectInputDevice8A* pDevice) : 
    m_pDevice(pDevice), 
    m_refCount(1) 
{
}

// IUnknown methods
STDMETHODIMP DIDeviceWrapper::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    return m_pDevice->QueryInterface(riid, ppvObj);
}

STDMETHOD_(ULONG, DIDeviceWrapper::AddRef)()
{
    return InterlockedIncrement(&m_refCount);
}

STDMETHOD_(ULONG, DIDeviceWrapper::Release)()
{
    LONG count = InterlockedDecrement(&m_refCount);
    if (count == 0)
    {
        delete this;
    }
    return count;
}

// IDirectInputDevice8A methods
STDMETHODIMP DIDeviceWrapper::GetCapabilities(LPDIDEVCAPS lpDIDevCaps)
{
    return m_pDevice->GetCapabilities(lpDIDevCaps);
}

STDMETHODIMP DIDeviceWrapper::EnumObjects(LPDIENUMDEVICEOBJECTSCALLBACKA lpCallback, LPVOID pvRef, DWORD dwFlags)
{
    return m_pDevice->EnumObjects(lpCallback, pvRef, dwFlags);
}

STDMETHODIMP DIDeviceWrapper::GetProperty(REFGUID rguidProp, LPDIPROPHEADER pdiph)
{
    return m_pDevice->GetProperty(rguidProp, pdiph);
}

STDMETHODIMP DIDeviceWrapper::SetProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph)
{
    return m_pDevice->SetProperty(rguidProp, pdiph);
}

STDMETHODIMP DIDeviceWrapper::Acquire()
{
    return m_pDevice->Acquire();
}

STDMETHODIMP DIDeviceWrapper::Unacquire()
{
    return m_pDevice->Unacquire();
}

STDMETHODIMP DIDeviceWrapper::GetDeviceState(DWORD cbData, LPVOID lpvData)
{
    HRESULT hr = m_pDevice->GetDeviceState(cbData, lpvData);
    
    // Если это клавиатура и мы получили состояние, можем изменить его
    if (SUCCEEDED(hr) && lpvData && cbData == 256) // 256 байт для состояния клавиатуры
    {
        BYTE* keyboardState = static_cast<BYTE*>(lpvData);
        
        // Проверяем, нужно ли заблокировать какие-то клавиши
        for (int i = 0; i < 256; i++)
        {
            if (keyboardState[i] & 0x80) // Если клавиша нажата
            {
                if (ShouldBlockKey(static_cast<BYTE>(i)))
                {
                    keyboardState[i] = 0; // Обнуляем состояние клавиши
                }
            }
        }
    }
    
    return hr;
}

STDMETHODIMP DIDeviceWrapper::GetDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags)
{
    HRESULT hr = m_pDevice->GetDeviceData(cbObjectData, rgdod, pdwInOut, dwFlags);
    
    if (SUCCEEDED(hr) && rgdod && pdwInOut && *pdwInOut > 0)
    {
        // Обрабатываем полученные данные о нажатиях
        for (DWORD i = 0; i < *pdwInOut; i++)
        {
            if (ShouldBlockKey(static_cast<BYTE>(rgdod[i].dwOfs)))
            {
                // Убираем это событие, сдвигая остальные
                for (DWORD j = i; j < *pdwInOut - 1; j++)
                {
                    rgdod[j] = rgdod[j + 1];
                }
                (*pdwInOut)--;
                i--; // Проверяем текущую позицию снова
            }
        }
    }
    
    return hr;
}

STDMETHODIMP DIDeviceWrapper::SetDataFormat(LPCDIDATAFORMAT lpdf)
{
    return m_pDevice->SetDataFormat(lpdf);
}

STDMETHODIMP DIDeviceWrapper::SetEventNotification(HANDLE hEvent)
{
    return m_pDevice->SetEventNotification(hEvent);
}

STDMETHODIMP DIDeviceWrapper::SetCooperativeLevel(HWND hwnd, DWORD dwFlags)
{
    return m_pDevice->SetCooperativeLevel(hwnd, dwFlags);
}

STDMETHODIMP DIDeviceWrapper::GetObjectInfo(LPDIDEVICEOBJECTINSTANCEA pdidoi, DWORD dwObj, DWORD dwHow)
{
    return m_pDevice->GetObjectInfo(pdidoi, dwObj, dwHow);
}

STDMETHODIMP DIDeviceWrapper::GetDeviceInfo(LPDIDEVICEINSTANCEA pdidi)
{
    return m_pDevice->GetDeviceInfo(pdidi);
}

STDMETHODIMP DIDeviceWrapper::RunControlPanel(HWND hwndOwner, DWORD dwFlags)
{
    return m_pDevice->RunControlPanel(hwndOwner, dwFlags);
}

STDMETHODIMP DIDeviceWrapper::Initialize(HINSTANCE hinst, DWORD dwVersion, REFGUID rguid)
{
    return m_pDevice->Initialize(hinst, dwVersion, rguid);
}

STDMETHODIMP DIDeviceWrapper::CreateEffect(REFGUID rguid, LPCDIEFFECT lpeff, LPDIRECTINPUTEFFECT* ppdeff, LPUNKNOWN punkOuter)
{
    return m_pDevice->CreateEffect(rguid, lpeff, ppdeff, punkOuter);
}

STDMETHODIMP DIDeviceWrapper::EnumEffects(LPDIENUMEFFECTSCALLBACKA lpCallback, LPVOID pvRef, DWORD dwEffType)
{
    return m_pDevice->EnumEffects(lpCallback, pvRef, dwEffType);
}

STDMETHODIMP DIDeviceWrapper::GetEffectInfo(LPDIEFFECTINFOA pdei, REFGUID rguid)
{
    return m_pDevice->GetEffectInfo(pdei, rguid);
}

STDMETHODIMP DIDeviceWrapper::GetForceFeedbackState(LPDWORD pdwOut)
{
    return m_pDevice->GetForceFeedbackState(pdwOut);
}

STDMETHODIMP DIDeviceWrapper::SendForceFeedbackCommand(DWORD dwCommand)
{
    return m_pDevice->SendForceFeedbackCommand(dwCommand);
}

STDMETHODIMP DIDeviceWrapper::EnumCreatedEffectObjects(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD fl)
{
    return m_pDevice->EnumCreatedEffectObjects(lpCallback, pvRef, fl);
}

STDMETHODIMP DIDeviceWrapper::Escape(LPDIEFFESCAPE pesc)
{
    return m_pDevice->Escape(pesc);
}

STDMETHODIMP DIDeviceWrapper::Poll()
{
    // Проверяем, нужно ли запустить макрос
    CheckMacroTrigger();
    
    return m_pDevice->Poll();
}

STDMETHODIMP DIDeviceWrapper::SendData(DWORD cbObjectData, LPCDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD fl)
{
    return m_pDevice->SendData(cbObjectData, rgdod, pdwInOut, fl);
}

STDMETHODIMP DIDeviceWrapper::EnumEffectsInFile(LPCSTR lpszFileName, LPDIENUMEFFECTSINFILECALLBACK pec, LPVOID pvRef, DWORD dwFlags)
{
    return m_pDevice->EnumEffectsInFile(lpszFileName, pec, pvRef, dwFlags);
}

STDMETHODIMP DIDeviceWrapper::WriteEffectToFile(LPCSTR lpszFileName, DWORD dwEntries, LPDIFILEEFFECT rgDiFileEft, DWORD dwFlags)
{
    return m_pDevice->WriteEffectToFile(lpszFileName, dwEntries, rgDiFileEft, dwFlags);
}