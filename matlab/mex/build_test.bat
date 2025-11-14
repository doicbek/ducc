@echo off
REM Build test script for DUCC0 MATLAB MEX interface
REM This script tests if the MEX files can be built

echo ========================================
echo DUCC0 MATLAB MEX Interface - Build Test
echo ========================================
echo.

REM Check if MATLAB is available
where matlab >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: MATLAB not found in PATH
    echo Please add MATLAB to PATH or set MATLAB_ROOT
    exit /b 1
)

echo MATLAB found
echo.

REM Check if CMake is available
where cmake >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake not found in PATH
    echo Please install CMake or add it to PATH
    exit /b 1
)

echo CMake found
echo.

REM Create build directory
if not exist build mkdir build
cd build

REM Run CMake
echo Running CMake...
cmake ..
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake configuration failed
    exit /b 1
)

echo CMake configuration succeeded
echo.

REM Build
echo Building MEX files...
cmake --build . --config Release
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed
    exit /b 1
)

echo Build succeeded
echo.

REM Check if MEX files were created
echo Checking for MEX files...
dir *.mex* /b >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo WARNING: No MEX files found in build directory
    exit /b 1
)

echo MEX files found:
dir *.mex* /b

echo.
echo ========================================
echo Build test completed successfully
echo ========================================
echo.

cd ..

