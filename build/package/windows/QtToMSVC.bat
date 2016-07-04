@ECHO OFF

IF "%QTDIR%"=="" (
  ECHO Error - QTDIR environment variable is not set
  EXIT /B 1
)

IF "%1"=="" (
  ECHO Usage: QtToMSVC project.pro
  ECHO Where "project.pro" is the full absolute filename of the qmake project file to convert
  EXIT /B 1
)

%QTDIR%\bin\qmake.exe -spec win32-msvc2015 -tp vc %1