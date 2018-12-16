@echo off
ECHO building LabAcme

SET current=%cd%

if not exist "prereq" mkdir prereq
cd prereq

if not exist "LabAcme\README.md" ^
git clone https://meshula@github.com/meshula/LabAcme.git

cd LabAcme
git pull
cd ..\..

if not exist "build\LabAcme" ^
mkdir build\LabAcme
cd build\LabAcme

premake5 vs2015 --file=..\..\prereq\LabAcme\Premake5.lua
msbuild ..\..\prereq\LabAcme\LabAcme.sln /t:Build /p:Configuration=Release /p:Platform=windows
msbuild ..\..\prereq\LabAcme\LabAcme.sln /t:Build /p:Configuration=Debug /p:Platform=windows

cd %current%

if not exist "include\LabAcme" ^
mkdir include\LabAcme

copy .\prereq\LabAcme\src\*.h .\include\LabAcme\
copy .\prereq\LabAcme\bin\windows\Debug\LabAcme.* .\lib\windows\Debug\
copy .\prereq\LabAcme\bin\windows\Release\LabAcme.* .\lib\windows\Release\
copy .\prereq\LabAcme\bin\windows\Debug\*.dll ..\bin\windows\Debug\
copy .\prereq\LabAcme\bin\windows\Debug\*.exe ..\bin\windows\Debug\
copy .\prereq\LabAcme\bin\windows\Release\*.dll ..\bin\windows\Release\
copy .\prereq\LabAcme\bin\windows\Release\*.exe ..\bin\windows\Release\
