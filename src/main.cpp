#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <implot.h>
#include <cstdio>
#include <string>
#include "hours.h"
#include "data.h"
#include "read_logs.h"

namespace {

    /*Window layout */
    constexpr int WINDOW_WIDTH  = 480;
    constexpr int WINDOW_HEIGHT = 360;

    constexpr ImVec2 BATTERY_PANEL_POS  {0, 0};
    constexpr ImVec2 BATTERY_PANEL_SIZE {480, 55};
    constexpr float  PLOT_PANEL_TOP     = 55;
    constexpr float  PLOT_PANEL_MARGIN  = 0;   // bottom margin

    constexpr ImGuiWindowFlags PANEL_FLAGS =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove     | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

    /*Battery constants */
    constexpr int BATTERY_MARGIN = 8; // Default Imgui Window padding according to Google
    constexpr int BATTERY_WIDTH  = 480 - 16;
    constexpr int BATTERY_HEIGHT = 5;
    constexpr int BATTERY_BORDER = 1;

    constexpr ImU32 COL_GREEN = IM_COL32(55, 255, 55, 255);
    constexpr ImU32 COL_BLUE  = IM_COL32(55, 55, 255, 255);
    constexpr ImU32 COL_RED   = IM_COL32(255, 55, 55, 255);
    constexpr ImU32 COL_WHITE = IM_COL32(255, 255, 255, 255);

    // If getPowerState() wraps SDL_GetPowerInfo, prefer SDL_POWERSTATE_* here.
    enum PowerState { POWER_ON_BATTERY = 1, POWER_NO_BATTERY = 2, POWER_CHARGING = 3 };

    /*Time interval for logging*/
    constexpr int LOG_EVERY_N_FRAMES = 60;

    struct BatteryInfo {
        int         percentage;
        int         powerState;
        std::string remaining;
    };

    BatteryInfo readBattery()
    {
        return { getBatteryPercentage(), getPowerState(), getRemaining() };
    }

    ImU32 decideColor(int powerState)
    {
        switch (powerState) {
            case POWER_NO_BATTERY:
            case POWER_CHARGING:   return COL_GREEN;
            case POWER_ON_BATTERY: return COL_BLUE;
            default:               return COL_RED;
        }
    }

    void drawBattery(ImDrawList* drawList, ImVec2 position, int percentage, int powerState)
    {
        const ImVec2 outerSize(BATTERY_WIDTH  + 2 * BATTERY_BORDER,
                            BATTERY_HEIGHT + 2 * BATTERY_BORDER);
        const ImVec2 innerPos(position.x + BATTERY_BORDER, position.y + BATTERY_BORDER);
        const float  fillWidth = BATTERY_WIDTH * percentage / 100.0f;

        drawList->AddRectFilled(position, ImVec2(position.x + outerSize.x, position.y + outerSize.y), COL_WHITE);
        drawList->AddRectFilled(
            innerPos,
            ImVec2(innerPos.x + fillWidth, innerPos.y + BATTERY_HEIGHT),
            decideColor(powerState)
        );
    }

    /*UI utility functions*/
    void beginPanel(const char* name, ImVec2 pos, ImVec2 size)
    {
        ImGui::SetNextWindowPos(pos);
        ImGui::SetNextWindowSize(size);
        ImGui::Begin(name, nullptr, PANEL_FLAGS);
    }

    /* Draws the battery readout and bar. */
    void batterySection(const BatteryInfo& battery)
    {
        ImGui::Text("Battery capacity : %d%%", battery.percentage);
        ImGui::Text("%s", battery.remaining.c_str());

        drawBattery(ImGui::GetWindowDrawList(), ImGui::GetCursorScreenPos(),
                    battery.percentage, battery.powerState);
        ImGui::Dummy(ImVec2(BATTERY_WIDTH + 2 * BATTERY_BORDER,
                            BATTERY_HEIGHT + 2 * BATTERY_BORDER));
    }

