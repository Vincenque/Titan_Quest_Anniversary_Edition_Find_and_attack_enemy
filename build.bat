@echo off

rem Remove the old executable if it exists
if exist "%~dp0bin\program.exe" del /q "%~dp0bin\Titan Quest Anniversary Edition - Find and attack enemy.exe.exe"

rem Create the bin directory if it doesn't exist
if not exist "%~dp0bin" mkdir "%~dp0bin"

rem Compile with debugging symbols (-g flag)
gcc -g -I"%~dp0headers" -LC:/MinGW/lib "%~dp0source\main.c" -o "%~dp0bin\Titan Quest Anniversary Edition - Find and attack enemy.exe" -lmingw32 -lgdi32
