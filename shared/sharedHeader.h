#pragma once
#include <fstream>
#define EXTCODE_RECOMPILE 42
#define EXTCODE_BAD_PROJECT_FOLDER 40
#define EXTCODE_EXIT 43

#include "nlohmann/json.hpp"
using json = nlohmann::json;

void createNewProject(std::filesystem::path newProjectPath);
