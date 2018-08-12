#include "game.h"

#include <algorithm>

#include <SDL_image.h>

static const int chuvak_w = 50;
static const int chuvak_h = 100;

Game::Game(SDL_Renderer *renderer, Input &input, std::random_device &rd)
    : renderer(renderer),
      input(input),
      mt(rd()),
      chuvak1(IMG_LoadTexture(renderer, "data/chuvak1.png")),
      chuvak15(IMG_LoadTexture(renderer, "data/chuvak15.png")),
      chuvak2(IMG_LoadTexture(renderer, "data/chuvak2.png")),
      chuvak25(IMG_LoadTexture(renderer, "data/chuvak25.png")),
      asteroid(IMG_LoadTexture(renderer, "data/asteroid1.png")),
      ufo(IMG_LoadTexture(renderer, "data/ufo.png")),
      chuvak_anim(0),
      chuvak_speed_x(0.0),
      chuvak_speed_y(0.0),
      dead(false),
      asteroid_spawn_delay(0.0),
      play_time(0.0)
{
    SDL_RenderGetLogicalSize(renderer, &game_w, &game_h);
    chuvak_x = (game_w - chuvak_w) * 0.5;
    chuvak_y = (game_h - chuvak_h) * 0.5;
}

Game::~Game()
{
    SDL_DestroyTexture(chuvak1);
    SDL_DestroyTexture(chuvak2);
    SDL_DestroyTexture(chuvak15);
    SDL_DestroyTexture(chuvak25);
    SDL_DestroyTexture(asteroid);
}

static double decelerate(double value, double deceleration)
{
    if (value > 0) {
        return std::max(0.0, value - deceleration);
    }
    if (value < 0) {
        return std::min(0.0, value + deceleration);
    }
    return 0.0;
}

void Game::update(double scale)
{
    for (auto &p : blood) {
        if (p.y < game_h) {
            p.x += p.speed_x * scale;
            p.y += p.speed_y * scale;
            p.speed_y += scale;
        }
    }

    for (auto i = asteroids.begin(); i != asteroids.end();) {
        i->angle += i->rotation_speed * scale;
        while (i->angle < 0.0) i->angle += 360.0;
        while (i->angle > 720.0) i->angle -= 360.0;

        i->x -= i->speed * scale;
        if (i->x < -i->size) {
            i = asteroids.erase(i);
        } else {
            i++;
        }
    }

    asteroid_spawn_delay -= scale;
    if (asteroid_spawn_delay <= 0.0) {
        std::uniform_real_distribution<double> next_spawn(1.0, 1000.0 / std::log(3.0 + play_time));
        asteroid_spawn_delay += next_spawn(mt);

        std::uniform_int_distribution<int> ufo_prob(0, 15);
        if (ufo_prob(mt) == 0) {
            gen_ufo();
        } else {
            gen_asteroid();
        }
    }

    if (dead) {
        return;
    }

    for (auto &a : asteroids) {
        if (a.homing) {
            double chuvak_center_y = chuvak_y + chuvak_h * 0.5;
            double asteroid_center_y = a.y + a.size * 0.5;
            double d = chuvak_center_y - asteroid_center_y;
            double scaled_speed = a.speed * scale * 0.1;
            if (d > scaled_speed) d = scaled_speed;
            if (d < -scaled_speed) d = -scaled_speed;
            a.y += d;
        }
    }

    play_time += scale;

    chuvak_anim += scale * 0.3;
    while (chuvak_anim > 2.0) chuvak_anim -= 2.0;

    chuvak_x += chuvak_speed_x * scale;
    chuvak_y += chuvak_speed_y * scale;
    chuvak_speed_x += input.mx * 0.4 * scale;
    chuvak_speed_y += input.my * 0.4 * scale;

    static const double deceleration = 0.025;
    chuvak_speed_x = decelerate(chuvak_speed_x, deceleration * scale);
    chuvak_speed_y = decelerate(chuvak_speed_y, deceleration * scale);

    for (auto &a : asteroids) {
        double chuvak_center_x = chuvak_x + chuvak_w * 0.5;
        double chuvak_center_y = chuvak_y + chuvak_h * 0.5;
        double asteroid_center_x = a.x + a.size * 0.5;
        double asteroid_center_y = a.y + a.size * 0.5;
        double dx = chuvak_center_x - asteroid_center_x;
        double dy = chuvak_center_y - asteroid_center_y;
        double d_sq = dx * dx + dy * dy;
        double d_min = ((chuvak_w + chuvak_h) * 0.5 + a.size) * 0.5;
        if (d_sq < d_min * d_min) {
            double d = sqrt(d_sq);
            double off = d_min - d;
            double dx_norm = dx / d;
            double dy_norm = dy / d;
            chuvak_x += dx_norm * off;
            chuvak_y += dy_norm * off;

            chuvak_speed_x += dx_norm * a.speed;
            chuvak_speed_y += dy_norm * a.speed;
        }
    }

    if (chuvak_x > game_w - chuvak_w) {
        chuvak_x = game_w - chuvak_w;
        chuvak_speed_x = std::min(0.0, chuvak_speed_x);
    }
    if (chuvak_y < 0.0) {
        chuvak_y = 0.0;
        chuvak_speed_y = std::max(0.0, chuvak_speed_y);
    }
    if (chuvak_y > game_h - chuvak_h) {
        chuvak_y = game_h - chuvak_h;
        chuvak_speed_y = std::min(0.0, chuvak_speed_y);
    }

    if (chuvak_x < -chuvak_w) {
        dead = true;

        std::uniform_real_distribution<double> speed_x(1.0, 10.0);
        std::uniform_real_distribution<double> speed_y(-20.0, 20.0);

        for (int i = 0; i < 150; i++) {
            BloodParticle p;
            p.y = chuvak_y + chuvak_h * 0.5;
            p.x = 0;
            p.speed_x = speed_x(mt);
            p.speed_y = speed_y(mt);
            blood.push_back(p);
        }
    }
}

