@echo off

REM
REM  To run this at startup, use this as your shortcut target:
REM  %windir%\system32\cmd.exe /k w:\Pixel_RE\misc\shell.bat
REM

call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64

set LIB=%LIB%;C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Lib\x64
