# This installs two files, app.exe and logo.ico, creates a start menu shortcut, builds an uninstaller, and
# adds uninstall information to the registry for Add/Remove Programs
 
# To get started, put this script into a folder with the two files (app.exe, logo.ico, and license.rtf -
# You'll have to create these yourself) and run makensis on it
 
# If you change the names "app.exe", "logo.ico", or "license.rtf" you should do a search and replace - they
# show up in a few places.
# All the other settings can be tweaked by editing the !defines at the top of this script
!define APPNAME "Platform SDK"
!define COMPANYNAME "Euclideon"
!define DESCRIPTION "Platform Software Development Kit"
# These three must be integers
!define VERSIONMAJOR 0
!define VERSIONMINOR 9
!define VERSIONBUILD 0
# These will be displayed by the "Click here for support information" link in "Add/Remove Programs"
# It is possible to use "mailto:" links in here to open the email client
#!define HELPURL "http://..." # "Support Information" link
#!define UPDATEURL "http://..." # "Product Updates" link
#!define ABOUTURL "http://..." # "Publisher" link
# This is the size (in kB) of all the files copied into "Program Files"
#!define INSTALLSIZE 7233

!ifdef TARGET32
InstallDir "$PROGRAMFILES32\Euclideon\PlatformSDK"
!endif 

!ifdef TARGET64
InstallDir "$PROGRAMFILES64\Euclideon\PlatformSDK"
!endif 

RequestExecutionLevel admin ;Require admin rights on NT6+ (When UAC is turned on)
 
 
# rtf or txt file - remember if it is txt, it must be in the DOS text format (\r\n)
LicenseData "license.rtf"
# This will be in the installer/uninstaller's title bar
Name "${COMPANYNAME} - ${APPNAME}"
#Icon "logo.ico"
outFile "EP_Setup.exe"
 
!include LogicLib.nsh
!include nsDialogs.nsh
 
Var Dialog
Var Label
#Var Text
#Var Text_State
Var QtCCheckbox
Var QtCCheckbox_State
Var QtLaunchQtCreatorCheckbox
Var QtLaunchQtCreatorCheckbox_State
Var QtBrowseButton
Var QtFolder
Var QtEnvPath
Var QtFolderText
Var VSCheckbox
Var VSCheckbox_State
Var InstalButton
Var textBuffer
Var comp
Var VSPath
# Just three pages - license agreement, install location, and installation
Page license
Page directory
Page custom nsDialogsPage
#Page custom installMessage
Page instfiles
 
!macro VerifyUserIsAdmin
UserInfo::GetAccountType
pop $0
${If} $0 != "admin" ;Require admin rights on NT4+
        messageBox mb_iconstop "Administrator rights required!"
        setErrorLevel 740 ;ERROR_ELEVATION_REQUIRED
        quit
${EndIf}
!macroend

Function .onInit
  setShellVarContext all
  !insertmacro VerifyUserIsAdmin
FunctionEnd


Function nsDialogsPage

  ReadEnvStr $0 QTDIR
  ${If} $0 != error
      StrCpy $0 "$0\..\..\Tools\QtCreator\bin"            
  ${Else}
    ReadEnvStr $0 QT_DIR
    ${If} $0 != error
      StrCpy $0 "$0\..\..\Tools\QtCreator\bin"
    ${Endif}
  ${EndIf} 
  
  GetFullPathName $QtEnvPath $0
  ${NSD_SetText} $QtEnvPath $QtFolder
  
  GetDlgItem $InstalButton $hwndparent 1 ; Get the handle to the button
