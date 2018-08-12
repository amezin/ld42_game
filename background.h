#pragma once

#include <random>
#include <vector>

#include <SDL.h>

struct Background
{
    struct Particle
    {
        double x;
        double speed;

        double angle;
        double rotation_speed;

        int y;
        int size;
        SDL_Color color;
        SDL_Texture *texture;
    };

    Background(SDL_Renderer *, std::random_device &);
    ~Background();

    void update();
    void draw();
    void draw_border();

    void gen_star(Particle &);
    void gen_fog(Particle &);

    SDL_Renderer *const renderer;
    SDL_Texture *const star_texture, *const fog_texture, *const border_texture;
    int game_w, game_h;
    std::vector<Particle> particles;
    std::mt19937 mt;
};
