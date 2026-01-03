@echo off

mkdir .\build
pushd .\build
cl -FC -Zi c:\code\26_games\breakout\main.cpp user32.lib gdi32.lib
popd