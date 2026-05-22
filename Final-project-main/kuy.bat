@echo off
g++ -std=c++17 arena.cpp -o arena.exe -I.\SFML-3.1.0\include -L.\SFML-3.1.0\lib -lsfml-graphics -lsfml-window -lsfml-system
if %errorlevel% == 0 (
    echo Compile success! Running...
    arena.exe
) else (
    echo Compile failed.
)