SDL_Rect Game::chuvak_rect() const
{
    return { int(chuvak_x), int(chuvak_y), chuvak_w, chuvak_h };
}

void Game::draw()
{
    SDL_Texture *texture = nullptr;
    if (chuvak_anim > 1.5) {
        texture = chuvak25;
    } else if (chuvak_anim > 1.0) {
        texture = chuvak2;
    } else if (chuvak_anim > 0.5) {
        texture = chuvak15;
    } else {
        texture = chuvak1;
    }
    auto rect = chuvak_rect();
    SDL_RenderCopy(renderer, texture, nullptr, &rect);

    for (auto &a : asteroids) {
        SDL_SetTextureColorMod(a.texture, a.color.r, a.color.g, a.color.b);

        SDL_Rect rect { int(a.x), int(a.y), a.size, a.size };
        SDL_RenderCopyEx(renderer, a.texture, nullptr, &rect, a.angle, nullptr, SDL_FLIP_NONE);
    }

    for (auto &p : blood) {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_Rect rect { int(p.x), int(p.y), 5, 5 };
        SDL_RenderFillRect(renderer, &rect);
    }
}

void Game::gen_asteroid()
{
    std::uniform_int_distribution<int> particle_y(0, game_h);
    std::uniform_real_distribution<double> particle_angle(0.0, 360.0);

    std::uniform_int_distribution<int> asteroid_size(32, 320);
    std::uniform_int_distribution<Uint8> asteroid_color(127);
    std::uniform_real_distribution<double> asteroid_speed(5.0, 25.0);
    std::uniform_real_distribution<double> asteroid_rotation_speed(-0.5, 0.5);

    Asteroid a;
    a.x = game_w;
    a.size = asteroid_size(mt);
    a.y = particle_y(mt) - a.size / 2;
    a.color.a = 255;
    a.color.r = asteroid_color(mt);
    a.color.g = asteroid_color(mt);
    a.color.b = asteroid_color(mt);
    a.angle = particle_angle(mt);
    a.speed = asteroid_speed(mt);
    a.rotation_speed = asteroid_rotation_speed(mt);
    a.texture = asteroid;
    a.homing = false;
    asteroids.push_back(a);
}

void Game::gen_ufo()
{
    std::uniform_int_distribution<int> particle_y(0, game_h);

    std::uniform_int_distribution<int> asteroid_size(32, 128);
    std::uniform_int_distribution<Uint8> asteroid_color(200);
    std::uniform_real_distribution<double> asteroid_speed(10.0, 25.0);

    Asteroid a;
    a.x = game_w;
    a.size = asteroid_size(mt);
    a.y = particle_y(mt) - a.size / 2;
    a.color.a = 255;
    a.color.r = asteroid_color(mt);
    a.color.g = asteroid_color(mt);
    a.color.b = asteroid_color(mt);
    a.angle = 0.0;
    a.speed = asteroid_speed(mt);
    a.rotation_speed = 0.0;
    a.texture = ufo;
    a.homing = true;
    asteroids.push_back(a);
}
