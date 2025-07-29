#include <iostream>
#include "SoundImageConverter/Converter.h"
#include <filesystem>
#include <sstream>
#include <SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <tinyfiledialogs.h>
#include <string>
#include <SDL_syswm.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <gl/GL.h>

std::string generateUniqueFileName(const std::string& basePath)
{
    if (!std::filesystem::exists(basePath))
    {
        return basePath;
    }

    std::filesystem::path path(basePath);
    std::string stem = path.stem().string();
    std::string extension = path.extension().string();
    std::string directory = path.parent_path().string();

    int counter = 1;
    std::string newPath;
    do
    {
        std::ostringstream oss;
        oss << directory << "/" << stem << "_" << counter << extension;
        newPath = oss.str();
        counter++;
    } while (std::filesystem::exists(newPath));

    return newPath;
}

int main(int, char**)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        std::cerr << "Error: SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    SDL_Window* window = SDL_CreateWindow(
        "SoundImageConverter",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        480, 600,
        SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS
    );
    if (!window)
    {
        std::cerr << "Error: SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

#ifdef _WIN32
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(window, &wmInfo);
	HWND hwnd = wmInfo.info.win.window;

	// Set the window to be layered and transparent with per-pixel alpha
	LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
	SetWindowLong(hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hwnd, 0, 255, LWA_COLORKEY);
#endif

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext)
    {
        std::cerr << "Error: SDL_GL_CreateContext failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_GL_MakeCurrent(window, glContext);
    SDL_GL_SetSwapInterval(1);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    
    // Styling
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 12.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    
    // Color scheme
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.10f, 1.0f);
    colors[ImGuiCol_Header] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.22f, 0.22f, 0.25f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.28f, 0.28f, 0.31f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.45f, 0.75f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.55f, 0.85f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.30f, 0.60f, 0.90f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.05f, 0.05f, 0.07f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.28f, 0.60f);
    
    // Styling addons
    style.WindowRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.WindowPadding = ImVec2(16, 16);
    style.FramePadding = ImVec2(10, 6);
    style.ItemSpacing = ImVec2(10, 8);
    style.ItemInnerSpacing = ImVec2(8, 6);
    style.Alpha = 0.97f;
    
    // Font scaling
    style.ScaleAllSizes(1.1f);
    
    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init("#version 330");

    // UI state
    std::string wavPath, pngPath, statusMessage;
    char wavPathBuffer[256] = "";
    char pngPathBuffer[256] = "";
    bool running = true;
    bool showUI = true;
    bool isDragging = false;
    ImVec2 dragOffset;

    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
        }

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        if (showUI)
        {
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(io.DisplaySize);
            ImGui::Begin("##MainWindow", nullptr, 
                ImGuiWindowFlags_NoResize | 
                ImGuiWindowFlags_NoMove | 
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoBackground); // Important to avoid creating default ImGui background
            
            // Draw custom background with rounder corners manually
            ImDrawList* drawList = ImGui::GetWindowDrawList();
			ImVec2 windowPos = ImGui::GetWindowPos();
			ImVec2 windowSize = ImGui::GetWindowSize();
			ImU32 backgroundColor = IM_COL32(20, 20, 30, 255); // Background color

            drawList->AddRectFilled(
                windowPos,
                ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y),
                backgroundColor,
                style.WindowRounding
            );

            // Title bar
            const float titleBarHeight = 48.0f;
            ImVec2 titleBarMin = ImGui::GetWindowPos();
            ImVec2 titleBarMax = ImVec2(titleBarMin.x + ImGui::GetWindowWidth(), titleBarMin.y + titleBarHeight);
            
            drawList->AddRectFilled(titleBarMin, titleBarMax, IM_COL32(20, 20, 25, 255), style.WindowRounding, ImDrawFlags_RoundCornersAll);
            drawList->AddLine(ImVec2(titleBarMin.x, titleBarMax.y), titleBarMax, IM_COL32(80, 80, 90, 255));
            
            // Title with icon
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 12);
            ImGui::SetCursorPosX(20);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.95f, 1.0f, 1.0f));
            ImGui::Text("Sound Image Converter");
            ImGui::PopStyleColor();
            
            // Control buttons
            const float buttonSize = 24.0f;
            const float buttonY = (titleBarHeight - buttonSize) * 0.5f;
            
            // Position for both buttons (right-aligned)
            ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - (buttonSize * 2 + 8), buttonY));
            
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 0));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.4f, 0.4f, 0.5f));
            
            // Minimize button
            if (ImGui::Button("-", ImVec2(buttonSize, buttonSize)))
            {
                SDL_MinimizeWindow(window);
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Minimize");
            
            ImGui::SameLine();
            
            // Close button
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 0.9f));
            if (ImGui::Button("X", ImVec2(buttonSize, buttonSize)))
            {
                running = false;
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Close");
            ImGui::PopStyleColor(3);
            ImGui::PopStyleVar(2);
            
            // Dragging
            ImGui::SetCursorPos(ImVec2(0, 0));
            ImGui::InvisibleButton("##TitleBar", ImVec2(ImGui::GetWindowWidth() - 80, titleBarHeight));
            
#ifdef _WIN32
            if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
            {
				SDL_SysWMinfo wmInfo;
                SDL_VERSION(&wmInfo.version);
                if (SDL_GetWindowWMInfo(window, &wmInfo))
                {
                    HWND hwnd = wmInfo.info.win.window;
                    ReleaseCapture();
                    SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
                }
            }
#endif
            // Main content
            ImGui::SetCursorPosY(titleBarHeight + 20);
            
            // Encode section
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.12f, 0.12f, 0.15f, 1.0f));
            if (ImGui::CollapsingHeader("Encode Audio to Image", ImGuiTreeNodeFlags_DefaultOpen))  // ASCII arrow
            {
                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
                ImGui::Text("Select WAV Audio File");
                ImGui::PopStyleColor();
                
                ImGui::SetNextItemWidth(-100);
                ImGui::InputText("##WavFile", wavPathBuffer, sizeof(wavPathBuffer), ImGuiInputTextFlags_ReadOnly);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Selected audio file path");
                ImGui::SameLine();
                if (ImGui::Button("Browse##Wav", ImVec2(80, 0)))
                {
                    const char* filters[] = { "*.wav" };
                    const char* path = tinyfd_openFileDialog("Select Audio File", "", 1, filters, "WAV Files (*.wav)", 0);
                    if (path)
                    {
                        strncpy_s(wavPathBuffer, sizeof(wavPathBuffer), path, _TRUNCATE);
                        wavPath = path;
                    }
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Select a WAV file to convert");
                
                ImGui::Spacing();
                ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 140) * 0.5f);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.50f, 0.25f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.60f, 0.35f, 1.0f));
                if (ImGui::Button("Convert to PNG", ImVec2(140, 32)))  // Remove checkmark
                {
                    if (!wavPath.empty())
                    {
                        std::string outPng = generateUniqueFileName("resources/output.png");
                        if (SoundImageConverter::Encoder::encode(wavPath, outPng))
                        {
                            statusMessage = "Encoded successfully to " + outPng;
                        }
                        else
                        {
                            statusMessage = "Encoding failed!";
                        }
                    }
                    else
                    {
                        statusMessage = "Please select an audio file first!";
                    }
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Convert selected WAV to PNG");
                ImGui::PopStyleColor(2);
                ImGui::Spacing();
            }
            ImGui::PopStyleColor();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Decode section
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.12f, 0.12f, 0.15f, 1.0f));
            if (ImGui::CollapsingHeader("Decode Image to Audio", ImGuiTreeNodeFlags_DefaultOpen))  // ASCII arrow
            {
                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
                ImGui::Text("Select PNG Image File");
                ImGui::PopStyleColor();
                
                ImGui::SetNextItemWidth(-100);
                ImGui::InputText("##PngFile", pngPathBuffer, sizeof(pngPathBuffer), ImGuiInputTextFlags_ReadOnly);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Selected image file path");
                ImGui::SameLine();
                if (ImGui::Button("Browse##Png", ImVec2(80, 0)))
                {
                    const char* filters[] = { "*.png" };
                    const char* path = tinyfd_openFileDialog("Select Image File", "", 1, filters, "PNG Files (*.png)", 0);
                    if (path)
                    {
                        strncpy_s(pngPathBuffer, sizeof(pngPathBuffer), path, _TRUNCATE);
                        pngPath = path;
                    }
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Select a PNG file to convert");
                
                ImGui::Spacing();
                ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 140) * 0.5f);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.35f, 0.60f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.45f, 0.70f, 1.0f));
                if (ImGui::Button("Convert to WAV", ImVec2(140, 32)))
                {
                    if (!pngPath.empty())
                    {
                        std::string outWav = generateUniqueFileName("resources/output.wav");
                        if (SoundImageConverter::Decoder::decode(pngPath, outWav))
                        {
                            statusMessage = "Decoded successfully to " + outWav;
                        }
                        else
                        {
                            statusMessage = "Decoding failed!";
                        }
                    }
                    else
                    {
                        statusMessage = "Please select an image file first!";
                    }
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Convert selected PNG to WAV");
                ImGui::PopStyleColor(2);
                ImGui::Spacing();
            }
            ImGui::PopStyleColor();

            // Status area
            if (!statusMessage.empty())
            {
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                
                ImGui::SetCursorPosX(20);
                ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 80);
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 6));
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.10f, 0.10f, 0.12f, 0.9f));
                
                if (statusMessage.find("OK") != std::string::npos)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.9f, 0.3f, 1.0f));
                }
                else if (statusMessage.find("X") != std::string::npos)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
                }
                else
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
                }
                
                ImGui::BeginChild("StatusArea", ImVec2(ImGui::GetWindowWidth() - 40, 40), true);
                ImGui::TextWrapped("%s", statusMessage.c_str());
                ImGui::EndChild();
                ImGui::PopStyleColor(2);
                ImGui::PopStyleVar();
            }

            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}