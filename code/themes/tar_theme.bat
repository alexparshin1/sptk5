@echo off

THEME=%~1
tar cf %THEME%.tar -C %THEME% %~2 %~3 %~4 %~5 %~6 %~7 %~8
