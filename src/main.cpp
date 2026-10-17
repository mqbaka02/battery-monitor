#include <SDL3/SDL.h>
#include <imgui.h>
#include <implot.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include <cstdio>

int main()
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Battery Monitor",
        1280,
        720,
        SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);

    if (!renderer) {
        std::fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGui::StyleColorsDark();

    if (!ImGui_ImplSDL3_InitForSDLRenderer(window, renderer) ||
        !ImGui_ImplSDLRenderer3_Init(renderer)) {
        std::fprintf(stderr, "Failed to initialize ImGui SDL3 backends.\n");

        ImPlot::DestroyContext();
        ImGui::DestroyContext();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool running = true;

    while (running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);

            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Battery Monitor");
        ImGui::Text("SDL3, Dear ImGui, and ImPlot are running.");

        if (ImPlot::BeginPlot("Example plot")) {
            static const double xs[] = {0.0, 1.0, 2.0, 3.0, 4.0};
            static const double ys[] = {3.7, 3.8, 3.75, 3.9, 4.0};

            ImPlot::PlotLine("Voltage", xs, ys, 5);
            ImPlot::EndPlot();
        }

        ImGui::End();

        ImGui::Render();

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer3_RenderDrawData(
            ImGui::GetDrawData(),
            renderer
        );
        SDL_RenderPresent(renderer);
    }

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}