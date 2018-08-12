#include <list>
#include <memory>
#include <random>

#include <SDL.h>
#include <SDL_image.h>

#include "background.h"
#include "input.h"
#include "game.h"
#include "menu.h"

static const int game_w = 1366;
static const int game_h = 768;

int main()
{
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);

    SDL_Init(SDL_INIT_EVERYTHING);
    atexit(SDL_Quit);

    IMG_Init(IMG_INIT_PNG);
    atexit(IMG_Quit);

    auto window = SDL_CreateWindow("Chuvak Is Running Out Of Space",
                                   SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED,
                                   game_w,
                                   game_h,
                                   SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    auto renderer = SDL_CreateRenderer(window,
                                       -1,
                                       SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    SDL_RenderSetLogicalSize(renderer, game_w, game_h);

    std::random_device rd;
    Background background(renderer, rd);
    Input input;
    Menu menu(renderer, input);
    std::unique_ptr<Game> game;

    SDL_Event event{};
    while (event.type != SDL_QUIT) {
        if (SDL_PollEvent(&event)) {
            switch (event.type) {
            }
        } else {
            input.update();
            background.update();

            if (game) {
                for (int i = 0; i < 50; i++) {
                    game->update(0.02);
                }
            }

            if (!game || game->dead) {
                if (input.button_pressed) {
                    game.reset(new Game(renderer, input, rd));
                }
            }

            background.draw();

            if (game) {
                game->draw();
            }

            if (!game || game->dead) {
                menu.draw();
            }

            background.draw_border();
            SDL_RenderPresent(renderer);
        }
    }

    return 0;
}
