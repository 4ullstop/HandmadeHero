@echo off

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build
cl -nologo -Gm- -GR- -EHa- -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -FC -Zi -Fmwin32_handmade.map ..\code\win32_handmade.cpp  user32.lib gdi32.lib /std:c++17
popd
