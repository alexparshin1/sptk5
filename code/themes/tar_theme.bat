@echo off

SET THEME=%~1
SET THEME_DIR=%~2
SET CWD=%cd%

echo Current directory: %CWD%
echo Theme directory:   %THEME_DIR%

cd %THEME_DIR%
tar cf ..\%THEME%.tar -C %THEME_DIR% %~3 %~4 %~5 %~6 %~7 %~8
