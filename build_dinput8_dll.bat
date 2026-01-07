@echo off
echo Building DInput8 DLL for MKXL Trainer...

REM Проверяем, установлен ли Visual Studio Build Tools или Visual Studio
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\Common7\Tools\VsDevCmd.bat" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\Common7\Tools\VsDevCmd.bat"
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
) else (
    echo ERROR: Visual Studio or Build Tools not found!
    pause
    exit /b 1
)

REM Переходим в директорию проекта
cd /d "%~dp0MinimalDInput8Hook"

REM Собираем проект
msbuild MinimalDInput8Hook.vcxproj /p:Configuration=Release /p:Platform=x64

if %errorlevel% neq 0 (
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo Build completed successfully!
echo Output file should be in: x64\Release\DINPUT8.dll
pause