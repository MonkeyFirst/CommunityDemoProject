@echo off

cd %~dp0Build
mingw32-make

..\nircmd.exe waitprocess mingw32-make.exe
..\nircmd.exe wait 2000

cd ..\bin
go.exe
