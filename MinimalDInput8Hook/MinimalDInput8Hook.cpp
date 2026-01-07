// MinimalDInput8Hook.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "DInput8.h"
#include "CustomHooks.h"
#include "DIDeviceWrapper.h"
DirectInput8Create_t OriginalFunction = nullptr;
HMODULE DInput8DLL = nullptr;

// Перехватываем оригинальную функцию DirectInput8Create
DINPUT8_API HRESULT DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID * ppvOut, LPUNKNOWN punkOuter)
{
	HRESULT hr = S_FALSE;
	
	if (OriginalFunction)
	{
		IDirectInputDevice8A* pOriginalDevice = nullptr;
		
		// Вызываем оригинальную функцию
		hr = OriginalFunction(hinst, dwVersion, riidltf, (LPVOID*)&pOriginalDevice, punkOuter);
		
		if (SUCCEEDED(hr) && pOriginalDevice != nullptr)
		{
			// Создаем обертку для устройства
			IDirectInputDevice8A* pWrappedDevice = new DIDeviceWrapper(pOriginalDevice);
			
			// Возвращаем обернутый объект
			*ppvOut = pWrappedDevice;
		}
	}
	
	return hr;
}
