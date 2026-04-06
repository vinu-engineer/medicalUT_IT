@echo off
echo =====================================================
echo   Generate Architecture + Design Documentation
echo   Tool   : Doxygen + Graphviz
echo   Output : docs\html\index.html
echo   Standard: IEC 62304 Class B
echo =====================================================
echo.

:: Check Doxygen is installed
where doxygen >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Doxygen not found.
    echo.
    echo Install it with:
    echo   winget install --id DimitriVanHeesch.Doxygen
    pause
    exit /b 1
)
echo [OK] Doxygen found.

:: Graphviz check (informational only -- Doxygen handles it via DOT_PATH)
if exist "C:\Program Files\Graphviz\bin\dot.exe" (
    echo [OK] Graphviz found.
) else (
    echo [NOTE] Graphviz not found in default location.
    echo        Diagrams will be skipped. Install with:
    echo        winget install --id Graphviz.Graphviz
)

:: Clean previous output
echo.
echo [1/3] Cleaning previous docs...
if exist "docs" rmdir /s /q "docs"
mkdir docs
echo       Done.

:: Run Doxygen
echo.
echo [2/3] Running Doxygen...
doxygen Doxyfile
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Doxygen failed. See output above.
    pause
    exit /b 1
)

:: Open result
echo.
echo [3/3] Done.
echo.
echo   HTML   : docs\html\index.html
echo   XML    : docs\xml\
echo   Warnings: docs\doxygen_warnings.log
echo.
start "" "docs\html\index.html"
pause