#  SendMessage $0 ${WM_SETTEXT} 0 `STR:$(^NextBtn)` ; The part after STR: can be any string, using the next button language string here
  
  
  nsDialogs::Create 1018
  Pop $Dialog
  
  ${If} $Dialog == error
    Abort
  ${EndIf}
  
  ${NSD_CreateLabel} 0 0 100% 12u "Install PlatformSDK Project Templates:"
  Pop $Label
  
  ${NSD_CreateCheckbox} 0 30u 100% 10u "QtCreator"
  Pop $QtCCheckbox
  ${NSD_OnClick} $QtCCheckbox QtCCheckboxOnClick
  
  ${If} $QtCCheckbox_State == ${BST_CHECKED}
    ${NSD_Check} $QtCCheckbox
  ${EndIf}
  
  ${NSD_CreateLabel} 5% 45u 60% 10u "Please enter the location of qtcreator.exe:"
  ${NSD_CreateText}} 5% 60u 60% 10u $QtEnvPath
  Pop $QtFolderText
  EnableWindow $QtFolderText 0
  
  #provide the path to the folder of the used version of Qt
  ${NSD_CreateBrowseButton} 70% 57u 20% 15u "Browse"
  Pop $QtBrowseButton
  EnableWindow $QtBrowseButton 0
  ${NSD_OnClick} $QtBrowseButton QtBrowseButtonOnclick
  
  ${NSD_OnChange} $QtFolderText QtFolderValidate
  
  ${NSD_CreateCheckbox} 5% 75u 100% 10u "Launch QtCreator at the end of the installation to update the wizards"
  Pop $QtLaunchQtCreatorCheckbox
  ${NSD_OnClick} $QtLaunchQtCreatorCheckbox QtLaunchQtCreatorCheckboxOnClick
  ${NSD_GetState} $QtLaunchQtCreatorCheckbox $QtLaunchQtCreatorCheckbox_State
  EnableWindow $QtLaunchQtCreatorCheckbox 0
  
  #${NSD_CreateCheckbox} 0 100u 100% 10u "Visual Studio"
  #Pop $VSCheckbox
  #${NSD_OnClick} $VSCheckbox VSCheckboxOnClick
  
  #${If} $VSCheckbox_State == ${BST_CHECKED}
    #${NSD_Check} $VSCheckbox
  #${EndIf}
  
  nsDialogs::Show
    
FunctionEnd

Function QtFolderValidate
  ${NSD_GetState} $QtCCheckbox $QtCCheckbox_State
  ${NSD_GetText} $QtFolderText $textBuffer
  push 0
  pop $comp
  ${If} $QtCCheckbox_State == ${BST_CHECKED}
    IfFileExists "$textBuffer\qtcreator.exe" exists notexists  
exists:
    push 1
    pop $comp    
notexists:    
  ${EndIf}  
  
  ${If} $comp == 1
    EnableWindow $InstalButton 1
  ${Else}
    EnableWindow $InstalButton 0  
  ${Endif}  
  Push $textBuffer
  Pop $QtFolder
FunctionEnd


Function QtCCheckboxOnClick
  ${NSD_GetState} $QtCCheckbox $QtCCheckbox_State
  ${If} $QtCCheckbox_State == ${BST_CHECKED}
    EnableWindow $QtBrowseButton 1
    EnableWindow $QtFolderText 1  
    EnableWindow $QtLaunchQtCreatorCheckbox 1
    ${NSD_GetText} $QtFolderText $textBuffer
    Push $textBuffer
    Pop $QtFolder
  ${Else}
    EnableWindow $QtBrowseButton 0
    EnableWindow $QtFolderText 0
   ${NSD_SetState} $QtLaunchQtCreatorCheckbox 0
    EnableWindow $QtLaunchQtCreatorCheckbox 0
    EnableWindow $InstalButton 1
  ${EndIf}
FunctionEnd

Function QtLaunchQtCreatorCheckboxOnClick
  ${NSD_GetState} $QtLaunchQtCreatorCheckbox $QtLaunchQtCreatorCheckbox_State
FunctionEnd


Function QtBrowseButtonOnclick
  nsDialogs::SelectFolderDialog "Select QtCreator bin directory..." "C:\Qt"
  Pop $QtFolder
  ${If} $QtFolder == "error"
    Goto error
  ${EndIf}
  IfFileExists "$QtFolder\qtcreator.exe" found unfound
found:
  Goto end
unfound:
  MessageBox MB_OK "$QtFolder does not contain qtcreator.exe"
end:
  ${NSD_SetText} $QtFolderText $QtFolder
error:
FunctionEnd

Function VSCheckboxOnClick
  ${NSD_GetState} $VSCheckbox $VSCheckbox_State
FunctionEnd
 
