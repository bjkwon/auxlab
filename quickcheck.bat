@echo off
auxcom _temp
::Check if the last return value is consistent with _temp.aux (currently -10)
echo %ERRORLEVEL%