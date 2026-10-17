#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include "hours.h"
#include <implot.h>
//#include <implot_internal.h>
#include "data.h"
#include <string>
#include <cstdio>

#define COL_GREEN IM_COL32(55, 255, 55, 255)
#define COL_BLUE IM_COL32(55, 55, 255, 255)
#define COL_RED IM_COL32(255, 55, 55, 255)
constexpr int WINDOW_WIDTH  = 480;
constexpr int WINDOW_HEIGHT = 360;
constexpr int LOG_EVERY_N_FRAMES = 60;
constexpr int BATTERY_WIDTH = 100;
constexpr int BATTERY_HEIGHT = 50;

ImU32 decideColor(int power_state)
{
    if (power_state== 2 || power_state== 3){
        return(COL_GREEN);
    } else if (power_state== 1) {
        return(COL_BLUE);
    } else {
        return(COL_RED);
    }
}

void drawRectangle(ImDrawList* draw_list, ImVec2 position, ImVec2 dimensions, ImU32 color)
{
    draw_list->AddRectFilled(ImVec2(position.x, position.y), ImVec2(position.x + dimensions.x, position.y + dimensions.y), color);
}

void drawBattery(ImDrawList* draw_list, ImVec2 position, int batteryPercentage, int power_state)
{
    ImVec2 inner_position= ImVec2(position.x + 1, position.y + 1);
    ImVec2 batteryVisualSize= ImVec2(BATTERY_WIDTH + 2, BATTERY_HEIGHT + 2);
    ImVec2 batteryInnerSize= ImVec2(batteryPercentage, BATTERY_HEIGHT);

    drawRectangle(draw_list, position, batteryVisualSize, IM_COL32(255, 255, 255, 255));
    drawRectangle(draw_list, inner_position, batteryInnerSize, decideColor(power_state));
}

void renderFrame(SDL_Renderer *renderer)
{
    ImGui::Render();
    SDL_SetRenderDrawColor(renderer, 25, 25, 25, 255);
    SDL_RenderClear(renderer);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_RenderPresent(renderer);
}

void prepareForShutdown()
{

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    SDL_Quit();
}

void processEvent(bool* running, SDL_Event* event)
{

    while (SDL_PollEvent(event))
    {
        ImGui_ImplSDL3_ProcessEvent(event);
        if (event->type == SDL_EVENT_QUIT)
            *running = false;
    }
}

void beginImGuiFrame()
{
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

/**
 * Draws the baterry bar
 */
void batterySection(int batteryPercentage, int powerState, const std::string& timeRemaining)
{
    ImGui::Text("Battery capacity : %d%%", batteryPercentage);
    ImGui::Text("%s", timeRemaining.c_str());

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 cursorPosition = ImGui::GetCursorScreenPos();
    drawBattery(draw_list, cursorPosition, batteryPercentage, getPowerState());
    ImGui::Dummy(ImVec2(BATTERY_WIDTH + 2, BATTERY_HEIGHT + 2));
}

void plotSection(const PairData& data)
{
    if (ImPlot::BeginPlot("Battery usage")) {

        ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 2.5f);
        ImPlot::PushStyleColor(ImPlotCol_Line, IM_COL32(0, 200, 100, 255));

        // ImPlot::SetupAxesLimits(0, 60, 0, 100, ImGuiCond_Always); //locking navigation x-min, x-max, y-min, y-max but locks zoom

        ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, 0, data.count);

        ImPlot::SetupAxisLimitsConstraints(ImAxis_Y1, 0, 100);

        ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4(0.2f, 0.6f, 0.9f, 0.25f)); // blueish, semi-transparent

        ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 5.0f, ImVec4(1.0f, 0.0f, 0.0f, 1.0f), -1.0f);


        ImPlot::PlotLine("My Line Plot", data.x, data.y, data.count);
        ImPlot::PlotShaded("Filled Region", data.x, data.y, data.count, 0.0);


        ImPlot::PopStyleColor(2);
        // ImPlot::PopStyleColor();
        ImPlot::PopStyleVar();
        ImPlot::EndPlot();
    }
}

int main()
{

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow("SDL3 + ImGui", WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

    // Setup Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO(); (void)io;

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    ImGui::StyleColorsDark();
    FILE *recorded_data= fopen("log.txt", "a");

    // bool show_demo_window = true;
    bool running = true;
    SDL_Event event;

    int frameCount= 0;

    PairData data= read_pairs("../temp_logs/data.txt"); //The file name's path should be in relation to the header file

    ImPlot::CreateContext();

    while (running) {
        processEvent(&running, &event);
        beginImGuiFrame();

        // ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(0,0));
        ImGui::SetNextWindowSize(ImVec2(200,100));

        ImGui::Begin("Battery", nullptr,
                    ImGuiWindowFlags_NoTitleBar |
                    ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoMove |
                    ImGuiWindowFlags_NoCollapse |
                    ImGuiWindowFlags_NoScrollbar |
                    ImGuiWindowFlags_NoScrollWithMouse
        );

            int battery      = getBatteryPercentage();
            int powerState   = getPowerState();
            std::string time = getRemaining();
            batterySection(battery, powerState, time);

        ImGui::End();
        
        ImGui::SetNextWindowPos(ImVec2(0,120));
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y - 140));
        ImGui::Begin("Plot", nullptr,
                     ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoScrollWithMouse
        );
            plotSection(data);
        ImGui::End();


        renderFrame(renderer);//boring stuff that I won't bother to remember

        if (frameCount== LOG_EVERY_N_FRAMES){
            fprintf(recorded_data, "Frame : %d, battery : %d%%, %s", frameCount, battery, time.c_str());
            frameCount= 1;
        } else {
            frameCount++;
        }
    }

    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    prepareForShutdown();//boring Imgui stuff that I won't bother to remember
    fclose(recorded_data);

    return 0;
}
