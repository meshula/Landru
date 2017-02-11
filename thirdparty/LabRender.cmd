@echo off
ECHO building LabText

SET current=%cd%

if not exist "prereq" mkdir prereq
cd prereq

if not exist "LabRender\README.md" ^
git clone https://meshula@bitbucket.org/meshula/labrender.git

cd LabRender
git pull
cd ..\..

if not exist "build\LabRender" ^
mkdir build\LabRender
cd build\LabRender

premake5 vs2015 --file=..\..\prereq\LabRender\Premake5.lua
msbuild ..\..\prereq\LabRender\LabRender.sln /t:Build /p:Configuration=Release /p:Platform=windows
msbuild ..\..\prereq\LabRender\LabRender.sln /t:Build /p:Configuration=Debug /p:Platform=windows

cd %current%
