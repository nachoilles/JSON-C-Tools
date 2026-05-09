@echo off
setlocal

if "%~1"=="" goto all

if /I "%~1"=="tokenizer" goto tokenizer
if /I "%~1"=="cursor" goto cursor
if /I "%~1"=="json" goto json

echo Unknown test: %~1
echo.
echo Usage:
echo   run_tests.bat tokenizer
echo   run_tests.bat cursor
echo   run_tests.bat json
echo   run_tests.bat
exit /b 1

:tokenizer
gcc tokenizer_test.c ..\src\tokenizer.c -o test.exe
if errorlevel 1 exit /b 1

.\test.exe
del .\test.exe
exit /b 0

:cursor
gcc cursor_test.c ..\src\tokenizer.c ..\src\cursor.c -o test.exe
if errorlevel 1 exit /b 1

.\test.exe
del .\test.exe
exit /b 0

:json
gcc json_test.c ..\src\tokenizer.c ..\src\cursor.c ..\src\json.c ..\src\table.c -o test.exe
if errorlevel 1 exit /b 1

.\test.exe
del .\test.exe
exit /b 0

:all
call "%~f0" tokenizer
call "%~f0" cursor
call "%~f0" json