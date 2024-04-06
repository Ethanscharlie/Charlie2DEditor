#include <SDL2/SDL_video.h>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

#include <SDL2/SDL.h>

namespace fs = std::filesystem;

const std::string copyFromPath = "/usr/local/share/engine/";
const std::string enginePath = "/tmp/engine/";

SDL_Window *window;

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
}

int main() {
  // createWindow();

  if (!fs::exists(copyFromPath)) {
    std::cerr << "Error: Engine files not correctly copied to the system\n";
    return 1;
  }

  // Copy engine files to /tmp
  if (!fs::exists(enginePath)) {
    fs::copy(copyFromPath, enginePath, fs::copy_options::recursive);
  } else {
    fs::remove_all(enginePath);
    fs::copy(copyFromPath, enginePath, fs::copy_options::recursive);
  }

  // Folder path containing header files
  std::string folderPath;
  std::ifstream prevProjectFile(enginePath + "prevProject.txt");
  if (prevProjectFile.is_open()) {
    std::getline(prevProjectFile, folderPath);
    prevProjectFile.close();
  } else {
    std::cerr << "Error: File not found :3" << std::endl;
    SDL_DestroyWindow(window);
    SDL_Quit();
  }

  // Create a temporary include file
  std::ofstream includeFile(enginePath + "include/include_tmp.h");
  includeFile.close(); // Create or truncate the file

  // Iterate over header files and generate #include directives
  for (const auto &entry : fs::directory_iterator(folderPath + "/src")) {
    if (entry.is_regular_file() && entry.path().extension() == ".h") {
      std::ofstream outFile(enginePath + "include/include_tmp.h",
                            std::ios_base::app);
      outFile << "#include " << entry.path().filename() << "" << std::endl;
      outFile.close();
    }
  }

  while (true) {
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
      int buildResult = std::system("cmake --build . && make");
      if (buildResult == 0) {
        // Run the executable
        int runResult = std::system(("./index " + folderPath).c_str());
        if (WIFEXITED(runResult)) {
          int exitCode = WEXITSTATUS(runResult);
          // Handle exit code
          if (exitCode == 42) {
            std::cout << "Restarting the editor..." << std::endl;
          } else {
            std::cout << "Exiting the editor." << std::endl;
            break;
          }
        }
      } else {
        std::cerr << "Build failed, exiting the editor." << std::endl;
        break;
      }
    } else {
      std::cerr << "CMake configuration failed, exiting the editor."
                << std::endl;
      break;
    }

    if (chdir("..") != 0) {
      std::cerr << "Error: Unable to change directory." << std::endl;
    }
  }

  // SDL_DestroyWindow(window);
  // SDL_Quit();
  return 0;
}
