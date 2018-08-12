#include "background.h"

#include <SDL_image.h>

Background::Background(SDL_Renderer *renderer, std::random_device &rd)
    : renderer(renderer),
      star_texture(IMG_LoadTexture(renderer, "data/star.png")),
      fog_texture(IMG_LoadTexture(renderer, "data/fog.png")),
      border_texture(IMG_LoadTexture(renderer, "data/border.png")),
      mt(rd())
{
    SDL_SetTextureBlendMode(star_texture, SDL_BLENDMODE_ADD);
    SDL_SetTextureBlendMode(fog_texture, SDL_BLENDMODE_ADD);
    SDL_RenderGetLogicalSize(renderer, &game_w, &game_h);

    std::uniform_real_distribution<double> particle_x(0.0, game_w);

    for (int i = 0; i < 100; i++) {
        Particle star;
        gen_star(star);
        star.x = particle_x(mt);
        particles.push_back(star);
    }

    for (int i = 0; i < 25; i++) {
        Particle fog;
        gen_fog(fog);
        fog.x = particle_x(mt);
        particles.push_back(fog);
    }
}

Background::~Background()
{
    SDL_DestroyTexture(star_texture);
    SDL_DestroyTexture(fog_texture);
    SDL_DestroyTexture(border_texture);
}

void Background::update()
{
    for (auto &p : particles) {
        p.angle += p.rotation_speed;
        while (p.angle > 720.0) p.angle -= 360.0;
        while (p.angle < 0.0) p.angle += 360.0;

        p.x -= p.speed;
        if (p.x < -p.size) {
            if (p.texture == fog_texture) {
                gen_fog(p);
            } else {
                gen_star(p);
            }
        }
    }
}

void Background::draw()
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (auto &p : particles) {
        SDL_SetTextureColorMod(p.texture, p.color.r, p.color.g, p.color.b);

        SDL_Rect rect { int(p.x), p.y, p.size, p.size };
        SDL_RenderCopyEx(renderer, p.texture, nullptr, &rect, p.angle, nullptr, SDL_FLIP_NONE);
    }
}

void Background::draw_border()
{
    const int border_w = 128;
    SDL_Rect rect { game_w - border_w, 0, border_w, game_h };
    SDL_SetTextureBlendMode(border_texture, SDL_BLENDMODE_ADD);
    SDL_RenderCopy(renderer, border_texture, nullptr, &rect);

    SDL_Rect rect2 { 0, 0, border_w, game_h };
    SDL_SetTextureBlendMode(border_texture, SDL_BLENDMODE_MOD);
    SDL_RenderCopy(renderer, border_texture, nullptr, &rect2);
}

void Background::gen_star(Particle &star)
{
    std::uniform_int_distribution<int> particle_y(0, game_h);
    std::uniform_real_distribution<double> particle_angle(0.0, 360.0);

    std::uniform_int_distribution<int> star_size(1, 32);
    std::uniform_int_distribution<Uint8> star_color(200);
    std::uniform_real_distribution<double> star_speed(0.0, 2.0);
    std::uniform_real_distribution<double> star_rotation_speed(0.01, 1.0);

    star.x = game_w;
    star.size = star_size(mt);
    star.y = particle_y(mt) - star.size / 2;
    star.color.a = 255;
    star.color.r = star_color(mt);
    star.color.g = star_color(mt);
    star.color.b = star_color(mt);
    star.angle = particle_angle(mt);
    star.speed = star.size * 0.1 + star_speed(mt);
    star.rotation_speed = star_rotation_speed(mt);
    star.texture = star_texture;
}

void Background::gen_fog(Particle &fog)
{
    std::uniform_int_distribution<int> particle_y(0, game_h);
    std::uniform_real_distribution<double> particle_angle(0.0, 360.0);

    std::uniform_int_distribution<int> fog_size(512, 1024);
    std::uniform_int_distribution<Uint8> fog_color(1, 50);
    std::uniform_real_distribution<double> fog_speed(1.0, 10.0);
    std::uniform_real_distribution<double> fog_rotation_speed(-0.25, 0.25);

    fog.x = game_w;
    fog.size = fog_size(mt);
    fog.y = particle_y(mt) - fog.size / 2;
    fog.color.a = 255;
    fog.color.r = fog_color(mt);
    fog.color.g = fog_color(mt);
    fog.color.b = fog_color(mt);
    fog.angle = particle_angle(mt);
    fog.speed = fog_speed(mt);
    fog.rotation_speed = fog_rotation_speed(mt);
    fog.texture = fog_texture;
}
