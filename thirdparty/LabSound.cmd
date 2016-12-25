@echo off
ECHO building LabSound

SET current=%cd%

if not exist "src" mkdir src
cd src

if not exist "LabSound\README.md" ^
git clone https://github.com/LabSound/LabSound.git --recursive

cd LabSound
git pull
cd ..\..

if not exist "build\LabSound" ^
mkdir build\LabSound
cd build\LabSound

msbuild ..\..\src\LabSound\win.vs2015\LabSound.sln /t:Build /p:Configuration=Release /p:Platform=x64
msbuild ..\..\src\LabSound\win.vs2015\LabSound.sln /t:Build /p:Configuration=Debug /p:Platform=x64

cd %current%
