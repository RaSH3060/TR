#include "stdafx.h"
#include "MemoryReader.h"
#include "DInput8.h"
#include <psapi.h>
#include <thread>
#include <chrono>

// Глобальные переменные для работы с памятью
DWORD g_processId = 0;
HANDLE g_hProcess = nullptr;
DWORD g_baseAddress = 0x33C8A98; // Базовый адрес по ТЗ
std::vector<DWORD> g_offsets = {0x8, 0x130, 0x108, 0x78, 0x90, 0x120, 0xF20}; // Оффсеты по ТЗ

// Функция для поиска процесса по имени
DWORD FindProcessId(const std::wstring& processName)
{
    PROCESSENTRY32W processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32W);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    if (Process32FirstW(snapshot, &processEntry)) {
        do {
            if (processName == processEntry.szExeFile) {
                CloseHandle(snapshot);
                return processEntry.th32ProcessID;
            }
        } while (Process32NextW(snapshot, &processEntry));
    }

    CloseHandle(snapshot);
    return 0;
}

// Функция для получения базового адреса модуля
DWORD GetModuleBaseAddress(DWORD procId, const std::wstring& modName)
{
    MODULEENTRY32W moduleEntry;
    moduleEntry.dwSize = sizeof(MODULEENTRY32W);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    if (Module32FirstW(snapshot, &moduleEntry)) {
        do {
            if (modName == moduleEntry.szModule) {
                CloseHandle(snapshot);
                return (DWORD)moduleEntry.modBaseAddr;
            }
        } while (Module32NextW(snapshot, &moduleEntry));
    }

    CloseHandle(snapshot);
    return 0;
}

// Функция для чтения памяти по цепочке указателей
bool ReadPointerChain(DWORD procId, DWORD baseAddress, const std::vector<DWORD>& offsets, DWORD& result)
{
    HANDLE hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, procId);
    if (hProcess == NULL) {
        return false;
    }

    DWORD address = baseAddress;
    SIZE_T bytesRead;
    
    // Считываем базовый адрес
    if (!ReadProcessMemory(hProcess, (LPCVOID)address, &address, sizeof(DWORD), &bytesRead) || bytesRead != sizeof(DWORD)) {
        CloseHandle(hProcess);
        return false;
    }

    // Проходим по цепочке оффсетов
    for (size_t i = 0; i < offsets.size(); i++) {
        address += offsets[i];
        
        if (i < offsets.size() - 1) { // Не на последнем элементе
            if (!ReadProcessMemory(hProcess, (LPCVOID)address, &address, sizeof(DWORD), &bytesRead) || bytesRead != sizeof(DWORD)) {
                CloseHandle(hProcess);
                return false;
            }
        } else { // На последнем элементе - возвращаем значение
            DWORD value;
            if (!ReadProcessMemory(hProcess, (LPCVOID)address, &value, sizeof(DWORD), &bytesRead) || bytesRead != sizeof(DWORD)) {
                CloseHandle(hProcess);
                return false;
            }
            result = value;
        }
    }

    CloseHandle(hProcess);
    return true;
}

// Инициализация чтения памяти
bool InitializeMemoryReader()
{
    // Ищем процесс MK10.exe
    g_processId = FindProcessId(L"MK10.exe");
    if (g_processId == 0) {
        return false;
    }

    // Открываем процесс
    g_hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, g_processId);
    if (g_hProcess == NULL) {
        return false;
    }

    // Получаем базовый адрес модуля
    g_baseAddress = GetModuleBaseAddress(g_processId, L"MK10.exe");
    if (g_baseAddress == 0) {
        CloseHandle(g_hProcess);
        g_hProcess = nullptr;
        return false;
    }

    // Корректируем базовый адрес с учетом смещения из ТЗ
    g_baseAddress += 0x33C8A98;

    return true;
}

// Основной цикл чтения памяти
void MemoryReadingLoop()
{
    while (true) {
        DWORD frameValue = 0;
        if (ReadPointerChain(g_processId, g_baseAddress, g_offsets, frameValue)) {
            // Обновляем текущее значение фрейма
            SetCurrentFrameValue(frameValue);
        }
        
        // Ждем 10 мс перед следующим чтением (как указано в ТЗ)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// Функция запуска чтения памяти
void StartMemoryReading()
{
    if (!InitializeMemoryReader()) {
        return;
    }
    
    // Запускаем цикл чтения памяти в отдельном потоке
    std::thread(MemoryReadingLoop).detach();
}