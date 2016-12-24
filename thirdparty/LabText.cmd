@echo off
ECHO building LabText

SET current=%cd%

if not exist "src" mkdir src
cd src

if not exist "LabText\README.md" ^
git clone https://github.com/meshula/LabText.git

cd LabText
git pull
cd ..\..

if not exist "build\LabText" ^
mkdir build\LabText
cd build\LabText

premake5 vs2015 --file=..\..\src\LabText\Premake5.lua
msbuild ..\..\src\LabText\LabText.sln /t:Build /p:Configuration=Release /p:Platform=windows
msbuild ..\..\src\LabText\LabText.sln /t:Build /p:Configuration=Debug /p:Platform=windows

cd %current%
