@echo off
ECHO building LabRender

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

xcopy .\prereq\LabRender\include\*.h .\include\ /s /y
copy .\prereq\LabRender\bin\windows\Debug\labrender.* .\lib\windows\Debug\
copy .\prereq\LabRender\bin\windows\Release\labrender.* .\lib\windows\Release\
copy .\prereq\LabRender\bin\windows\Debug\*.dll ..\bin\windows\Debug\
copy .\prereq\LabRender\bin\windows\Debug\*.exe ..\bin\windows\Debug\
copy .\prereq\LabRender\bin\windows\Release\*.dll ..\bin\windows\Release\
copy .\prereq\LabRender\bin\windows\Release\*.exe ..\bin\windows\Release\
