@ECHO OFF

REM =======================================
REM ===========  WINDOWS BUILD  ===========
REM =======================================
SET MSBUILD="C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe"
SET SLNFILE=..\Src\_BuildAll.sln

%MSBUILD% %SLNFILE% /m /t:Rebuild /p:Configuration=ReleaseMT;Platform=x64
IF %ERRORLEVEL% NEQ 0 GOTO Error
%MSBUILD% %SLNFILE% /m /t:Rebuild /p:Configuration=Release;Platform=x64
IF %ERRORLEVEL% NEQ 0 GOTO Error

REM =====================================
REM ===========  LINUX BUILD  ===========
REM =====================================
RD /S /Q ..\Output\LinuxRelease
mkdir ..\Output\LinuxRelease

cd ..\Output\LinuxRelease
wsl cmake ../../Src
IF %ERRORLEVEL% NEQ 0 GOTO Error
wsl cmake --build . --parallel 16
IF %ERRORLEVEL% NEQ 0 GOTO Error
cd ..\..\Build

RD /S /Q ..\Output\libarchive
mkdir ..\Output\libarchive

cd ..\Output\libarchive
wsl cmake ../../Src/libarchive
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