@ECHO OFF

REM =====================================
REM ===========  LINUX BUILD  ===========
REM =====================================
RD /S /Q .\LinuxRelease
RD /S /Q ..\Output\LinuxRelease
mkdir ..\Output\LinuxRelease

cd ..\Output\LinuxRelease
wsl cmake -DCMAKE_BUILD_TYPE=Debug ../../Src
IF %ERRORLEVEL% NEQ 0 GOTO Error
wsl cmake --build . --parallel 16
IF %ERRORLEVEL% NEQ 0 GOTO Error


REM ===============================
REM ===========  Final  ===========
REM ===============================
:Success
echo Build succeeded
exit /b 0

:Error
echo Error occurred, code:%ERRORLEVEL%
pause
exit /b -1