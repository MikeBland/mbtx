;NSIS Modern User Interface
;Start Menu Folder Selection Example Script
;Written by Joost Verburg

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"
  !include "FileAssociation.nsh"



;--------------------------------
;General

  ;Name and file
  Name "eePe"
  OutFile "eePeInstall.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\eePe"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\er9x-eePe" ""
  
  ;Compressor options
  SetCompressor /FINAL /SOLID lzma
  SetCompressorDictSize 64

  ;Request application privileges for Windows Vista
  RequestExecutionLevel admin

;--------------------------------
;Variables

  Var StartMenuFolder

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "license.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  
  ;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU" 
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\er9x-eePe" 
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
  
  !insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
  
  !insertmacro MUI_PAGE_INSTFILES
  
        # These indented statements modify settings for MUI_PAGE_FINISH
    !define MUI_FINISHPAGE_NOAUTOCLOSE
    !define MUI_FINISHPAGE_RUN
    !define MUI_FINISHPAGE_RUN_CHECKED
    !define MUI_FINISHPAGE_RUN_TEXT "Launch eePe"
    !define MUI_FINISHPAGE_RUN_FUNCTION "LaunchLink"
  #  !define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED
  #  !define MUI_FINISHPAGE_SHOWREADME $INSTDIR\readme.txt
  !insertmacro MUI_PAGE_FINISH
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "eePe" SecDummy

  SetOutPath "$INSTDIR"
  
  File "license.txt"
  File "eepe.exe"
  File "eepskye.exe"
  File "libgcc_s_dw2-1.dll"
  File "mingwm10.dll"
  File "QtCore4.dll"
  File "QtGui4.dll"
  File "QtNetwork4.dll"
  File "QtXml4.dll"
  File "avrdude.exe"
  File "avrdude.conf"
  File "libusb0.dll"
  File "ER9x_Manual_2015-v01.pdf"
  
  CreateDirectory "$INSTDIR\lang"
  SetOutPath "$INSTDIR\lang"
  File "*.qm"
  
  
  SetOutPath "$INSTDIR"
  ;Store installation folder
  WriteRegStr HKCU "Software\er9x-eePe" "" $INSTDIR
  
  ;Associate with extentions ,bin and .hex
  ${registerExtension} "$INSTDIR\eepe.exe" ".bin" "BIN_File"
  ${registerExtension} "$INSTDIR\eepe.exe" ".hex" "HEX_File"
  ${registerExtension} "$INSTDIR\eepe.exe" ".eepe" "EEPE_File"
  ${registerExtension} "$INSTDIR\eepe.exe" ".eepm" "EEPM_File"
  ${registerExtension} "$INSTDIR\eepe.exe" ".eepg" "EEPG_File"

  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    
    ;Create shortcuts
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
	CreateShortCut "$SMPROGRAMS\$StartMenuFolder\eePe.lnk" "$INSTDIR\eePe.exe"
	CreateShortCut "$SMPROGRAMS\$StartMenuFolder\eePskye.lnk" "$INSTDIR\eePskye.exe"
	CreateShortCut "$SMPROGRAMS\$StartMenuFolder\er9x Users Manual.lnk" "$INSTDIR\ER9x_Manual_2015-v01.pdf"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  
  !insertmacro MUI_STARTMENU_WRITE_END

SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecDummy ${LANG_ENGLISH} "eePe er9x EEPROM editor."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDummy} $(DESC_SecDummy)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END
 
;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ;ADD YOUR OWN FILES HERE...

  Delete "$INSTDIR\license.txt"
  Delete "$INSTDIR\eepe.exe"
  Delete "$INSTDIR\eepskye.exe"
  Delete "$INSTDIR\libgcc_s_dw2-1.dll"
  Delete "$INSTDIR\mingwm10.dll"
  Delete "$INSTDIR\QtCore4.dll"
  Delete "$INSTDIR\QtGui4.dll"
  Delete "$INSTDIR\QtNetwork4.dll"
  Delete "$INSTDIR\QtXml4.dll"
  Delete "$INSTDIR\avrdude.exe"
  Delete "$INSTDIR\avrdude.conf"
  Delete "$INSTDIR\libusb0.dll"
  Delete "$INSTDIR\ER9x Users Manual.pdf"
  Delete "$INSTDIR\Uninstall.exe"
  
  Delete "$INSTDIR\lang\*.*"

  RMDir "$INSTDIR\lang"
  RMDir "$INSTDIR"
  
  ${unregisterExtension} ".bin" "BIN File"
  ${unregisterExtension} ".hex" "HEX File"
  ${unregisterExtension} ".eepe" "EEPE File"
  ${unregisterExtension} ".eepm" "EEPM File"
  ${unregisterExtension} ".eepg" "EEPG File"
  
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
    
  Delete "$SMPROGRAMS\$StartMenuFolder\eePe.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\eePskye.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
  RMDir "$SMPROGRAMS\$StartMenuFolder"
  
  DeleteRegKey /ifempty HKCU "Software\er9x-eePe"

SectionEnd

Function LaunchLink
  ExecShell "" "$INSTDIR\eepe.exe"
FunctionEnd
