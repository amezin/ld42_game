#pragma once

#include <SDL.h>

struct Input
{
    Input();
    ~Input();

    void update();

    SDL_GameController *const controller;
    double mx, my;
    bool button_pressed;
};
