#pragma once

#include "Hook.h"

// Функция для проверки, нужно ли блокировать клавишу
bool ShouldBlockKey(BYTE key);

// Функция для проверки запуска макроса
void CheckMacroTrigger();

void SetupHooks();