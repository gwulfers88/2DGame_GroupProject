@echo off

IF NOT EXIST ..\build\Debug mkdir ..\build\Debug
pushd ..\build\Debug

REM this are the compiler flags we want to use for our program.
set compilerFlags = -MTd -nologo -fp:fast -Gm- -GR- -EHa- -Od -Oi -W4 -FC -Zi
set linkerFlags = /DEBUG -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib

del *.pdb > NUL 2> NUL
cl %compilerFlags% ..\..\SDL_Game\Game.cpp -FmGame.map -LD /link /DEBUG -incremental:no -opt:ref -PDB:game_%random%.pdb -EXPORT:UpdateRender
del lock.tmp
cl %compilerFlags% ..\..\SDL_Game\sys_win.cpp -FmSDL_Game.map -FoSDL_Game.obj /link /DEBUG %linkerFlags% -PDB:SDL_Game.pdb
popd