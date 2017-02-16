
if not exist "lib\windows\debug" mkdir lib\windows\debug
if not exist "lib\windows\release" mkdir lib\windows\release

if not exist "include\LabCmd" mkdir include\LabCmd
copy /y .\prereq\LabCmd\src\*.h .\include\LabCmd\
copy /y .\prereq\LabCmd\bin\windows\debug\*.* .\lib\windows\debug
copy /y .\prereq\LabCmd\bin\windows\release\*.* .\lib\windows\release
copy /y .\prereq\LabCmd\bin\windows\debug\*.dll ..\bin\windows\debug
copy /y .\prereq\LabCmd\bin\windows\release\*.dll ..\bin\windows\release

#if not exist "include\LabRender" mkdir include\LabRender
copy /y .\prereq\LabRender\include\LabRender\*.h .\include\LabRender\
copy /y .\prereq\LabRender\bin\windows\debug\*.* .\lib\windows\debug
copy /y .\prereq\LabRender\bin\windows\release\*.* .\lib\windows\release
copy /y .\prereq\LabRender\bin\windows\debug\*.dll ..\bin\windows\debug
copy /y .\prereq\LabRender\bin\windows\release\*.dll ..\bin\windows\release
copy /y .\prereq\LabRender\bin\windows\debug\*.exe ..\bin\windows\debug
copy /y .\prereq\LabRender\bin\windows\release\*.exe ..\bin\windows\release
