@echo off

:: Look here for compiler flags documentation https://docs.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-alphabetically?view=msvc-160

:: DEVELOPER 64
set BuildCompilerFlags=-DDEVELOPER -MT -nologo -Gm- -GR- -EHsca- -Oi -W4 -wd4201 -wd4100 -wd4505 -wd4996 -Zi -I ../libs/
set BuildLinkerFlags=-opt:ref -DEBUG -PDB:game.pdb user32.lib gdi32.lib ../libs/glew/64bit/glew32s.lib opengl32.lib

:: DEVELOPER 32
@REM set BuildCompilerFlags=-DDEVELOPER -MT -nologo -Gm- -GR- -EHsca- -Oi -W4 -wd4201 -wd4100 -wd4505 -wd4996 -Zi -I ../libs/
@REM set BuildLinkerFlags=-opt:ref -DEBUG -PDB:game.pdb user32.lib gdi32.lib ../libs/glew/32bit/glew32s.lib opengl32.lib

:: RELEASE 32
@REM set BuildCompilerFlags=-MT -nologo -Gm- -GR- -EHsca- -Oi -O2 -W4 -wd4201 -wd4100 -wd4505 -wd4996 -Zi -I ../libs/
@REM set BuildLinkerFlags=-opt:ref user32.lib gdi32.lib ../libs/glew/32bit/glew32s.lib opengl32.lib

:: RELEASE 64
@REM set BuildCompilerFlags=-MT -nologo -Gm- -GR- -EHsca- -Oi -O2 -W4 -wd4201 -wd4100 -wd4505 -wd4996 -Zi -I ../libs/
@REM set BuildLinkerFlags=-opt:ref user32.lib gdi32.lib ../libs/glew/64bit/glew32s.lib opengl32.lib

pushd %~dp0%
cd ..
if not exist build mkdir build
cd build

:: 32 bit build
:: cl %BuildCompilerFlags% ..\src\unity.cpp /link -subsystem:windows,5.01 %BuildLinkerFlags%

cl /Fe"game" %BuildCompilerFlags% ..\src\unity.cpp /link %BuildLinkerFlags%

popd
