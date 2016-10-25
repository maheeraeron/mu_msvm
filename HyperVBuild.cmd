@echo off
setlocal enableextensions enabledelayedexpansion

rem
rem This script automatically copies UEFI symbols from the secure depot to VM depot for check-in.
rem it will also copy the firmware image as UEFIBIOS.BIN.
rem

echo UEFI Build Helper

if NOT DEFINED WORKSPACE (
  echo.
  echo Error --- Cannot find UEFI build Workspace
  echo This utility must be run from within a UEFI Workspace
  echo Run edk2setup.bat and try again.
  echo.
  exit /b 1
)

set _This=%~n0

rem ----------------------------------
:ParseArgs

set _UpdateConf=1

if "%1"=="" (
  goto :ParseArgsComplete
) else if "%1" EQU "/h" (
  goto :Usage
) else if "%1" EQU "/?" (
  goto :Usage
) else if /I "%1" EQU "/verbose" (
  echo on
) else if /I "%1" EQU "/target" (
  set _Target=%~2
  shift /0
) else if /I "%1" EQU "/buildopt" (
  set _BuildOptions=%~2
  shift /0
) else (
  echo Ignoring unknown parameter [%1]
)

shift /0
goto :ParseArgs

:ParseArgsComplete

if "%_Target%"=="" (
  set _Target=DEBUG RELEASE
  echo.
  echo No build flavor specified, defaulting to !_Target!
  echo.
)

set _UEFI_ROOT=%WORKSPACE%\build\msvmx64

rem ----------------------------------
rem Update build configuration first if needed
rem
if DEFINED _UpdateConf (
  echo.
  echo Updating build configuration...
  echo.
  rd /s /q %WORKSPACE%\conf
  call %WORKSPACE%\edk2setup.bat --reconfig
  @rem edksetup likes to turn echo back on...
  @echo off
)

rem ----------------------------------
rem Process build targets one at a time.

for %%A in (!_Target!) do (
  call :BuildTarget %%A || exit /b !errorlevel!
)

goto :EOF

rem ----------------------------------
rem  Build Target Helper
rem  Performs the Build and Publish steps for a build target
rem  %1 - Target
rem
:BuildTarget

if "%1"=="DEBUG" (
 set _nt_Flavor=Chk
) else if "%1"=="RELEASE" (
 set _nt_Flavor=Fre
) else (
  echo.
  echo Error! Unknown build type "%1".  DEBUG or RELEASE are supported.
  exit /b 2
)

set _Object_Root=%1_NTDEV
set _symSavePath=%SDXROOT%\vm\dv\bios\vdev\sym\%_nt_Flavor%
set _FW_Source=%_UEFI_ROOT%\%_Object_Root%\FV\MSVM.fd
set _FW_DestName=%SDXROOT%\vm\dv\bios\vdev\UEFIBIOS%_nt_Flavor%.bin

rem ----------------------------------
rem Clean Build
rem

  echo.
  echo Performing Clean %1 build...
  echo.
  if EXIST %_UEFI_ROOT%\%_Object_Root%\ (
    echo   Removing output %_UEFI_ROOT%\%_Object_Root%\
    rd /s /q %_UEFI_ROOT%\%_Object_Root%\>NUL || echo Error cleaning output && exit /b !errorlevel!
  )

  pushd %WORKSPACE%
  build --buildtarget=%1 !_BuildOptions! 1>build%1.log

  if "!errorlevel!" NEQ "0" (
    echo.
    echo Build did not complete successfully.
    echo See build%1.log for complete build output.
    echo.
    popd
    exit /b 5
  )
  popd
  echo Build Successfull.
  echo Firmware Image: [%_FW_Source%]
  echo.

goto :EOF
rem
rem End of main script
rem ----------------------------------


rem ----------------------------------
rem Usage Helper
rem
:Usage
echo.
echo Helper script to assist in build and publishing UEFI symbols and firmware image from
echo the UEFI workspace to the VM depot.
echo.
echo Usage: %_This% [/target DEBUG ^| RELEASE] [/?] [/verbose]
echo.
echo Options:
echo    /target     - Build target to use for build and/or publishing.
echo                  Multiple targets can be specified and seperated by spaces or commas
echo                  Supported Targets are DEBUG and RELEASE
echo                  Default is DEBUG and RELEASE.
echo    /verbose    - turns on CMD echo and generally make a mess of the output.
echo                  Usefull for debugging script issues.
echo.
echo Examples:
echo   %_This%
echo.
echo   - Performs a full clean build
echo.
echo   %_This% /target DEBUG
echo.
echo   - Performs a clean build of DEBUG target
echo.
goto :EOF
rem ----------------------------------



