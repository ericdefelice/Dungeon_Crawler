@echo off

set CommonCompilerFlags=-MTd -nologo -fp:fast -Gm- -GR- -EHa- -Od -Oi -EHsc -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4005 -DBUILD_DEBUG=1 -DBUILD_WIN32=1 -FC -Z7
set CommonLinkerFlags= -incremental:no -opt:ref


REM 64-bit build
del *.pdb > NUL 2> NUL
REM Optimization switches /O2 /Oi /fp:fast

REM -- BUILD THE GAME ENGINE --
cl %CommonCompilerFlags%  obj_to_gmd_conv.cpp -Feobj_to_gmd_conv.exe /link %CommonLinkerFlags%

popd