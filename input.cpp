#include "input.h"

#include <limits>

Input::Input()
    : controller(SDL_GameControllerOpen(0)), mx(0.0), my(0.0), button_pressed(false)
{
}

Input::~Input()
{
    SDL_GameControllerClose(controller);
}

static double axis_to_double(Sint16 axis)
{
    if (axis > 0) {
        return double(axis) / double(std::numeric_limits<Sint16>::max());
    }
    if (axis < 0) {
        return -(double(axis) / double(std::numeric_limits<Sint16>::min()));
    }
    return 0.0;
}

void Input::update()
{
    mx = 0.0;
    my = 0.0;

    static const double hyst = 0.1;

    double lx = axis_to_double(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX));
    if (abs(lx) > abs(mx)) mx = lx;

    double rx = axis_to_double(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX));
    if (abs(rx) > abs(mx)) mx = rx;

    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT)) {
        mx = -1.0;
    }
    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) {
        mx = 1.0;
    }

    double ly = axis_to_double(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY));
    if (abs(ly) > abs(my)) my = ly;

    double ry = axis_to_double(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY));
    if (abs(ry) > abs(my)) my = ry;

    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP)) {
        my = -1.0;
    }
    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN)) {
        my = 1.0;
    }

    if (abs(mx) < hyst) mx = 0.0;
    if (abs(my) < hyst) my = 0.0;

    button_pressed = false;
    for (int i = SDL_CONTROLLER_BUTTON_A; i < SDL_CONTROLLER_BUTTON_LEFTSTICK; i++) {
        if (SDL_GameControllerGetButton(controller, SDL_GameControllerButton(i))) {
            button_pressed = true;
        }
    }
}
