@echo off
ECHO building LabJson

SET current=%cd%

if not exist "src" mkdir src
cd src

if not exist "LabJson\README.md" ^
git clone https://github.com/meshula/LabJson.git

cd LabJson
git pull
cd ..\..

if not exist "build\LabJson" ^
mkdir build\LabJson
cd build\LabJson

premake5 vs2015 --file=..\..\src\LabJson\Premake5.lua
msbuild ..\..\src\LabJson\LabJson.sln /t:Build /p:Configuration=Release /p:Platform=windows
msbuild ..\..\src\LabJson\LabJson.sln /t:Build /p:Configuration=Debug /p:Platform=windows

cd %current%
