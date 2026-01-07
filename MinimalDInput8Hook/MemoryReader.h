#pragma once
#include <windows.h>
#include <vector>
#include <string>

// Функция запуска чтения памяти
void StartMemoryReading();

// Инициализация чтения памяти
bool InitializeMemoryReader();

// Функция для поиска процесса по имени
DWORD FindProcessId(const std::wstring& processName);

// Функция для получения базового адреса модуля
DWORD GetModuleBaseAddress(DWORD procId, const std::wstring& modName);

// Функция для чтения памяти по цепочке указателей
bool ReadPointerChain(DWORD procId, DWORD baseAddress, const std::vector<DWORD>& offsets, DWORD& result);