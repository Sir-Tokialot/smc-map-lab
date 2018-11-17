@echo off
title VPC Complete Project File Generator for MapLab
REM // this gets inputs from the user, then selects the options to goto from them, and continues going back and forth
REM // between the executing whats in the options and selecting the options, until there are no more, 
REM // and then it dumps everything into the vpc command line, which you see a preview of it before you run it
:choose
set /p mksln=Create New Solution File? (y or nothing): 
if "%mksln%" == "y" (set /p sln_name=Enter a name for the solution file:)
set /p force=Force Rebuild all projects? (y or nothing): 

REM Groups
echo Choose groups to build:
echo - nothing for game
echo - 1 for everything
echo - 2 for gamedlls
echo - 3 for shaders
echo - 4 for dedicated
echo - or type in 0 to type in a group: 
echo -------------------------------------
echo (no spaces allowed, commas not needed)
set /p group="Selections: "

REM ==========================================
REM Projects

set /p project=Build Projects (n or nothing for yes)? 

REM =====================================================================================================================

:select_mksln
if "%mksln%" == "y" ( goto make_sln
) else goto dont_make_sln

:select_force
if "%force%" == "y" ( goto rebuild
) else goto dont_rebuild

REM ============================================================
REM groups

:select_group_everything
REM checks if the variable is empty
if [%group%] == [] (set grp_all=+game & goto select_project
) else goto select_group_0

:select_group_0
REM this searches for the character 0 in %group%, and goes to group_custom if it finds it, else it goes to select_group_1
(echo %group% | findstr /i /c:"0" >nul) && (goto group_add) || (goto select_group_1)

:select_group_1
REM this searches for the character 1 in %group%, and goes to group_1 if it finds it, else it goes to select_group_2
(echo %group% | findstr /i /c:"1" >nul) && (goto group_1) || (goto select_group_2)

:select_group_2
(echo %group% | findstr /i /c:"2" >nul) && (goto group_2) || (goto select_group_3)

:select_group_3
(echo %group% | findstr /i /c:"3" >nul) && (goto group_3) || (goto select_group_4)

:select_group_4
(echo %group% | findstr /i /c:"4" >nul) && (goto group_4) || (goto select_project)

REM ============================================================
REM projects

:select_project
REM this is for default projects
if "%project%" == "n" ( goto createprojects
) else set proj=/maplab & goto createprojects 

REM =====================================================================================================================

REM ==============================
REM Solution File
:make_sln
set mksln=/mksln
goto select_force

:dont_make_sln
set mksln=
goto select_force

REM ==============================
REM Force Rebuild Projects
:rebuild
set force=/f
goto select_group_everything

:dont_rebuild
set force=
goto select_group_everything

REM ==============================
REM Groups

:group_add
echo ------------------------------
echo Enter project groups you want
echo make sure each group looks like this: +example
echo and space each word out

set /p grp_add=Groups: 
echo ------------------------------
goto select_group_1

:group_1
set grp_01=+everything
goto select_group_2

:group_2
set grp_02=+gamedlls
goto select_group_3

:group_3
set grp_03=+shaders
goto select_group_4

:group_4
set grp_04=+dedicated
goto select_project

:createprojects
echo =============================================
echo Current VPC command line:
echo %grp_all% %grp_add% %grp_01% %grp_02% %grp_03% %grp_04% %force% %proj% %mksln% "%sln_name%"
pause
echo =============================================
REM can i merge all the groups into one variable? %groups%? maybe with projects as well?
devtools\bin\vpc %grp_all% %grp_add% %grp_01% %grp_02% %grp_03% %grp_04% %force% %proj% %mksln% "%sln_name%"
pause
exit
