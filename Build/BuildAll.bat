@ECHO OFF

SET MSBUILD="C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
SET SLNFILE=..\Src\_BuildAll.sln


%MSBUILD% %SLNFILE% /m /t:Rebuild /p:Configuration=ReleaseMT;Platform=x64
IF %ERRORLEVEL% NEQ 0 GOTO Error
%MSBUILD% %SLNFILE% /m /t:Rebuild /p:Configuration=Debug;Platform=x64
IF %ERRORLEVEL% NEQ 0 GOTO Error
%MSBUILD% %SLNFILE% /m /t:Rebuild /p:Configuration=Release;Platform=x64
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