#include <SDL3/SDL.h>
#include "hours.h"

int getHours(int seconds)
{
    int hours= ((seconds) - (seconds%3600)) / 3600;
    return hours;
}

int getMinutes(int seconds)
{
    int rest= (seconds%3600);
    int minutes= (rest - (rest%60)) / 60;
    return minutes;
}

int getRestSeconds(int seconds)
{
    return (seconds%3600) % 60;
}

std::string getRemaining(void) {
    if (!SDL_Init(0)) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return "error";
    }

    int secs, pct;
    SDL_PowerState state = SDL_GetPowerInfo(&secs, &pct);


    if (secs != -1) {
        return("Time remaining: " + std::to_string(getHours(secs)) + " hours " + std::to_string(getMinutes(secs)) + " minutes " + std::to_string(getRestSeconds(secs)) + " seconds\n");
    } else {
        return("Time remaining: unknown\n");
    }
}

int getBatteryPercentage()
{
    if (!SDL_Init(0)) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return -1;
    }

    int secs, pct;
    SDL_PowerState state = SDL_GetPowerInfo(&secs, &pct);
    return pct;
}

int getPowerState()
{
    if (!SDL_Init(0)) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return -1;
    }

    int secs, pct;
    SDL_PowerState state = SDL_GetPowerInfo(&secs, &pct);
    switch(state)
    {
        case SDL_POWERSTATE_ON_BATTERY:
            return 1;
            break;
        case SDL_POWERSTATE_CHARGING:
            return 2;
            break;
        case SDL_POWERSTATE_CHARGED:
            return 3;
            break;
        default:
            return 0;
            break;
    }
}
