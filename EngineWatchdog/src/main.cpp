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

#include <ImGuiFileDialog.h>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

#include "sharedHeader.h"

namespace fs = std::filesystem;

const std::string copyFromPath = "/usr/local/share/engine/";
const std::string enginePath = "/tmp/engine/";
std::stringstream buffer;

fs::path folderPath;
bool firstCompile = true;
bool debugMode = false;

std::string newlineInvertString(const std::string &input) {
  std::istringstream iss(input);
  std::vector<std::string> result;
  std::string line;
  while (std::getline(iss, line)) {
    result.insert(result.begin(), line);
  }
  std::ostringstream oss;
  for (const auto &str : result) {
    oss << str << "\n";
  }
  return oss.str();
}

std::string removeQuotes(const std::string &str) {
  if (!str.empty() && str.front() == '"' && str.back() == '"') {
    return str.substr(1, str.size() - 2);
  }
  return str;
}

void resetPrevPath() {
  std::ofstream file("/tmp/engine/prevProject.txt");
  file << "";
  file.close();
  folderPath = "";
}

class WindowSystem {
public:
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture = NULL;
  bool open = false;

  ~WindowSystem() { closeWindow(); }

  void createWindow() {
    if (open) {
      std::cout << "Window already open\n";
      return;
    }
    open = true;
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
  }

  void closeWindow() {
    if (!open)
      return;
    open = false;
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
    ImGui::SetNextWindowPos(ImVec2(50, 50));
    ImGui::SetNextWindowSize(ImVec2(150, 200));
    ImGui::Begin("Fixed Panel", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoTitleBar);

    if (ImGui::Button("New Project")) {
      IGFD::FileDialogConfig config;
      ImGuiFileDialog::Instance()->OpenDialog(
          "NewProject", "Choose Project Folder", nullptr, config);
    }

    if (ImGui::Button("Open Project")) {
      IGFD::FileDialogConfig config;
      ImGuiFileDialog::Instance()->OpenDialog("ChooseProject", "Choose File",
                                              nullptr, config);
    }

    if (ImGuiFileDialog::Instance()->Display(
            "ChooseProject", ImGuiWindowFlags_NoCollapse, ImVec2(700, 500))) {
      if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
        fs::path projectFolderpath =
            ImGuiFileDialog::Instance()->GetCurrentPath();
        std::ofstream file("/tmp/engine/prevProject.txt");
        file << removeQuotes(projectFolderpath) << std::endl;
        file.close();
        std::cout << "Set prev path as " << projectFolderpath << "\n";
      }

      // close
      ImGuiFileDialog::Instance()->Close();
    }

    if (ImGuiFileDialog::Instance()->Display(
            "NewProject", ImGuiWindowFlags_NoCollapse, ImVec2(700, 500))) {
      if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
        fs::path projectFolderpath =
            ImGuiFileDialog::Instance()->GetCurrentPath();
        createNewProject(projectFolderpath);

        std::ofstream newProjectFile("/tmp/engine/prevProject.txt");
        newProjectFile << removeQuotes(projectFolderpath) << std::endl;
        newProjectFile.close();
        std::cout << "Set prev path as " << projectFolderpath << "\n";
      }

      ImGuiFileDialog::Instance()->Close();
    }

    ImGui::End();
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());

    SDL_RenderPresent(renderer);
  }

  void imguiDebugLoop() {
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
    ImGui::SetNextWindowPos(ImVec2(50, 50));
    ImGui::SetNextWindowSize(ImVec2(700, 500));
    ImGui::Begin("Fixed Panel", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoTitleBar);

    if (ImGui::Button("Retry")) {
      std::cout << "Retry\n";
      debugMode = false;
    }

    if (ImGui::Button("Wipe prevProject and close")) {
      resetPrevPath();
      std::exit(0);
    }

    std::ifstream gdbFile("/tmp/engine/build/gdb.txt"); // Open the file
    std::stringstream gdbText;
    gdbText << gdbFile.rdbuf();
    gdbFile.close();

    std::ifstream cmakeFile("/tmp/engine/build/cmakeout.txt"); // Open the file
    std::stringstream cmakeText;
    cmakeText << cmakeFile.rdbuf();
    cmakeFile.close();

    ImGui::Text(gdbText.str().c_str());
    ImGui::Text(cmakeText.str().c_str());
    ImGui::Text(newlineInvertString(buffer.str()).c_str());

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

    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImVec2(350, 250));
    ImGui::SetNextWindowSize(ImVec2(100, 50));
    ImGui::Begin("Fixed Panel Loading", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoTitleBar);

    ImGui::Text("Compiling...");

    ImGui::End();
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());

    SDL_RenderPresent(renderer);
  }
};

