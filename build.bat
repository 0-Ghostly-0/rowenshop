@echo off
rem Rowen Fragment - one-click Windows build.
rem Needs: Visual Studio 2022 (with "Desktop development with C++") and CMake.
rem See docs\BUILD_WINDOWS.md if anything below fails.

setlocal
cd /d "%~dp0"

where cmake >nul 2>nul
if errorlevel 1 (
    echo.
    echo   CMake was not found. Install it from https://cmake.org/download
    echo   and tick "Add CMake to the system PATH" during setup, then reopen
    echo   this window and run build.bat again.
    echo.
    pause
    exit /b 1
)

echo.
echo === Step 1/3: Configuring (first run downloads JUCE - needs internet) ===
cmake -B build -G "Visual Studio 17 2022" -A x64
if errorlevel 1 goto :failed

echo.
echo === Step 2/3: Building (this takes a few minutes) ===
cmake --build build --config Release
if errorlevel 1 goto :failed

echo.
echo === Step 3/3: Running the automated tests ===
ctest --test-dir build -C Release --output-on-failure
if errorlevel 1 goto :failed

echo.
echo =========================================================
echo   SUCCESS. Your plugin is here:
echo   build\RowenFragment_artefacts\Release\VST3\
echo.
echo   Copy "Rowen Fragment.vst3" (the whole folder) into:
echo   C:\Program Files\Common Files\VST3\
echo   then rescan plugins in your DAW.
echo =========================================================
start "" "build\RowenFragment_artefacts\Release\VST3"
pause
exit /b 0

:failed
echo.
echo   The build stopped on an error. Scroll up to the FIRST line that says
echo   "error" and send it to me - later errors are usually just echoes.
echo.
pause
exit /b 1
