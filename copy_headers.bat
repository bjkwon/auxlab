@echo off
setlocal
:next
if "%~1"=="" exit /b 0
call "%~dp0copy_header.bat" "%~1" "%~dp0%~1"
if errorlevel 1 exit /b 1
shift
goto next
