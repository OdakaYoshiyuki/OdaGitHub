@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"

cd %~dp0


MSBuild ..\LogTool\LogTool.sln /p:Configuration=Debug /p:Platform=Win32 /t:Rebuild


pause



rem ���Ȃ��̊��� Visual Studio 2022 Community / Professional / Enterprise �̂ǂ���g���Ă��܂����H

