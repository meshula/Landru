@echo off
ECHO building LabCmd

SET current=%cd%

if not exist "prereq" mkdir prereq
cd prereq

if not exist "LabCmd\README.md" ^
git clone https://github.com/meshula/LabCmd.git

cd LabCmd
git pull
cd ..\..

if not exist "build\LabCmd" ^
mkdir build\LabCmd
cd build\LabCmd

premake5 vs2015 --file=..\..\prereq\LabCmd\Premake5.lua
msbuild ..\..\prereq\LabCmd\LabCmd.sln /t:Build /p:Configuration=Release /p:Platform=windows
msbuild ..\..\prereq\LabCmd\LabCmd.sln /t:Build /p:Configuration=Debug /p:Platform=windows

cd %current%
