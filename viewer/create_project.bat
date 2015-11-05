@ECHO OFF

ECHO Select the type of project you would like to create:
ECHO 1. Visual Studio 2015 Solution
ECHO 2. CodeLite
ECHO 3. GNU Makefile

CHOICE /N /C:123 /M "[1-3]:"

IF ERRORLEVEL ==3 GOTO THREE
IF ERRORLEVEL ==2 GOTO TWO
IF ERRORLEVEL ==1 GOTO ONE
GOTO END

:THREE
 ECHO Creating GNU Makefile...
 ..\ud\bin\premake\premake5.exe gmake
 GOTO END
:TWO
 ECHO Creating CodeLite Project...
 ..\ud\bin\premake\premake5.exe codelite
 GOTO END
:ONE
 ECHO Creating VS2012 Project...
 ..\ud\bin\premake\premake5.exe vs2015
 GOTO END

:END
