@echo off
set VERSION=0.2

rem update version.h
for /f "usebackq" %%H in (`git rev-parse --short HEAD`) do set COMMITHASH=%%H
echo #pragma once>version.h
echo constexpr const wchar_t *version = L"%VERSION% ( %COMMITHASH% )";>>version.h

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsMSBuildCmd.bat"
cd %~dp0
rmdir /s /q ..\build .\dist
mkdir ..\build
cd ..\build
cmake -G "Visual Studio 16" -A x64 ..
MSBuild cef.sln /t:aviutl_browser:rebuild /p:Configuration=Release;Platform="x64"
del aviutl_browser\Release\aviutl_browser.exp
del aviutl_browser\Release\aviutl_browser.lib
cd %~dp0

bash pack.bash
