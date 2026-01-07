#pragma once
#include <unknwn.h>
#include <dinput.h>
#include <dinput8.h>
#include <windows.h>
#include <vector>
#include <set>
#include <mutex>
#include <thread>
#include <chrono>
#include <atomic>
#include <winternl.h>

// Определение API для экспорта
#ifndef DINPUT8_API
#define DINPUT8_API extern "C" __declspec(dllexport)
#endif

typedef HRESULT(*DirectInput8Create_t)(
	HINSTANCE hinst,
	DWORD dwVersion,
	REFIID riidltf,
	LPVOID * ppvOut,
	LPUNKNOWN punkOuter
	);

extern DirectInput8Create_t OriginalFunction;
extern HMODULE DInput8DLL;

// Структура для хранения информации о макросе
struct MacroStep {
	UINT key;
	DWORD duration;  // в миллисекундах
	DWORD delay;     // задержка после выполнения в миллисекундах
};

// Глобальные переменные для управления блокировкой и макросами
extern std::set<BYTE> blockedKeys;
extern std::vector<MacroStep> macroSteps;
extern std::atomic<bool> macroRunning;
extern std::atomic<bool> inputBlockingEnabled;
extern std::atomic<DWORD> blockThreshold;
extern std::atomic<DWORD> macroThreshold;
extern std::atomic<DWORD> currentFrameValue;
extern std::mutex macroMutex;

// Объявление функций для работы с DirectInput
extern "C"
{
	DINPUT8_API HRESULT DirectInput8Create(
		HINSTANCE hinst,
		DWORD dwVersion,
		REFIID riidltf,
		LPVOID * ppvOut,
		LPUNKNOWN punkOuter
		);
}

// Функции для управления блокировкой и макросами
void InitializeInputManager();
void SetBlockedKeys(const std::set<BYTE>& keys);
void SetMacroSteps(const std::vector<MacroStep>& steps);
void StartMacro();
void StopMacro();
void SetBlockThreshold(DWORD threshold);
void SetMacroThreshold(DWORD threshold);
void SetCurrentFrameValue(DWORD value);
DWORD GetCurrentFrameValue();