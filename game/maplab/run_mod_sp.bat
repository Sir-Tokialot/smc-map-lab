@echo off

set GameDir=%~dp0
set GameDir=%GameDir:~0,-1%
"C:\Program Files (x86)\Steam\Steam.exe" -applaunch 243730 -windowed -game "%GameDir%"