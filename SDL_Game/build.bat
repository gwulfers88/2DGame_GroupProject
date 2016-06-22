@echo off

REM this are the compiler flags we want to use for our program.
set compilerFlags = -MTd -nologo -fp:fast -Gm- -GR- -EHa- -Od -Oi -WX -W4 -FC -Z7
set linkerFlags = -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib

IF NOT EXIST ..\build\Debug mkdir ..\build\Debug
pushd ..\build\Debug

del *.pdb > NUL 2> NUL
cl %compilerFlags% ..\..\SDL_Game\Game.cpp -FmGame.map -LD /link -incremental:no -opt:ref -PDB:game_%random%.pdb -EXPORT:UpdateRender
del lock.tmp
cl %compilerFlags% ..\..\SDL_Game\sys_win.cpp -Fmsys_win.map /link %linkerFlags%
popd