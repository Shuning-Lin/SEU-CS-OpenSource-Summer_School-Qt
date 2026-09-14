@echo off
setlocal
rem Optional arguments: build_release.cmd "C:\Qt\6.11.2\mingw_64" "C:\Qt\Tools\mingw1310_64"
set "QT_DIR=%~1"
set "MINGW_DIR=%~2"
if not defined QT_DIR set "QT_DIR=C:\Qt\6.11.2\mingw_64"
if not defined MINGW_DIR set "MINGW_DIR=C:\Qt\Tools\mingw1310_64"
if not exist "%QT_DIR%\bin\qmake.exe" goto missing_tools
if not exist "%MINGW_DIR%\bin\mingw32-make.exe" goto missing_tools
set "PATH=%QT_DIR%\bin;%MINGW_DIR%\bin;%PATH%"
pushd "%~dp0" || exit /b 1
if not exist build mkdir build
pushd build || goto failed_root
"%QT_DIR%\bin\qmake.exe" ..\LightNovelSystem.pro -spec win32-g++ "CONFIG+=release" "CONFIG-=debug" "CONFIG-=debug_and_release"
if errorlevel 1 goto failed_build
"%MINGW_DIR%\bin\mingw32-make.exe" -j4
if errorlevel 1 goto failed_build
copy /y LightNovelSystem.exe ..\LightNovelSystem.exe >nul
if errorlevel 1 goto failed_build
popd
"%QT_DIR%\bin\windeployqt.exe" --release --compiler-runtime --no-translations --no-system-d3d-compiler --no-system-dxc-compiler LightNovelSystem.exe
if errorlevel 1 goto failed_root
popd
echo Build and deployment completed. Existing novel_data.json was preserved.
exit /b 0
:missing_tools
echo Qt or MinGW not found. Pass their installation directories as two arguments.
exit /b 1
:failed_build
popd
:failed_root
popd
echo Build failed. Check the error messages above. Use an ASCII-only source path.
exit /b 1
