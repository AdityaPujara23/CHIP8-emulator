#!/bin/bash
echo Aditya Pujara\'s CHIP8 Emulator
echo This emulator requires SDL2 to be correctly installed.
echo
echo
echo 'Enter the name of the ROM to use if its in the same path.'
echo 'Else, enter the path of the ROM to use.'
read ROM
g++ system.cpp Chip8.cpp -lSDL2main -lSDL2
./a.out $ROM
