@echo off
setlocal
if "%~1"=="" exit /b 1
if "%~2"=="" exit /b 1
if not exist "%~dp0include\" mkdir "%~dp0include" || exit /b 1
copy /y "%~2\%~1.h" "%~dp0include\%~1.h" >nul
exit /b %errorlevel%
