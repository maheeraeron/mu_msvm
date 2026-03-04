:: Download one package with Terrapin.

:: script: terrapin.cmd ${{parameters.url}} ${{parameters.hash}} ${{parameters.path}}

:: One trailing slash on blobURL is required.
:: TODO: Automate removing any given.
:: TODO: Download and use TerrapinRetrievalTool.
::       The problem is that the 1ES nuget feeds is not in our upstreams?
::       Workaround: Curl to backend, discovered by running TerrapinRetrievalTool locally. It logs it.
@echo on
@type %~f0
mkdir %STUART_EXTDEP_CACHE_PATH%
cd    %STUART_EXTDEP_CACHE_PATH%

:: Atomicity: Download to x.temp and rename to x.
:: If x.temp exists, that means a previous run was interrupted.
:: Delete it and start over clean.
:: Reality is probably a stateless container and this does not matter.

if exist %3 goto :eof
rmdir /q/s %3.temp
mkdir %3.temp || goto :eof
cd %3.temp || goto :eof
:;
:: TODO: TerrapinRetrievalTool is not in our Nuget feed (or upstream).
::
:: %PkgTerrapinRetrievalTool%\win-x64\TerrapinRetrievalTool.exe ^
::     --blobURL %TerrapinStore%/                               ^
::     --packageURL %1                                          ^
::     --sha %2                                                 ^
::     --downloadPath .\%~nx1                                   ^
::     --allowUpload true                                       ^
::     --uploadTokenSource Environment || goto :eof

curl %TerrapinStore%/%2 --output %~nx1 || goto :eof
tar xvf %~nx1 || goto :eof
if exist basetools call :F2 || goto :eof
cd ..
ren %~nx3.temp %~nx3 || goto :eof
goto :eof

:F2
cd basetools || goto :eof
:: There might be only directories, no files, so this can fail.
:: Or maybe only files and no directories. Either way, the
:: delete of empty directory is expected to succeed.
move * ..
for /d %%a in (*) do move %%a ..
cd ..  || goto :eof
rmdir basetools || goto :eof
goto :eof
