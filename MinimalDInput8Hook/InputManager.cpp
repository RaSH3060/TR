#include "stdafx.h"
#include "DInput8.h"
#include <thread>
#include <chrono>

// Определения глобальных переменных
std::set<BYTE> blockedKeys;
std::vector<MacroStep> macroSteps;
std::atomic<bool> macroRunning(false);
std::atomic<bool> inputBlockingEnabled(false);
std::atomic<DWORD> blockThreshold(260); // по умолчанию 260 мс
std::atomic<DWORD> macroThreshold(280); // по умолчанию 280 мс
std::atomic<DWORD> currentFrameValue(0);
std::mutex macroMutex;

// Функция для инициализации менеджера ввода
void InitializeInputManager() {
    // Инициализация переменных
    blockedKeys.clear();
    macroSteps.clear();
    macroRunning = false;
    inputBlockingEnabled = false;
    blockThreshold = 260;
    macroThreshold = 280;
    currentFrameValue = 0;
}

// Установка заблокированных клавиш
void SetBlockedKeys(const std::set<BYTE>& keys) {
    std::lock_guard<std::mutex> lock(macroMutex);
    blockedKeys = keys;
}

// Установка шагов макроса
void SetMacroSteps(const std::vector<MacroStep>& steps) {
    std::lock_guard<std::mutex> lock(macroMutex);
    macroSteps = steps;
}

// Запуск макроса
void StartMacro() {
    if (macroSteps.empty()) {
        return;
    }
    
    macroRunning = true;
    
    // Запускаем выполнение макроса в отдельном потоке
    std::thread([]
    {
        for (const auto& step : macroSteps) {
            if (!macroRunning) break;
            
            // Имитация нажатия клавиши
            if (step.key >= 0x01 && step.key <= 0xFE) {
                INPUT input;
                input.type = INPUT_KEYBOARD;
                input.ki.wVk = step.key;
                input.ki.wScan = MapVirtualKey(step.key, MAPVK_VK_TO_VSC);
                input.ki.dwFlags = 0; // Нажатие
                input.ki.time = 0;
                input.ki.dwExtraInfo = 0;
                
                SendInput(1, &input, sizeof(INPUT));
                
                // Ждем заданное время
                std::this_thread::sleep_for(std::chrono::milliseconds(step.duration));
                
                // Отпускание клавиши
                input.ki.dwFlags = KEYEVENTF_KEYUP;
                SendInput(1, &input, sizeof(INPUT));
            }
            
            // Задержка после выполнения шага
            if (step.delay > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(step.delay));
            }
            
            if (!macroRunning) break;
        }
        
        macroRunning = false;
    }).detach();
}

// Остановка макроса
void StopMacro() {
    macroRunning = false;
}

// Установка порога блокировки
void SetBlockThreshold(DWORD threshold) {
    blockThreshold = threshold;
}

// Установка порога макроса
void SetMacroThreshold(DWORD threshold) {
    macroThreshold = threshold;
}

// Установка текущего значения фрейма
void SetCurrentFrameValue(DWORD value) {
    currentFrameValue = value;
}

// Получение текущего значения фрейма
DWORD GetCurrentFrameValue() {
    return currentFrameValue.load();
}