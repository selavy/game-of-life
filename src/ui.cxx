#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include <cstdio>
#include <array>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <variant>
#include <fstream>
#include <chrono>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "golife.h"


#define DEBUG(format, ...) fmt::print(std::cerr, "[DEBUG ({:s})]: " format "\n", __func__, ##__VA_ARGS__)
#define INFO(format, ...)  fmt::print(std::cerr, "[INFO  ({:s})]: " format "\n", __func__, ##__VA_ARGS__)

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

static ImVec4 MakeColor(float r, float g, float b, float a = 255) noexcept
{
    float sc = 1.0f/255.0f;
    float x = r * sc;
    float y = g * sc;
    float z = b * sc;
    float w = a * sc;
    return { x, y, z, w };
}

ImVec4 LiveColor = MakeColor( 50,  50, 233); // dark blue
// ImVec4 LiveColor = MakeColor( 66,  66,  66); // gray
ImVec4 DeadColor = MakeColor(206, 187, 158); // light oak

static ImVec4 GetColor(bool islive) noexcept
{
    return islive ? LiveColor : DeadColor;
}

struct GameOfLife
{
    using Clock = std::chrono::steady_clock;
    using TimePoint = typename Clock::time_point;
    using Duration  = typename Clock::duration;

    bool setup_mode = true;
    int window_w;
    int window_h;
    gol::Board setupBoard = {};
    std::vector<gol::Board> boards;

    bool      playing = false;
    TimePoint next_tick_ts = {};
    Duration  tick_period  = {};
};

bool IsSteadyState(GameOfLife& state)
{
    auto& boards = state.boards;
    auto& board = boards.back();
    if (board.empty()) {
        return true;
    }
    if (boards.size() <= 1) {
        return false;
    }
    auto& board2 = boards[boards.size() - 2];
    return board == board2;
}

void ShowGameOfLifeWindow(bool* show_game_of_life_window, GameOfLife& state)
{
    auto& boards = state.boards;

    if (!state.setup_mode && state.playing) {
        auto now = GameOfLife::Clock::now();
        if (now >= state.next_tick_ts) {
            state.next_tick_ts = now + state.tick_period;
            boards.push_back(boards.back().tick());
        }
        if (IsSteadyState(state)) {
            state.playing = false;
        }
    }

    auto& board = boards.back();
    auto& setupBoard = state.setupBoard;
    const int xmax = board.ncols;
    const int ymax = board.nrows;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(state.window_w, state.window_h));

    const auto window_flags = 0
        | ImGuiWindowFlags_NoDecoration
        | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_AlwaysAutoResize
        ;
    if (ImGui::Begin("Game Of Life", show_game_of_life_window, window_flags))
    {
        if (state.setup_mode) {
            int id = 0;
            for (int y = 0; y < ymax; ++y) {
                for (int x = 0; x < xmax; ++x) {
                    ImGui::PushID(id++);
                    const auto square_color = GetColor(setupBoard.live(x, y));
                    ImGui::PushStyleColor(ImGuiCol_Button, square_color);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, square_color);
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, square_color);
                    ImGui::SameLine(/*offset_from_start_x*/0., /*spacing*/5.);
                    if (ImGui::Button("", ImVec2(40, 40))) {
                        setupBoard.flip_state(x, y);
                    }
                    ImGui::PopStyleColor(3);
                    ImGui::PopID();
                }
                ImGui::NewLine();
            }
            ImGui::NewLine();

            const int button_w = 100;
            ImGui::SameLine(state.window_w / 2 - button_w / 2, 0);
            if (ImGui::Button("Done", ImVec2(button_w, 40)))
            {
                state.setup_mode = false;
                state.boards.clear();
                state.boards.emplace_back(state.setupBoard);
            }
        } else {
            int id = 0;
            for (int y = 0; y < ymax; ++y) {
                for (int x = 0; x < xmax; ++x) {
                    ImGui::PushID(id++);
                    const auto square_color = GetColor(board.live(x, y));
                    ImGui::PushStyleColor(ImGuiCol_Button, square_color);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, square_color);
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, square_color);
                    ImGui::SameLine(/*offset_from_start_x*/0., /*spacing*/5.);
                    ImGui::Button("", ImVec2(40, 40));
                    ImGui::PopStyleColor(3);
                    ImGui::PopID();
                }
                ImGui::NewLine();
            }
            ImGui::NewLine();

            ImGui::Text("Iteration: %zu", boards.size());
            if (state.playing) {
                if (ImGui::Button("Stop", ImVec2(100, 40)))
                {
                    state.playing = false;
                }
            } else {
                if (ImGui::Button("Play", ImVec2(100, 40)))
                {
                    state.playing = true;
                }
                ImGui::SameLine(0, 5);
                if (ImGui::Button("Prev", ImVec2(100, 40)))
                {
                    if (boards.size() > 1) {
                        boards.pop_back();
                    }
                }
                ImGui::SameLine(0, 5);
                if (ImGui::Button("Next", ImVec2(100, 40)))
                {
                    boards.push_back(board.tick());
                }
                ImGui::SameLine(0, 5);
                if (ImGui::Button("Setup", ImVec2(100, 40)))
                {
                    state.setup_mode = true;
                    state.setupBoard = boards.back();
                }
                ImGui::SameLine(0, 5);
                if (ImGui::Button("Reset", ImVec2(100, 40)))
                {
                    state.setupBoard = boards[0];
                    state.boards.clear();
                    state.boards.emplace_back(state.setupBoard);
                }
            }
        }
    }
    ImGui::End();
}

void OnWindowResize(GLFWwindow* window, int width, int height)
{
    auto* s = static_cast<GameOfLife*>(glfwGetWindowUserPointer(window));
    s->window_w = width;
    s->window_h = height;
}

int main(int argc, char** argv)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        return 1;
    }

    // Decide GL+GLSL versions
#if __APPLE__
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    const int start_window_w = 700; // 1280;
    const int start_window_h = 800; // 720;
    GLFWwindow* window = glfwCreateWindow(start_window_w, start_window_h,
            "Game Of Life", NULL, NULL);
    if (window == NULL) {
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    if (gl3wInit() != 0) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = 1.5;

    // ImGui::StyleColorsDark();
    ImGui::StyleColorsClassic();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    bool show_demo_window = false;
    bool show_metrics_window = false;
    bool show_game_of_life_window = true;
    GameOfLife gol_state{
        .window_w = start_window_w,
        .window_h = start_window_h,
        .setupBoard = { .nrows = 15, .ncols = 15 },
        .boards = {},
        .tick_period = std::chrono::milliseconds(200),
    };


    const std::vector<std::pair<int, int>> starting_position = {
        // Blinker (period=2)
        { 3, 4 },
        { 3, 5 },
        { 3, 6 },

        // Glider
        { 7, 7 },
        { 8, 7 },
        { 9, 7 },
        { 9, 6 },
        { 8, 5 },
    };
    for (auto&& [x, y] : starting_position) {
        gol_state.setupBoard.set_live(x, y);
    }
    gol_state.boards.push_back(gol_state.setupBoard);

    glfwSetWindowUserPointer(window, &gol_state);
    glfwSetWindowSizeCallback(window, &OnWindowResize);
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        if (show_demo_window) {
            ImGui::ShowDemoWindow(&show_demo_window);
        }
        if (show_metrics_window) {
            ImGui::ShowMetricsWindow(&show_metrics_window);
        }
        if (show_game_of_life_window) {
            ShowGameOfLifeWindow(&show_game_of_life_window, gol_state);
        }
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
