@echo off

IF NOT EXIST ..\build\Debug mkdir ..\build\Debug
pushd ..\build\Debug

REM this are the compiler flags we want to use for our program.
set compilerFlags= -MTd -nologo -Gm- -GR- -EHa- -Od -Oi -W4 -wd4201 -wd4100 -wd4189 -wd4505 -FC -Z7
set linkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib

del *.pdb > NUL 2> NUL

echo WAITING FOR PDB > lock.tmp
cl %compilerFlags% ..\..\SDL_Game\Game.cpp -FmGame.map -LD /link -incremental:no -opt:ref /PDB:Game_%random%.pdb /EXPORT:UpdateRender
del lock.tmp
cl %compilerFlags% ..\..\SDL_Game\sys_win.cpp -Fmsys_win.map /link %linkerFlags% /PDB:sys_win.pdb

popd