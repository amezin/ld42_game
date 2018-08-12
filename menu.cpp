#include "menu.h"

#include <SDL_image.h>

Menu::Menu(SDL_Renderer *renderer, Input &input)
    : renderer(renderer), input(input), menu_texture(IMG_LoadTexture(renderer, "data/welcome.png"))
{
}

Menu::~Menu()
{
    SDL_DestroyTexture(menu_texture);
}

void Menu::draw()
{
    SDL_RenderCopy(renderer, menu_texture, nullptr, nullptr);
}
