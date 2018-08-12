#pragma once

#include <vector>
#include <random>
#include <list>

#include <SDL.h>

#include "input.h"

struct Game
{
    Game(SDL_Renderer *, Input &, std::random_device &);
    ~Game();

    void update(double scale);
    void draw();

    SDL_Rect chuvak_rect() const;

    SDL_Renderer *const renderer;
    Input &input;
    std::mt19937 mt;
    SDL_Texture *const chuvak1;
    SDL_Texture *const chuvak15;
    SDL_Texture *const chuvak2;
    SDL_Texture *const chuvak25;
    SDL_Texture *const asteroid;
    double chuvak_anim;
    double chuvak_x, chuvak_y;
    double chuvak_speed_x, chuvak_speed_y;
    int game_w, game_h;
    bool dead;
    double asteroid_spawn_delay;

    struct BloodParticle
    {
        double x, y, speed_x, speed_y;
    };
    std::vector<BloodParticle> blood;

    struct Asteroid
    {
        double x;
        double speed;

        double angle;
        double rotation_speed;

        int y;
        int size;
        SDL_Color color;
    };

    std::list<Asteroid> asteroids;
    void gen_asteroid();

    double play_time;
};
