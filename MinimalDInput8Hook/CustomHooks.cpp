#include "stdafx.h"
#include "CustomHooks.h"
#include "DInput8.h"

// Типы функций для перехвата
typedef HANDLE(*CreateFileA_t)(
	LPCSTR                lpFileName,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwCreationDisposition,
	DWORD                 dwFlagsAndAttributes,
	HANDLE                hTemplateFile);

typedef HANDLE(*CreateFileW_t)(
	LPCWSTR                lpFileName,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwCreationDisposition,
	DWORD                 dwFlagsAndAttributes,
	HANDLE                hTemplateFile);

// Оригинальные функции
CreateFileA_t OriginalCreateFileA;
CreateFileW_t OriginalCreateFileW;

// Перехватчик CreateFileA
HANDLE CreateFileA_Wrapper(
	LPCSTR                lpFileName,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwCreationDisposition,
	DWORD                 dwFlagsAndAttributes,
	HANDLE                hTemplateFile
)
{
	// Выводим имя файла для отладки
	WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), lpFileName, (DWORD)strlen(lpFileName), nullptr, nullptr);
	WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), "\n", 1, nullptr, nullptr);

	// Вызываем оригинальную функцию
	return OriginalCreateFileA(
		lpFileName,
		dwDesiredAccess,
		dwShareMode,
		lpSecurityAttributes,
		dwCreationDisposition,
		dwFlagsAndAttributes,
		hTemplateFile);
}

// Перехватчик CreateFileW
HANDLE CreateFileW_Wrapper(
	LPCWSTR                lpFileName,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwCreationDisposition,
	DWORD                 dwFlagsAndAttributes,
	HANDLE                hTemplateFile
)
{
	// Выводим имя файла для отладки
	WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), lpFileName, (DWORD)wcslen(lpFileName), nullptr, nullptr);
	WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), L"\n", 1, nullptr, nullptr);

	// Вызываем оригинальную функцию
	return OriginalCreateFileW(
		lpFileName,
		dwDesiredAccess,
		dwShareMode,
		lpSecurityAttributes,
		dwCreationDisposition,
		dwFlagsAndAttributes,
		hTemplateFile);
}

// Функция для проверки, нужно ли блокировать клавишу
bool ShouldBlockKey(BYTE key) {
	if (!inputBlockingEnabled) return false;
	
	DWORD currentFrame = GetCurrentFrameValue();
	DWORD threshold = blockThreshold.load();
	
	// Если текущее значение фрейма достигло или превысило порог блокировки
	if (currentFrame >= threshold) {
		std::lock_guard<std::mutex> lock(macroMutex);
		return blockedKeys.find(key) != blockedKeys.end();
	}
	
	return false;
}

// Функция для проверки запуска макроса
void CheckMacroTrigger() {
	DWORD currentFrame = GetCurrentFrameValue();
	DWORD threshold = macroThreshold.load();
	
	// Если текущее значение фрейма достигло или превысило порог макроса
	// и макрос еще не запущен
	if (currentFrame >= threshold && !macroRunning) {
		StartMacro();
	}
}

// Функция настройки хуков
void SetupHooks()
{
	// Создаем консоль для отладочного вывода
	AllocConsole();

	// Инициализируем менеджер ввода
	InitializeInputManager();

	// Устанавливаем хуки
	OriginalCreateFileA = HookFunction("KERNEL32.dll", "CreateFileA", &CreateFileA_Wrapper);
	OriginalCreateFileW = HookFunction("KERNEL32.dll", "CreateFileW", &CreateFileW_Wrapper);
}

