@ECHO OFF

IF "%QTDIR%"=="" (
  ECHO Error - QTDIR environment variable is not set
  PAUSE
  EXIT /B 1
)

%QTDIR%\\bin\\qmake.exe -spec win32-msvc2015 -tp vc %{ProjectName}.pro
powershell -Command "(gc %{ProjectName}.vcxproj) -replace 'Qt5Cored.lib', 'Qt5Core.lib' | Out-File -Encoding ASCII %{ProjectName}.vcxproj"
