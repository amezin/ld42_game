#include <list>
#include <memory>

#include <SDL.h>
#include <SDL_image.h>

static const int game_w = 1024;
static const int game_h = 768;

class GameObjectInterface
{
public:
    GameObjectInterface();
    virtual ~GameObjectInterface();

    virtual bool update();
    virtual void draw(SDL_Renderer *);
};

GameObjectInterface::GameObjectInterface()
{
}

GameObjectInterface::~GameObjectInterface()
{
}

bool GameObjectInterface::update()
{
    return true;
}

void GameObjectInterface::draw(SDL_Renderer *)
{
}

class GameLayer : public GameObjectInterface
{
public:
    GameLayer();
    ~GameLayer() override;

    bool update() override;
    void draw(SDL_Renderer *) override;
    void add(GameObjectInterface *);

    typedef std::unique_ptr<GameObjectInterface> ObjectPtr;
    std::list<ObjectPtr> objects;
};

GameLayer::GameLayer()
{
}

GameLayer::~GameLayer()
{
}

bool GameLayer::update()
{
    for (auto i = objects.begin(); i != objects.end();) {
        if ((*i)->update()) {
            i++;
        } else {
            i = objects.erase(i);
        }
    }

    return true;
}

void GameLayer::draw(SDL_Renderer *renderer)
{
    for (auto &ptr : objects) {
        ptr->draw(renderer);
    }
}

void GameLayer::add(GameObjectInterface *object)
{
    objects.push_back(ObjectPtr(object));
}

class GameObject : public GameObjectInterface
{
public:
    GameObject();
    ~GameObject() override;

    void draw(SDL_Renderer *) override;

    SDL_Rect rect;
    SDL_Color color;
    SDL_BlendMode blend_mode;
};

GameObject::GameObject()
    : rect{ 0, 0, 100, 100 },
      color{ 255, 255, 255, 255 },
      blend_mode(SDL_BLENDMODE_NONE)
{
}

GameObject::~GameObject()
{
}

void GameObject::draw(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawBlendMode(renderer, blend_mode);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}

class Chuvak : public GameObject
{
public:
    Chuvak();
    ~Chuvak() override;

    bool update() override;
    void draw(SDL_Renderer *) override;

    SDL_GameController *const controller;
    bool alive;
};

Chuvak::Chuvak()
    : controller(SDL_GameControllerOpen(0)), alive(true)
{
    rect.w = 50;
    rect.h = 100;
    rect.x = (game_w - rect.w) / 2;
    rect.y = (game_h - rect.h) / 2;
}

Chuvak::~Chuvak()
{
    SDL_GameControllerClose(controller);
}

bool Chuvak::update()
{
    if (!alive) {
        return false;
    }

    auto dx = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
    auto dy = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
    rect.x += dx / 4096;
    rect.y += dy / 4096;

    if (rect.x < 0) rect.x = 0;
    if (rect.y < 0) rect.y = 0;
    if (rect.x + rect.w > game_w) rect.x = game_w - rect.w;
    if (rect.y + rect.h > game_h) rect.y = game_h - rect.h;

    return true;
}

void Chuvak::draw(SDL_Renderer *renderer)
{
    if (alive) GameObject::draw(renderer);
}

class Star : public GameObject
{
public:
    Star();
    ~Star() override;

    bool update() override;

    int speed;
};

Star::Star()
{
    rect.w = rect.h = 1 + rand() % 4;
    rect.x = rand() % game_w;
    rect.y = rand() % game_h;
    color.r = 128 + rand() % 128;
    color.g = 128 + rand() % 128;
    color.b = 128 + rand() % 128;
    color.a = 255;
    speed = rect.w + rand() % 4;
}

Star::~Star()
{
}

bool Star::update()
{
    rect.x -= speed;
    if (rect.x < 0) rect.x = game_w;

    return true;
}

class Asteroid : public GameObject
{
public:
    explicit Asteroid(Chuvak *);
    ~Asteroid() override;

    bool update() override;

    Chuvak *const chuvak;
};

Asteroid::Asteroid(Chuvak *chuvak)
    : chuvak(chuvak)
{
    rect.w = rect.h = 10 + rand() % 20;
    rect.x = game_w;
    rect.y = rand() % (game_h - rect.h);
    color.r = 255;
    color.g = 0;
    color.b = 0;
}

Asteroid::~Asteroid()
{
}

bool Asteroid::update()
{
    rect.x -= 10;

    if (chuvak->alive && SDL_HasIntersection(&rect, &chuvak->rect)) {
        chuvak->alive = false;
        return false;
    }

    return rect.x > -rect.w;
}

int main()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    atexit(SDL_Quit);

    auto window = SDL_CreateWindow("Chuvak Is Running Out Of Space",
                                   SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED,
                                   game_w,
                                   game_h,
                                   SDL_WINDOW_OPENGL);
    auto renderer = SDL_CreateRenderer(window,
                                       -1,
                                       SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    Chuvak chuvak;
    GameLayer stars, asteroids;

    while (stars.objects.size() < 1000) {
        stars.add(new Star());
    }

    int asteroid_spawn_delay = 0;

    SDL_Event event{};
    while (event.type != SDL_QUIT) {
        if (SDL_PollEvent(&event)) {
            switch (event.type) {
            }
        } else {
            stars.update();
            asteroids.update();
            if (asteroid_spawn_delay == 0) {
                asteroids.add(new Asteroid(&chuvak));
                asteroid_spawn_delay = 100;
            } else {
                asteroid_spawn_delay--;
            }
            chuvak.update();

            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            stars.draw(renderer);
            chuvak.draw(renderer);
            asteroids.draw(renderer);

            SDL_RenderPresent(renderer);
        }
    }

    return 0;
}
