if not exist "game_maplab.sln" (
	devtools\bin\vpc.exe /maplab +game /mksln game_maplab.sln
) else (
	devtools\bin\vpc.exe /maplab +game
	)
pause