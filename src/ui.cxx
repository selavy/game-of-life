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

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "golife.h"


#define DEBUG(format, ...) fmt::print(std::cerr, "[DEBUG ({:s})]: " format "\n", __func__, ##__VA_ARGS__)
#define INFO(format, ...)  fmt::print(std::cerr, "[INFO  ({:s})]: " format "\n", __func__, ##__VA_ARGS__)

static void glfw_error_callback(int error, const char* description) {
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
    int window_w;
    int window_h;
    std::vector<gol::Board> boards;
};

void ShowGameOfLifeWindow(bool* show_game_of_life_window, GameOfLife& state)
{
    auto& boards = state.boards;
    auto& board = boards.back();
    const int xmax = board.ncols;
    const int ymax = board.nrows;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(state.window_w, state.window_h));

    if (ImGui::Begin("Game Of Life", show_game_of_life_window, ImGuiWindowFlags_MenuBar))
    {
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

        if (ImGui::Button("Prev", ImVec2(100, 40)))
        {
            if (boards.size() > 1) {
                boards.pop_back();
            }
        }
        if (ImGui::Button("Next", ImVec2(100, 40)))
        {
            boards.push_back(board.tick());
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

    const int start_window_w = 1280;
    const int start_window_h = 720;
    GLFWwindow* window = glfwCreateWindow(start_window_w, start_window_h,
            "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL) {
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // Enable vsync

    if (gl3wInit() != 0) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    io.FontGlobalScale = 1.5;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use
    // ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application
    // (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling
    // ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double
    // backslash \\ !
    // io.Fonts->AddFontDefault();
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL,
    // io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != NULL);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    bool show_demo_window = true;
    bool show_metrics_window = true;
    bool show_game_of_life_window = true;
    GameOfLife gol_state{
        .window_w = start_window_w,
        .window_h = start_window_h,
        .boards = { { .nrows = 8, .ncols = 8} },
    };
    gol_state.boards[0].set_live(3, 4);
    gol_state.boards[0].set_live(3, 5);
    gol_state.boards[0].set_live(3, 6);

    glfwSetWindowUserPointer(window, &gol_state);
    glfwSetWindowSizeCallback(window, &OnWindowResize);

    while (!glfwWindowShouldClose(window)) {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your
        // inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those
        // two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
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

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
