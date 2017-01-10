
ECHO building stb

SET current=%cd%

if not exist "prereq" mkdir prereq
cd prereq

if not exist "stb\README.md" ^
git clone https://github.com/nothings/stb.git

cd stb
git pull
cd ..

cd %current%

if not exist "local\include\stb" mkdir local\include\stb

copy "prereq\stb\*.h" "local\include\stb"
