@echo off
setlocal

echo ========================================
echo SeshNx Reactor - Build Script
echo ========================================

set BUILD_TYPE=%1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=Release

echo Build Type: %BUILD_TYPE%
echo.

:: Create build directory
if not exist build mkdir build

:: Configure CMake
echo [1/2] Configuring CMake...
cmake -B build -S . -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
if errorlevel 1 (
    echo CMake configuration failed!
    exit /b 1
)

:: Build
echo.
echo [2/2] Building...
cmake --build build --config %BUILD_TYPE%
if errorlevel 1 (
    echo Build failed!
    exit /b 1
)

echo.
echo ========================================
echo Build Successful!
echo ========================================
echo.
echo Output locations:
echo   VST3:       build\Reactor_artefacts\%BUILD_TYPE%\VST3\
echo   Standalone: build\Reactor_artefacts\%BUILD_TYPE%\Standalone\
echo.

endlocal
