@ECHO OFF

ECHO Select the type of project you would like to create:
ECHO 1. Visual Studio 2015 Solution
ECHO 2. Visual Studio 2015 Solution - Web
ECHO 3. GNU Makefile
ECHO 4. GNU Makefile - Web

CHOICE /N /C:1234 /M "[1-4]:"

IF ERRORLEVEL ==4 GOTO FOUR
IF ERRORLEVEL ==3 GOTO THREE
IF ERRORLEVEL ==2 GOTO TWO
IF ERRORLEVEL ==1 GOTO ONE
GOTO END

:FOUR
 ECHO Creating GNU Makefile - Web...
 bin\premake\premake5.exe --file=premake5-web.lua gmake
 GOTO END
:THREE
 ECHO Creating GNU Makefile...
 bin\premake\premake5.exe gmake
 GOTO END
:TWO
 ECHO Creating VS2015 Project - Web...
 bin\premake\premake5.exe --file=premake5-web.lua vs2015
 GOTO END
:ONE
 ECHO Creating VS2015 Project...
 bin\premake\premake5.exe vs2015
 GOTO END

:END