    void plotSection(const PairData& data)
    {
        if (!ImPlot::BeginPlot("Battery usage")) return;

        ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, 0, data.count);
        ImPlot::SetupAxisLimitsConstraints(ImAxis_Y1, 0, 100);

        ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 2.5f);
        ImPlot::PushStyleColor(ImPlotCol_Line, IM_COL32(0, 200, 100, 255));
        ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4(0.2f, 0.6f, 0.9f, 0.25f));

        ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 5.0f, ImVec4(1, 0, 0, 1), -1.0f);
        ImPlot::PlotLine("Battery %", data.x, data.y, data.count);
        ImPlot::PlotShaded("Battery % (area)", data.x, data.y, data.count, 0.0);

        ImPlot::PopStyleColor(2);
        ImPlot::PopStyleVar();
        ImPlot::EndPlot();
    }

    struct Segment {
        size_t start;
        size_t count;
        int power;
    };

    std::vector<Segment> buildSegments(const PlotSeries& series)
    {
        std::vector<Segment> segs;
        const size_t n= series.x.size();

        if (n< 2) return segs;

        size_t start= 0;
        for (size_t i= 1; i< n; i++)
        {
            if (series.power[i] != series.power[start]) //if the power state changes
            {
                segs.push_back({start, i - start + 1, series.power[start]}); //
                start= i;
            }
        }
        segs.push_back({start, n - start, series.power[start]});

        return segs;
    }

    const char* powerLabel(int powerState)
    {
        switch (powerState) {
            case POWER_NO_BATTERY:
            case POWER_CHARGING:   return "Charging / AC";
            case POWER_ON_BATTERY: return "On battery";
            default:               return "Unknown";
        }
    }

    void plotSection_(const PlotSeries& series, const std::vector<Segment>& segments)
    {
        if (!ImPlot::BeginPlot("Battery usage")) return;

        ImPlot::SetupAxis(ImAxis_X1, nullptr, ImPlotScale_Time);
        ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 100);

        const int n = (int)series.x.size();
        if (n > 0)
        {
            ImPlot::SetupAxisLimits(ImAxis_X1, series.x.front(), series.x.back(), ImPlotCond_Once);
            ImPlot::SetupAxisLimitsConstraints(ImAxis_Y1, 0, 100);
        }

        // Shaded areas first, so the line is drawn on top
        for (const Segment& seg : segments)
        {
            if (seg.count < 2) continue;

            const ImVec4 color = ImGui::ColorConvertU32ToFloat4(decideColor(seg.power));
            ImPlot::SetNextFillStyle(color, 0.35f);
            ImPlot::PlotShaded(
                powerLabel(seg.power),
                series.x.data() + seg.start,
                series.y.data() + seg.start,
                (int)seg.count,
                0.0
            );
        }

        if (n > 0)
        {
            ImPlot::SetNextLineStyle(ImVec4(1, 1, 1, 1), 2.0f);
            ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 3.0f, ImVec4(1, 1, 1, 1), -1.0f);
            ImPlot::PlotLine("Battery %", series.x.data(), series.y.data(), n);
        }

        ImPlot::EndPlot();
    }

    /*Frame plumbing */
    /** Returns false once the user asks to quit. */
    bool pumpEvents()
    {
        bool running = true;
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                running = false;
        }
        return running;
    }

    void beginImGuiFrame()
    {
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
    }

    void renderFrame(SDL_Renderer* renderer)
    {
        ImGui::Render();
        SDL_SetRenderDrawColor(renderer, 25, 25, 25, 255);
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    void logBattery(FILE* file, int frame, const BatteryInfo& battery)
    {
        if (file && frame % LOG_EVERY_N_FRAMES == 0)
            fprintf(
                file,
                "Frame : %d, battery : %d%%, %s\n",
                frame,
                battery.percentage,
                battery.remaining.c_str()
            );
    }

} // namespace

int main()
{
    std::vector<SingleInstantBatteryState> s= read_from_log_file_vector(const_cast<char*>("/home/mqbaka/31-5-2026.log"));
    const PlotSeries plots= getXY(s);
    const std::vector<Segment> segments= buildSegments(plots);
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window*   window   = SDL_CreateWindow("SDL3 + ImGui", WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer* renderer = window ? SDL_CreateRenderer(window, nullptr) : nullptr;
    if (!renderer) {
        fprintf(stderr, "Window/renderer creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    FILE* logFile = fopen("temp_logs/log.txt", "a");
    if (!logFile)
        fprintf(stderr, "Warning: could not open log.txt, logging disabled\n");

    // Path is relative to the working directory (see data.h)
    const PairData data = read_pairs("temp_logs/data.txt");

    const ImGuiIO& io = ImGui::GetIO();
    int frameCount = 0;

    while (pumpEvents()) {
        beginImGuiFrame();

        const BatteryInfo battery = readBattery();

        beginPanel(
            "Battery",
            BATTERY_PANEL_POS,
            BATTERY_PANEL_SIZE
        );
        batterySection(battery);
        ImGui::End();

        // Remove the default padding for the next window 
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        beginPanel(
            "Plot",
            ImVec2(0, PLOT_PANEL_TOP),
            ImVec2(io.DisplaySize.x, io.DisplaySize.y - PLOT_PANEL_TOP - PLOT_PANEL_MARGIN)
        );
        // The next line is needed whenever we use PushStyleVar
        ImGui::PopStyleVar();
        plotSection_(plots, segments);
        ImGui::End();

        renderFrame(renderer);
        logBattery(logFile, ++frameCount, battery);
    }

    if (logFile) fclose(logFile);

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}