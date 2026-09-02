@echo off

SET THEME=%~1
SET THEME_DIR=%~2
tar cf %THEME%.tar -C %THEME_DIR% %~3 %~4 %~5 %~6 %~7 %~8
