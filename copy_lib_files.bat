@echo off
setlocal
if "%~1"=="" exit /b 1
if "%~2"=="" exit /b 1
set "AUXLAB_COPY_SOURCE=%~1"
set "AUXLAB_COPY_DEST=%~2"
if not exist "%AUXLAB_COPY_DEST%\" mkdir "%AUXLAB_COPY_DEST%" || exit /b 1
shift
shift
:next
if "%~1"=="" exit /b 0
copy /y "%AUXLAB_COPY_SOURCE%\%~1" "%AUXLAB_COPY_DEST%\%~1" >nul
if errorlevel 1 exit /b 1
shift
goto next