int attemptCompile(WindowSystem &windowSystem, bool first = false) {
  // Generate CMake cache with additional include directory
  firstCompile = false;

  std::string compileEditorFlag;
  if (first) {
    compileEditorFlag = "-DCOMPILE_EDITOR=ON";
  } else {
    compileEditorFlag = "-DCOMPILE_EDITOR=OFF";
  }

  debugMode = false;

  int cmakeResult = std::system(
      std::format("cmake -DPROJECT_PATH=\"{}\" "
                  "-DCMAKE_INCLUDE_PATH=\"../include\" {} .. >> cmakeout.txt",
                  folderPath.string(), compileEditorFlag)
          .c_str());
  if (cmakeResult != 0) {
    windowSystem.createWindow();
    debugMode = true;
    return 1;
  }
  // splashLoop();
  int buildResult = std::system("stdbuf -oL -eL make > cmakeout.txt 2>&1");
  if (buildResult != 0) {
    windowSystem.createWindow();
    debugMode = true;
    return 1;
  }

  // Run the executable
  windowSystem.closeWindow();
  int gdbRunResult =
      std::system("gdb -batch -ex 'set logging on' "
                  "-ex \"run\" -ex \"bt\" -ex \"quit\" --args index");

  std::ifstream exitCodeFile("/tmp/engine/build/outputCode.txt");
  std::string exitCodeStr;

  if (exitCodeFile) {
    exitCodeFile >> exitCodeStr;
  } else {
    exitCodeStr = "0";
  }

  int exitCode = std::stoi(exitCodeStr);
  exitCodeFile.close();

  std::cout << "EXIT CODE " << exitCode << std::endl;
  // Handle exit code
  if (exitCode == EXTCODE_RECOMPILE) {
    std::cout << "Restarting the editor..." << std::endl;
  }

  else if (exitCode == EXTCODE_BAD_PROJECT_FOLDER) {
    resetPrevPath();
  }

  else if (exitCode == EXTCODE_EXIT) {
    std::cout << "Exiting the editor." << std::endl;
    return 0;
  }

  else {
    windowSystem.createWindow();
    debugMode = true;
    return 1;
  }
  return 1;
}

WindowSystem windowSystem = WindowSystem();

int main() {
  std::cout.rdbuf(buffer.rdbuf());

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError()
              << std::endl;
    return 0;
  }

  IMG_Init(IMG_INIT_PNG);

  windowSystem.createWindow();

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
    std::ifstream prevProjectFile(fs::path(enginePath) / "prevProject.txt");
    if (prevProjectFile.is_open()) {
      std::string path;
      std::getline(prevProjectFile, path);
      folderPath = fs::path(path);
      prevProjectFile.close();
    } else {
      std::cerr << "Error: File not found :3" << std::endl;
    }

    // folderPath = "/home/ethanscharlie/Projects/Code/C++/CharlieGamesv2/Fish
    // 2";

    bool prevPathBeenSet = folderPath != "";
    if (prevPathBeenSet && !debugMode) {
      fs::path projectCodePath = fs::path(removeQuotes(folderPath)) / "src";
      if (!fs::exists(projectCodePath)) {
        resetPrevPath();
        continue;
      }

      for (const auto &entry : fs::directory_iterator(projectCodePath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".h") {
          std::ofstream outFile(enginePath + "include/include_tmp.h",
                                std::ios_base::app);

          outFile << "#include " << entry.path().filename() << "" << std::endl;
          outFile.close();
        }
      }

      // Clean and create the build directory
      // fs::remove_all(enginePath + "build");
      if (!fs::exists(enginePath + "build")) {
        fs::create_directory(enginePath + "build");
      }

      if (chdir((enginePath + "build").c_str()) != 0) {
        std::cerr << "Error: Unable to change directory." << std::endl;
        windowSystem.closeWindow();
        SDL_Quit();
        return 1;
      }

      windowSystem.createWindow();
      windowSystem.splashLoop();

      bool keepOpen = attemptCompile(windowSystem, firstCompile);
      if (!keepOpen)
        break;
    }

    else if (debugMode) {
      windowSystem.imguiDebugLoop();
    }

    else {
      windowSystem.imguiLoop();
    }
  }

  windowSystem.closeWindow();
  SDL_Quit();
  return 0;
}
