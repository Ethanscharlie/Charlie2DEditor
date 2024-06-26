cmake_minimum_required(VERSION 3.10)

# set the project name
project(index)

# set the C++ standard to C++11
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PROJECT_PATH)
set(CMAKE_CURRENT_BINARY_DIR ${CMAKE_BINARY_DIR}/obj)

# get all source files
file(GLOB_RECURSE Game_SOURCES ${PROJECT_PATH}/src/*.cpp)

# file(GLOB_RECURSE CHARLIE_SOURCES /home/ethanscharlie/Projects/Code/Github/charlie2d/source/*.cpp)

set(CHARLIE2D_GITHUB_REPO_URL "https://github.com/Ethanscharlie/charlie2d.git")

file(GLOB_RECURSE IMGUI_SOURCES 
  ${PROJECT_SOURCE_DIR}/imgui/*.cpp 
  )

option(FINAL_BUILD "Final project build" OFF)
option(COMIPLE_EDITOR "Compile the editor and project" ON)

set(LOGICAL_WIDTH)
set(LOGICAL_HEIGHT)

# Define common source files
if (FINAL_BUILD) 
  set(COMMON_SOURCES
    src/main.cpp
    src/functionUtils.cpp
    src/ImguiPanels.cpp
    ${OBJECTS_SOURCES}
    ${Game_SOURCES}
    ${CMAKE_SOURCE_DIR}/shared/sharedHeader.cpp
    )

  if (EMSCRIPTEN OR WIN32)
    execute_process(
      COMMAND git clone ${CHARLIE2D_GITHUB_REPO_URL} "/tmp/charlie2Dsource"
      # RESULT_VARIABLE GIT_CLONE_RESULT
      )

    file(GLOB_RECURSE CHARLIE_SOURCES /tmp/charlie2Dsource/source/*.cpp)
    file(GLOB_RECURSE IMGUI_CHARLIE_SOURCES /tmp/charlie2Dsource/imgui/*.cpp)

    list(APPEND COMMON_SOURCES ${CHARLIE_SOURCES} ${IMGUI_CHARLIE_SOURCES})
  endif()

else()
  if (COMIPLE_EDITOR) 
    set(COMMON_SOURCES
      src/main.cpp
      src/functionUtils.cpp
      src/ImguiPanels.cpp
      src/move2.cpp
      ${OBJECTS_SOURCES}
      ${Game_SOURCES}
      ${IMGUI_SOURCES}
      ${CMAKE_SOURCE_DIR}/shared/sharedHeader.cpp
      )
  else()
    set(COMMON_SOURCES
      ${Game_SOURCES}
      )
  endif()
endif()

add_executable(index ${COMMON_SOURCES})

if(FINAL_BUILD)
  target_compile_definitions(index PRIVATE FINAL_BUILD)
endif()

if (WIN32) 
  set(sdl2_include_path "/usr/x86_64-w64-mingw32/include/SDL2")
  file(COPY ${CMAKE_SOURCE_DIR}/dll/. DESTINATION ${CMAKE_BINARY_DIR})
else()
  set(sdl2_include_path "/usr/include/SDL2")
endif()
 
# add include directories
target_include_directories(index PUBLIC
    ${sdl2_include_path}
    ${PROJECT_PATH}/src
    /usr/include/nlohmann
    /usr/local/include/charlie2D
    ${PROJECT_SOURCE_DIR}/include 
    ${PROJECT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/shared
)


if (WIN32)
  set(SDL_LIB_PATH "/usr/x86_64-w64-mingw32/lib")

  find_library(SDL2_      SDL2       PATHS ${SDL_LIB_PATH})
  find_library(SDL2_MAIN  SDL2main   PATHS ${SDL_LIB_PATH})
  find_library(SDL2_TTF   SDL2_ttf   PATHS ${SDL_LIB_PATH})
  find_library(SDL2_IMAGE SDL2_image PATHS ${SDL_LIB_PATH})
  find_library(SDL2_MIXER SDL2_mixer PATHS ${SDL_LIB_PATH})

  target_link_libraries(index
    -static-libgcc
    -static-libstdc++
    -static
    mingw32
    ${SDL2_MAIN}
    ${SDL2_}
    ${SDL2_TTF}
    ${SDL2_IMAGE}
    ${SDL2_MIXER}
    )

elseif(EMSCRIPTEN)
  target_link_libraries(index
    SDL2
    SDL2_ttf
    SDL2_mixer
    )

  # Enable Emscripten-specific flags
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL=2")
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -sNO_DISABLE_EXCEPTION_CATCHING")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL_IMAGE=2")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s SDL2_IMAGE_FORMATS=['png']")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL_TTF=2")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL_MIXER=2")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL_MIXER=2")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s WASM=0")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s ALLOW_MEMORY_GROWTH=1")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s EXPORTED_FUNCTIONS=\"['_main', '_on_resize']\"")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --use-preload-plugins")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --shell-file /usr/local/share/engine/sysimg/minimal.html")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --preload-file \"${PROJECT_PATH}/img@/img\"")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --preload-file \"/usr/local/share/engine/sysimg@/sysimg\"")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --preload-file \"${PROJECT_PATH}/EditorData.json@/EditorData.json\"")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -fexperimental-library")
else()
  # link libraries
  target_link_libraries(index
    SDL2
    SDL2_ttf
    SDL2_mixer
    SDL2_image
    charlie2D
    )
endif()

# copy image directory to build directory
file(COPY ${PROJECT_PATH}/img DESTINATION ${CMAKE_BINARY_DIR})
file(COPY /usr/local/share/engine/sysimg DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${PROJECT_PATH}/EditorData.json DESTINATION ${CMAKE_BINARY_DIR})

ADD_DEFINITIONS( -D_PROJECT_PATH=\"${PROJECT_PATH}\")
ADD_DEFINITIONS( -D_LOGICAL_WIDTH=\"${LOGICAL_WIDTH}\")
ADD_DEFINITIONS( -D_LOGICAL_HEIGHT=\"${LOGICAL_HEIGHT}\")

# set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

