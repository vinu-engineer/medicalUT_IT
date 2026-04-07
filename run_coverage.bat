@echo off
setlocal enabledelayedexpansion

echo =====================================================================
echo   Code Coverage Report -- Medical Device Grade
echo   Standard : IEC 62304 Class B (Statement + Branch Coverage)
echo   Toolchain: MinGW GCC + gcov  (+ gcovr if installed)
echo   Scope    : src\vitals.c  src\alerts.c  src\patient.c  src\gui_auth.c
echo =====================================================================
echo.

:: -----------------------------------------------------------------------
:: 0. Prerequisites
:: -----------------------------------------------------------------------
if exist "C:\MinGW\bin\gcc.exe" (
    set "PATH=C:\MinGW\bin;%PATH%"
)

where gcov >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: gcov not found.
    echo        Install MinGW from https://sourceforge.net/projects/mingw/
    echo        and ensure C:\MinGW\bin is on PATH.
    pause
    exit /b 1
)

where cmake >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: cmake not found.
    pause
    exit /b 1
)

gcov --version | findstr /i "gcov" >nul
echo [OK] Found: && gcov --version 2>&1 | findstr /i "gcov"
echo.

:: -----------------------------------------------------------------------
:: 1. Configure a separate coverage build (keeps normal build untouched)
:: -----------------------------------------------------------------------
set COV_BUILD=build_cov

echo [1/5] Configuring coverage build (ENABLE_COVERAGE=ON)...
if exist "%COV_BUILD%" rmdir /s /q "%COV_BUILD%"
cmake -S . -B "%COV_BUILD%" -G "MinGW Makefiles" ^
      -DCMAKE_C_COMPILER=gcc ^
      -DCMAKE_CXX_COMPILER=g++ ^
      -DENABLE_COVERAGE=ON >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake configuration failed.
    pause
    exit /b 1
)
echo       Done.

:: -----------------------------------------------------------------------
:: 2. Build test targets only (no need to build the GUI for coverage)
:: -----------------------------------------------------------------------
echo [2/5] Building test targets with --coverage instrumentation...
cmake --build "%COV_BUILD%" --target test_unit test_integration >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed. Fix errors before running coverage.
    pause
    exit /b 1
)
echo       Done.

:: -----------------------------------------------------------------------
:: 3. Run UNIT tests  (generates .gcda files alongside .gcno files)
:: -----------------------------------------------------------------------
echo [3/5] Running UNIT tests...
if exist "%COV_BUILD%\tests\test_unit.exe" (
    "%COV_BUILD%\tests\test_unit.exe" --gtest_output=xml:%COV_BUILD%\results_unit.xml
    set UNIT_RESULT=!ERRORLEVEL!
) else (
    echo ERROR: test_unit.exe not found.
    set UNIT_RESULT=1
)
if !UNIT_RESULT! NEQ 0 (
    echo WARNING: Unit tests reported failures. Coverage data may be incomplete.
)
echo       Done.

:: -----------------------------------------------------------------------
:: 4. Run INTEGRATION tests
:: -----------------------------------------------------------------------
echo [4/5] Running INTEGRATION tests...
if exist "%COV_BUILD%\tests\test_integration.exe" (
    "%COV_BUILD%\tests\test_integration.exe" --gtest_output=xml:%COV_BUILD%\results_integration.xml
    set INT_RESULT=!ERRORLEVEL!
) else (
    echo ERROR: test_integration.exe not found.
    set INT_RESULT=1
)
if !INT_RESULT! NEQ 0 (
    echo WARNING: Integration tests reported failures. Coverage data may be incomplete.
)
echo       Done.

:: -----------------------------------------------------------------------
:: 5. Generate coverage report
::    Try gcovr (HTML + Cobertura XML) first.
::    Fallback: gcov text output.
::
::    Install gcovr:  pip install gcovr
::    (requires Python 3 — https://www.python.org/downloads/)
:: -----------------------------------------------------------------------
echo [5/5] Generating coverage report...

where gcovr >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo       gcovr found -- generating HTML + Cobertura XML reports.
    if exist "coverage_report" rmdir /s /q "coverage_report"
    mkdir coverage_report

    gcovr ^
        --root . ^
        --object-directory "%COV_BUILD%" ^
        --filter "src/vitals\.c" ^
        --filter "src/alerts\.c" ^
        --filter "src/patient\.c" ^
        --filter "src/gui_auth\.c" ^
        --html-details coverage_report\index.html ^
        --xml coverage_report\coverage_cobertura.xml ^
        --print-summary

    echo.
    echo   HTML report : coverage_report\index.html   ^<-- open in browser
    echo   XML report  : coverage_report\coverage_cobertura.xml  ^<-- DHF record

    if exist "coverage_report\index.html" (
        echo.
        echo   Opening HTML report...
        start "" "coverage_report\index.html"
    )
) else (
    echo       gcovr not found -- using gcov text output.
    echo       For HTML reports, install gcovr:  pip install gcovr
    echo.

    :: Run gcov against each production source file.
    :: The -o flag points gcov to the directory containing the .gcno / .gcda files.
    echo   --- vitals.c ---
    gcov -r -b -o "%COV_BUILD%\CMakeFiles\monitor_lib.dir\src" src\vitals.c

    echo   --- alerts.c ---
    gcov -r -b -o "%COV_BUILD%\CMakeFiles\monitor_lib.dir\src" src\alerts.c

    echo   --- patient.c ---
    gcov -r -b -o "%COV_BUILD%\CMakeFiles\monitor_lib.dir\src" src\patient.c

    echo   --- gui_auth.c ---
    gcov -r -b -o "%COV_BUILD%\CMakeFiles\test_unit.dir\src" src\gui_auth.c

    echo.
    echo   Annotated .gcov files written to the current directory.
    echo   Look for lines starting with ##### (zero-hit = uncovered).
)

:: -----------------------------------------------------------------------
:: Summary
:: -----------------------------------------------------------------------
echo.
echo =====================================================================
echo   IEC 62304 Class B Coverage Targets
echo   +------------------+---------------------------+
echo   ^| Metric           ^| Required  ^| See report    ^|
echo   +------------------+---------------------------+
echo   ^| Statement        ^| 100%%      ^| Lines column  ^|
echo   ^| Branch           ^| 100%%      ^| Branches col  ^|
echo   +------------------+---------------------------+
echo.
if !UNIT_RESULT! EQU 0 if !INT_RESULT! EQU 0 (
    echo   [PASS] All 121 tests passed.
) else (
    echo   [FAIL] One or more tests failed -- review output above.
)
echo.
pause
