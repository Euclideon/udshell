@ECHO OFF

IF "%QTDIR%"=="" (
  ECHO Error - QTDIR environment variable is not set
  PAUSE
  EXIT /B 1
)

%QTDIR%\\bin\\qmake.exe -spec win32-msvc2015 -tp vc %{ProjectName}.pro
