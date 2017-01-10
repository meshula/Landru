
@echo off
ECHO building OpenVR

SET current=%cd%

if not exist "prereq" mkdir prereq
cd prereq

if not exist "openvr\README.md" ^
git clone https://github.com/ValveSoftware/openvr.git

cd openvr
git pull

cd %current%
