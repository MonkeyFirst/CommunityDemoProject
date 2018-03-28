@echo off
if "%~1"=="regen" (
    for %%i in (%~dp0Build\*) do if not "%%~nxi"=="bin" del /s /q "%%i"
    if exist "%~dp0Build\CMakeFiles" rd /S /Q "%~dp0Build\CMakeFiles"
    if exist "%~dp0Build\src" rd /S /Q "%~dp0Build\src")

cd %~dp0
cmake -E make_directory Build
@set "OPT=-DURHO3D_HOME=..\..\Urho3D\Build -DURHO3D_LUA=0 -DURHO3D_TOOLS=0 -DURHO3D_PCH=0 -DURHO3D_C++11=0 -DURHO3D_OPENGL=1 -DURHO3D_WIN32_CONSOLE=1 -DURHO3D_ANGELSCRIPT=0"
cmake -E chdir Build cmake %OPT% -G "MinGW Makefiles" ..\

::pause