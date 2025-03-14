@echo off

set commonCompilerFlags= -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -FC -Zi -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -Fmwin32_handmade.map

set commonLinkerFlags= user32.lib gdi32.lib winmm.lib 

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build
REM 32-bit build
REM cl %commonCompilerFlags ..\code\win32_handmade.cpp /link %commonLinkerFlags

REM 64 bit build
cl %commonCompilerFlags% ..\code\win32_handmade.cpp %commonLinkerFlags% /std:c++17
popd
