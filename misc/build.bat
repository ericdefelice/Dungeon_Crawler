@echo off

set CommonCompilerFlags=-MTd -nologo -fp:fast -Gm- -GR- -EHa- -Od -Oi -EHsc -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4005 -wd4238 -DBUILD_DEBUG=1 -DBUILD_WIN32=1 -FC -Z7
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

REM 64-bit build
del *.pdb > NUL 2> NUL
REM Optimization switches /O2 /Oi /fp:fast

REM -- BUILD THE MAIN GAME CODE INTO A DLL --
REM cl %CommonCompilerFlags% -I "..\engine\core\inc" -I "..\game\inc" ..\game\src\game_main.cpp -FmpixelRE_game.map -LD /link -incremental:no -opt:ref -PDB:pixelRE_%random%.pdb -EXPORT:GameGetSoundSamples -EXPORT:GameUpdateAndRender

REM -- BUILD THE GAME ENGINE --
cl %CommonCompilerFlags% -I "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include" -I "..\engine\common" -I "..\engine\core\inc" -I "..\game\inc" -I "..\game\src" ..\engine\core\src\win32_main.cpp -Fmdungeon_crawl.map -Fodungeon_crawl.obj -Fedungeon_crawl.exe /link %CommonLinkerFlags%

popd