Section "install"
  #C:\Qt\Qt5.6.0\Tools\QtCreator\share\qtcreator\templates\wizards\projects\qmlproject
  # Copy files for QtCreator
  setOutPath $INSTDIR
  
  ${If} $QtCCheckbox_State == ${BST_CHECKED}
    WriteIniStr "$INSTDIR\uninstall.ini" "Uninstall" "QtFolder" $QtFolder 
    setOutPath $QtFolder\..\share\qtcreator\templates\wizards\projects
    File /r "qttemplate\"   
  ${EndIf}

  ${If} $QtLaunchQtCreatorCheckbox_State == ${BST_CHECKED}
    Exec '"$QtFolder\qtcreator.exe" -customwizard-verbose'
  ${EndIf}
      
  #Run QtCreator QtLaunchQtCreatorCheckbox  
  # Files for the install directory - to build the installer, these should be in the same directory as the install script (this file)
  setOutPath $INSTDIR
  # Files added here should be removed by the uninstaller (see section "uninstall")
  File /r '..\..\..\tempinstalldir\*.dll'
  File /r '..\..\..\tempinstalldir\epshell.exe'
  File /r '..\..\..\tempinstalldir\epviewer.exe'
  File /r '..\..\..\tempinstalldir\libep-docs.chm'
  
  WriteIniStr "$INSTDIR\uninstall.ini" "Uninstall" "QtCCheckbox_State" $QtCCheckbox_State 
  WriteIniStr "$INSTDIR\uninstall.ini" "Uninstall" "VSCheckbox_State" $VSCheckbox_State
  
  # install template for Visual Studio
#  ${If} $VSCheckbox_State == ${BST_CHECKED}
    #File 'Euclideon Platform Plugin Template.vsix'
  
    #ReadEnvStr $R0 COMSPEC
    #nsExec::Exec '$R0 /C "$INSTDIR\Eucl  ideon Platform Plugin Template.vsix"' $0
    
    #Debugger assist for VS 14.0
    ReadEnvStr $0 VS140COMNTOOLS
    ${If} $0 != error
      StrCpy $VSPath "$0..\Packages\Debugger\Visualizers"
      SetOutPath $VSPath
      File "..\..\..\bin\natvis\euclideon.*"      

      ${If} $QtCCheckbox_State == ${BST_CHECKED}
        File "..\..\..\bin\natvis\qt5.natvis"
      ${EndIf}
    ${EndIf}

  # Libs and include
  CreateDirectory $INSTDIR\include
  setOutPath $INSTDIR\include
  File /r '..\..\..\tempinstalldir\include\'
  
  CreateDirectory $INSTDIR\lib
  setOutPath $INSTDIR\lib
  File /r '..\..\..\tempinstalldir\lib\'
  
  CreateDirectory $INSTDIR\plugins
  setOutPath $INSTDIR\plugins
  File /r '..\..\..\tempinstalldir\plugins\'
    
  #file "logo.ico"
  # Add any other files for the install directory (license files, app data, etc) here
 
  # Uninstaller - See function un.onInit and section "uninstall" for configuration
  writeUninstaller "$INSTDIR\uninstall.exe"
 
  # Start Menu
  #createDirectory "$SMPROGRAMS\${COMPANYNAME}"
  #createShortCut "$SMPROGRAMS\${COMPANYNAME}\${APPNAME}.lnk" "$INSTDIR\app.exe" "" "$INSTDIR\logo.ico"
  
  # Env. Variable
  !define env_hkcu 'HKCU "Environment"'
  WriteRegExpandStr ${env_hkcu} EPDIR $INSTDIR
  # Check 
  #SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
  
  # Registry information for add/remove programs
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "DisplayName" "${COMPANYNAME} - ${APPNAME} - ${DESCRIPTION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "UninstallString" "$\"$INSTDIR\uninstall.exe$\""
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "QuietUninstallString" "$\"$INSTDIR\uninstall.exe$\" /S"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "InstallLocation" "$\"$INSTDIR$\""
  #WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "DisplayIcon" "$\"$INSTDIR\logo.ico$\""
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "Publisher" "$\"${COMPANYNAME}$\""
  #WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "HelpLink" "$\"${HELPURL}$\""
  #WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "URLUpdateInfo" "$\"${UPDATEURL}$\""
  #WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "URLInfoAbout" "$\"${ABOUTURL}$\""
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "DisplayVersion" "$\"${VERSIONMAJOR}.${VERSIONMINOR}.${VERSIONBUILD}$\""
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "VersionMajor" ${VERSIONMAJOR}
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "VersionMinor" ${VERSIONMINOR}
  # There is no option for modifying or repairing the install
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "NoRepair" 1
  # Set the INSTALLSIZE constant (!defined at the top of this script) so Add/Remove Programs can accurately report the size
  #WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "EstimatedSize" ${INSTALLSIZE}
  
