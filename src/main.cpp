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

// Add OpenGL headers
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
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
	{
		std::cerr << "Error: SDL_Init failed: " << SDL_GetError() << std::endl;
		return 1;
	}

	// Create window
	SDL_Window* window = SDL_CreateWindow(
		"SoundImageConverter",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		800, 600,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);
	if (!window)
	{
		std::cerr << "Error: SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}

	// Create OpenGL context
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GLContext glContext = SDL_GL_CreateContext(window);
	if (!glContext)
	{
		std::cerr << "Error: SDL_GL_CreateContext failed: " << SDL_GetError() << std::endl;
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}
	SDL_GL_MakeCurrent(window, glContext);
	
	// Initialize ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForOpenGL(window, glContext);
	ImGui_ImplOpenGL3_Init("#version 330");

	// UI state
	std::string wavPath, pngPath, statusMessage;
	char wavPathBuffer[256] = "";
	char pngPathBuffer[256] = "";
	bool running = true;

	// Main loop
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

		// Start ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		// UI: Encode Section
		ImGui::Begin("SoundImageConverter");
		ImGui::Text("WAV to PNG (Encode)");
		ImGui::InputText("WAV File", wavPathBuffer, sizeof(wavPathBuffer));
		if (ImGui::Button("Select WAV File"))
		{
			const char* filters[] = { "*.wav" };
			const char* path = tinyfd_openFileDialog("Select WAV File", "", 1, filters, "WAV  Files (*.wav)", 0);
			if (path)
			{
				strncpy(wavPathBuffer, path, sizeof(wavPathBuffer) - 1);
				wavPath = path;
			}
		}
		if (ImGui::Button("Encode to PNG") && !wavPath.empty())
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

		// UI: Decode Section
		ImGui::Separator();
		ImGui::Text("PNG to WAV (Decode)");
		ImGui::InputText("PNG File", pngPathBuffer, sizeof(pngPathBuffer));
		if (ImGui::Button("Select PNG File"))
		{
			const char* filters[] = { "*.png" };
			const char* path = tinyfd_openFileDialog("Select PNG File", "", 1, filters, "PNG Files (*.png)", 0);
			if (path)
			{
				strncpy(pngPathBuffer, path, sizeof(pngPathBuffer) - 1);
				pngPath = path;
			}
		}
		if (ImGui::Button("Decode to WAV") && !pngPath.empty())
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

		// Status message
		ImGui::Separator();
		ImGui::Text("Status: %s", statusMessage.c_str());
		ImGui::End();

		// Render
		ImGui::Render();
		glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(window);
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(window);
	SDL_Quit();


	return 0;
}