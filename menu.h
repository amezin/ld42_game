#pragma once

#include <SDL.h>

#include "input.h"

struct Menu
{
    Menu(SDL_Renderer *, Input &);
    ~Menu();

    void draw();

    SDL_Renderer *const renderer;
    Input &input;
    SDL_Texture *menu_texture;
};
