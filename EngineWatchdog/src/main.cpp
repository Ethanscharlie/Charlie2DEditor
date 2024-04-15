#include "nlohmann/json.hpp"
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <cmath>
#include <cstdlib>
#include <cwchar>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

namespace fs = std::filesystem;

const std::string copyFromPath = "/usr/local/share/engine/";
const std::string enginePath = "/tmp/engine/";

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture = NULL;
std::string folderPath;

void createWindow() {

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError()
              << std::endl;
    return;
  }

  // Create a window
  window =
      SDL_CreateWindow("Charlie2D EngineWatchdog", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
  if (window == nullptr) {
    std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError()
              << std::endl;
    return;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
  ImGui_ImplSDLRenderer2_Init(renderer);

  texture =
      IMG_LoadTexture(renderer, "/usr/local/share/engine/sysimg/Splash.png");

  if (texture == nullptr) {
    std::cout << "Failed to load Image\n";
    std::cout << IMG_GetError() << std::endl;
  }

  ImGuiIO &io = ImGui::GetIO();
  io.DisplaySize = ImVec2((float)800, (float)600);
  // io.DisplayFramebufferScale =
  //     ImVec2(io.DisplaySize.x / 800, io.DisplaySize.y / 600);
}

void closeWindow() {
  if (window == nullptr)
    return;
  SDL_DestroyWindow(window);
  window = nullptr;

  ImGui_ImplSDLRenderer2_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
}

void imguiLoop() {
  ImGui_ImplSDLRenderer2_NewFrame();
  ImGui_ImplSDL2_NewFrame();

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL2_ProcessEvent(&event);
    switch (event.type) {
    case SDL_QUIT:
      closeWindow();
      std::exit(0);
      break;
    }
  }

  SDL_Rect renderRect = {0, 0, 800, 600};
  SDL_RenderCopy(renderer, texture, nullptr, &renderRect);

  ImGui::NewFrame();
  ImGui::Begin("Main");
  ImGui::Text("Hello World");
  ImGui::End();
  ImGui::Render();
  ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());

  SDL_RenderPresent(renderer);
}

void splashLoop() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      closeWindow();
      std::exit(0);
      break;
    }
  }

  SDL_Rect renderRect = {0, 0, 800, 600};
  SDL_RenderCopy(renderer, texture, nullptr, &renderRect);

  SDL_RenderPresent(renderer);
}

int doCompileLoop() {
  // Iterate over header files and generate #include directives
  for (const auto &entry : fs::directory_iterator(folderPath + "/src")) {
    if (entry.is_regular_file() && entry.path().extension() == ".h") {
      std::ofstream outFile(enginePath + "include/include_tmp.h",
                            std::ios_base::app);
      outFile << "#include " << entry.path().filename() << "" << std::endl;
      outFile.close();
    }
  }

  // Clean and create the build directory
  fs::remove_all(enginePath + "build");
  fs::create_directory(enginePath + "build");
  if (chdir((enginePath + "build").c_str()) != 0) {
    std::cerr << "Error: Unable to change directory." << std::endl;
    SDL_DestroyWindow(window);
    SDL_Quit();
  }

  // Generate CMake cache with additional include directory
  int cmakeResult = std::system(("cmake -DPROJECT_PATH=" + folderPath +
                                 " -DCMAKE_INCLUDE_PATH=\"../include\" ..")
                                    .c_str());
  if (cmakeResult == 0) {
    splashLoop();
    int buildResult = std::system("cmake --build . && make");
    if (buildResult == 0) {
      // Run the executable
      closeWindow();
      int runResult = std::system(("./index " + folderPath).c_str());
      if (WIFEXITED(runResult)) {
        int exitCode = WEXITSTATUS(runResult);
        // Handle exit code
        if (exitCode == 42) {
          std::cout << "Restarting the editor..." << std::endl;
        } else {
          std::cout << "Exiting the editor." << std::endl;
          return 0;
        }
      }
    } else {
      std::cerr << "Build failed, exiting the editor." << std::endl;
      return 0;
    }
  } else {
    std::cerr << "CMake configuration failed, exiting the editor." << std::endl;
    return 0;
  }

  if (chdir("..") != 0) {
    std::cerr << "Error: Unable to change directory." << std::endl;
  }

  return 1;
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);
  IMG_Init(IMG_INIT_PNG);

  createWindow();

  if (!fs::exists(copyFromPath)) {
    std::cerr << "Error: Engine files not correctly copied to the system\n";
    return 1;
  }

  // Copy engine files to /tmp
  if (!fs::exists(enginePath)) {
    std::system(std::format("cp -r {} /tmp/", copyFromPath).c_str());
  }

  // Create a temporary include file
  std::ofstream includeFile(enginePath + "include/include_tmp.h");
  includeFile.close(); // Create or truncate the file

  while (true) {
    std::ifstream prevProjectFile(enginePath + "prevProject.txt");
    if (prevProjectFile.is_open()) {
      std::getline(prevProjectFile, folderPath);
      prevProjectFile.close();
    } else {
      std::cerr << "Error: File not found :3" << std::endl;
    }

    folderPath = "/home/ethanscharlie/Projects/Code/C++/CharlieGamesv2/Fish2";

    bool prevPathBeenSet = folderPath != "";
    if (prevPathBeenSet) {
      bool keepOpen = doCompileLoop();
      if (!keepOpen) break;
    } else {
      imguiLoop();
    }
  }

  closeWindow();
  SDL_Quit();
  return 0;
}
