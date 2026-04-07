@echo off
echo ================================
echo   Building Patient Monitor
echo ================================

:: Check if cmake is available
where cmake >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake not found. Please install CMake from https://cmake.org/download/
    pause
    exit /b 1
)

:: Create build folder if it doesn't exist
if not exist "build" (
    echo Creating build folder...
    mkdir build
)

:: Run CMake configure
echo.
echo Configuring...
cmake -S . -B build
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake configuration failed.
    pause
    exit /b 1
)

:: Build the project
echo.
echo Building...
cmake --build build
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed.
    pause
    exit /b 1
)

:: Run the GUI executable
echo.
echo ================================
echo   Launching Patient Monitor GUI
echo ================================
if exist "build\Debug\patient_monitor_gui.exe" (
    start "" "build\Debug\patient_monitor_gui.exe"
) else if exist "build\patient_monitor_gui.exe" (
    start "" "build\patient_monitor_gui.exe"
) else (
    echo WARNING: GUI executable not found. Launching console app instead.
    build\Debug\patient_monitor.exe 2>nul || build\patient_monitor.exe
)

echo.
echo Done!
pause
