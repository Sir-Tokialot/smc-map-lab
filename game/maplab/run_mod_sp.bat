@echo off

set GameDir=%~dp0
set GameDir=%GameDir:~0,-1%
"D:\SteamLibrary\steamapps\common\Source SDK Base 2013 Singleplayer\hl2.exe" -windowed -noborder -game "%GameDir%"