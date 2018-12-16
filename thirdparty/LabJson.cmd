@echo off
ECHO building LabJson

SET current=%cd%

if not exist "prereq" mkdir prereq
cd prereq

if not exist "LabJson\README.md" ^
git clone https://github.com/meshula/LabJson.git

cd LabJson
git pull
cd ..\..

if not exist "build\LabJson" ^
mkdir build\LabJson
cd build\LabJson

premake5 vs2015 --file=..\..\prereq\LabJson\Premake5.lua
msbuild ..\..\prereq\LabJson\LabJson.sln /t:Build /p:Configuration=Release /p:Platform=windows
msbuild ..\..\prereq\LabJson\LabJson.sln /t:Build /p:Configuration=Debug /p:Platform=windows

cd %current%
