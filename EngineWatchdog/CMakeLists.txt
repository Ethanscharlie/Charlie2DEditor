cmake_minimum_required(VERSION 3.0)
project(EngineWatchdog)

# Add C++11 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE IMGUI_SOURCES 
  ${PROJECT_SOURCE_DIR}/imgui/*.cpp 
  )

# Add executable
add_executable(engine_watchdog 
  ${IMGUI_SOURCES}
  src/main.cpp
  )

if (WIN32) 
  set(sdl2_include_path "/usr/x86_64-w64-mingw32/include/SDL2")
else()
  set(sdl2_include_path "/usr/include/SDL2")
endif()

target_include_directories(engine_watchdog PUBLIC
  ${sdl2_include_path}
  /usr/include/nlohmann
  ${PROJECT_SOURCE_DIR}/include/imgui
  )

target_link_libraries(engine_watchdog
  SDL2
  # SDL2_ttf
  SDL2_image
  )

set(EDITOR_DIRECTORY "${CMAKE_SOURCE_DIR}/../engine")
file(REMOVE_RECURSE "/usr/local/share/engine")
install(DIRECTORY ${EDITOR_DIRECTORY} DESTINATION /usr/local/share)

install(TARGETS engine_watchdog
  RUNTIME DESTINATION bin
  )