SectionEnd
 
# Uninstaller
 
Function un.onInit
  SetShellVarContext all
 
  #Verify the uninstaller - last chance to back out
  MessageBox MB_OKCANCEL "Are you sure you want to remove ${APPNAME}?" IDOK next
    Abort
  next:
  !insertmacro VerifyUserIsAdmin
  
  # Remove Env. Variable
  DeleteRegValue ${env_hkcu} EPDIR
  # Check
  #SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
  
FunctionEnd
 
Section "uninstall"
 
  # Remove Start Menu launcher
  #delete "$SMPROGRAMS\${COMPANYNAME}\${APPNAME}.lnk"
  # Try to remove the Start Menu folder - this will only happen if it is empty
  #rmDir "$SMPROGRAMS\${COMPANYNAME}"

  #Remove libs and include
  RMDir /r $INSTDIR\include
  RMDir /r $INSTDIR\lib
  
  
  
# IfFileExists "C:\Program Files (x86)\Microsoft Visual Studio 12.0\*.*" found12 unfound12
#found12:
# SetOutPath "C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\Packages\Debugger\Visualizers"
# Delete "..\bin\natvis\euclideon.*"
#unfound12:
# 
# IfFileExists "C:\Program Files (x86)\Microsoft Visual Studio 11.0\*.*" found11 unfound11
#found11:
# SetOutPath "C:\Program Files (x86)\Microsoft Visual Studio 11.0\Common7\Packages\Debugger\Visualizers"
# Delete "..\bin\natvis\euclideon.*"
#unfound11:
  
  ReadINIStr $QtCCheckbox_State "$INSTDIR\uninstall.ini" "Uninstall" "QtCCheckbox_State" 
  ReadINIStr $VSCheckbox_State "$INSTDIR\uninstall.ini" "Uninstall" "VSCheckbox_State" 
  # Remove files
  #delete /r "$INSTDIR\EPDK"
  ${If} $QtCCheckbox_State == ${BST_CHECKED}
    ReadINIStr $QtFolder "$INSTDIR\uninstall.ini" "Uninstall" "QtFolder" 
    RMDir /r $QtFolder\..\share\qtcreator\templates\wizards\projects\qtshellplugin
  ${EndIf}

  #${If} $VSCheckbox_State == ${BST_CHECKED}
    #delete '$INSTDIR\Euclideon Platform Plugin Template.vsix'
    ReadEnvStr $0 VS140COMNTOOLS
    
    ${If} $0 != error
      StrCpy $VSPath "$0..\Packages\Debugger\Visualizers"
      delete "$VSPath\euclideon.*"
      ${If} $QtCCheckbox_State == ${BST_CHECKED}
        delete "$VSPath\qt5.natvis"
      ${EndIf}
    ${EndIf}
    
  #${EndIf}
  #delete "$INSTDIR\logo.ico" 
 
  delete $INSTDIR\plugins\*.dll
  rmDir $INSTDIR\plugins

  delete $INSTDIR\*.dll
  delete $INSTDIR\epshell.exe
  delete $INSTDIR\epviewer.exe
  delete $INSTDIR\libep-docs.chm

  # Always delete uninstaller as the last action
  delete $INSTDIR\uninstall.exe
  delete $INSTDIR\uninstall.ini

  # Try to remove the install directory - this will only happen if it is empty
  rmDir $INSTDIR
 
  # Remove uninstaller information from the registry
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}"
SectionEnd