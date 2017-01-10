@echo off
ECHO building LabSound

SET current=%cd%

if not exist "prereq" mkdir prereq
cd prereq

if not exist "LabSound\README.md" ^
git clone https://meshula@github.com/meshula/LabSound.git --recursive

cd LabSound
git pull
cd ..\..

if not exist "build\LabSound" ^
mkdir build\LabSound
cd build\LabSound

msbuild ..\..\prereq\LabSound\win.vs2015\LabSound.sln /t:Build /p:Configuration=Release /p:Platform=x64
msbuild ..\..\prereq\LabSound\win.vs2015\LabSound.sln /t:Build /p:Configuration=Debug /p:Platform=x64

cd %current%
