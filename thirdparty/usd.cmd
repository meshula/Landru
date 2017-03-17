@echo off
ECHO installing USD

SET current=%cd%

set dbg=false
if "%~1"=="debug" (set dbg=true)

IF NOT "%dbg%"=="true" GOTO Rel

cd lib\windows\debug
copy c:\Projects\usd\stage\local\lib\*.lib
cd ..\..\..\..\bin\windows\debug

rem for now, include the USD dlls in the PATH at runtime instead of copying
rem copy c:\Projects\usd\stage\local\bin\*.dll
rem copy c:\Projects\usd\stage\local\lib\*.dll
cd ..\..\..\thirdparty

goto Finish

:Rel

cd lib\windows\release
copy c:\Projects\usd\stage\local\lib\*.lib
cd ..\..\..\..\bin\windows\release

rem for now, include the USD dlls in the PATH at runtime instead of copying
rem copy c:\Projects\usd\stage\local\bin\*.dll
rem copy c:\Projects\usd\stage\local\lib\*.dll

cd ..\..\..\thirdparty

:Finish

xcopy c:\Projects\usd\stage\local\include include /s /e
