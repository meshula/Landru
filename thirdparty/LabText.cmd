@echo off
ECHO building LabText

SET current=%cd%

if not exist "prereq" mkdir prereq
cd prereq

if not exist "LabText\README.md" ^
git clone https://github.com/meshula/LabText.git

cd LabText
git pull
cd ..\..

if not exist "build\LabText" ^
mkdir build\LabText
cd build\LabText

premake5 vs2015 --file=..\..\prereq\LabText\Premake5.lua
msbuild ..\..\prereq\LabText\LabText.sln /t:Build /p:Configuration=Release /p:Platform=windows
msbuild ..\..\prereq\LabText\LabText.sln /t:Build /p:Configuration=Debug /p:Platform=windows

cd %current%
xcopy .\prereq\LabText\src\*.h .\include\ /s /y
xcopy .\prereq\LabText\src\*.hpp .\include\ /s /y
