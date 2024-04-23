#include "imgui_internal.h"

#include "functionUtils.h"
// #include "imguiDataPanels.h"
#include "ImGuiFileDialog.h"
#include "move.h"
#include "nlohmann/json.hpp"
#include <filesystem>
#include <format>

std::string projectFolderpath = "";
Entity *selectedEntity = nullptr;
json editorTempRunSave;
bool inRunState = false;
std::vector<Entity *> usedChildren;
std::stringstream buffer;

json serializeAllEntities() {
  json entitiesListJson;
  for (Entity *entity : GameManager::getAllObjects()) {
    if (entity->tag[0] == '$')
      continue;
    entitiesListJson[entity->tag].push_back(serialize(entity));
  }

  return entitiesListJson;
}

void writePrevProject(std::filesystem::path projectFolderpath) {
  std::ofstream file("../prevProject.txt");
  file << projectFolderpath << std::endl;
  file.close();
  std::cout << "Set prev path as " << projectFolderpath << "\n";
}

class TransformEdit;
void changeSelectedEntity(Entity *entity) {
  if (entity->tag[0] == '$')
    return;
  if (selectedEntity != nullptr) {
    selectedEntity->remove<TransformEdit>();
  }
  selectedEntity = entity;
  selectedEntity->add<TransformEdit>();
}

int compileForExport(std::filesystem::path exportFolder,
                     ExportTypes exportType) {
  int returnResult = 1;

  std::filesystem::path originalPath = std::filesystem::current_path();
  std::filesystem::current_path(exportFolder);

  std::string cmakeGenerateBuildFilesCommand;
  std::string cmakeBuildFilesCommand;
  switch (exportType) {
  case ExportTypes::Linux:
    cmakeGenerateBuildFilesCommand =
        std::format("cmake -DPROJECT_PATH=\"{}\" "
                    "-DCMAKE_INCLUDE_PATH=\"../include\" -DFINAL_BUILD=ON {}",
                    projectFolderpath, "/tmp/engine");
    cmakeBuildFilesCommand = "cmake --build . && make";
    break;
  case ExportTypes::Windows:
    cmakeGenerateBuildFilesCommand =
        std::format("cmake -DPROJECT_PATH=\"{}\" "
                    "-DCMAKE_INCLUDE_PATH=\"../include\" "
                    "-DCMAKE_TOOLCHAIN_FILE=/home/ethanscharlie/TC-mingw.cmake "
                    "-DFINAL_BUILD=ON {}",
                    projectFolderpath, "/tmp/engine");
    cmakeBuildFilesCommand = "make";
    break;
  case ExportTypes::Web:
    cmakeGenerateBuildFilesCommand =
        std::format("emcmake cmake -DPROJECT_PATH=\"{}\" "
                    "-DCMAKE_INCLUDE_PATH=\"../include\" -DFINAL_BUILD=ON {}",
                    projectFolderpath, "/tmp/engine");
    cmakeBuildFilesCommand = "emmake make";
    break;
  }

  int cmakeResult = std::system(cmakeGenerateBuildFilesCommand.c_str());
  if (cmakeResult == 0) {
    int buildResult = std::system(cmakeBuildFilesCommand.c_str());
    if (buildResult == 0) {
      std::cout << "Build Successful\n";
    } else {
      std::cerr << "Build failed" << std::endl;
      returnResult = 0;
      goto COMPILE_EXPORT_CLOSE;
    }
  } else {
    std::cerr << "CMake configuration failed" << std::endl;
    returnResult = 0;
    goto COMPILE_EXPORT_CLOSE;
  }

COMPILE_EXPORT_CLOSE:
  std::filesystem::current_path(originalPath);
  return returnResult;
}

void removeFolderContents(const std::string &folder) {
  for (const auto &entry : std::filesystem::directory_iterator(folder)) {
    if (entry.is_regular_file()) {
      std::filesystem::remove(entry.path());
    } else if (entry.is_directory()) {
      removeFolderContents(entry.path().string());
      std::filesystem::remove(entry.path());
    }
  }
}

void refreshAssets() {
  removeFolderContents("img");

  for (const auto &entry : std::filesystem::directory_iterator(
           std::filesystem::path(projectFolderpath) / "img")) {
    std::filesystem::copy(entry.path(), "img" / entry.path().filename());
  }

  ResourceManager::getInstance(GameManager::renderer).reloadAllTextures();

  for (Sprite *sprite : GameManager::getComponents<Sprite>()) {
    sprite->image = Image(sprite->image.path);
  }
}

json getEditorData() {
  std::filesystem::path editorDataFilePath =
      std::filesystem::path(projectFolderpath) / "EditorData.json";
  std::ifstream file(editorDataFilePath);
  json jsonData = json::parse(file);
  file.close();
  return jsonData;
}

void changeEditorData(json jsonData) {
  std::filesystem::path editorDataFilePath =
      std::filesystem::path(projectFolderpath) / "EditorData.json";
  std::ofstream file(editorDataFilePath);
  file << std::setw(2) << jsonData << std::endl;
  file.close();
}

json getMainScene() {
  std::filesystem::path sceneFilePath = getEditorData()["scene"];
  std::ifstream file(sceneFilePath);
  json jsonData = json::parse(file);
  file.close();
  return jsonData;
}

void changeMainScene(std::filesystem::path newScenePath) {
  json newEditorData = getEditorData();
  newEditorData["scene"] = newScenePath;
  changeEditorData(newEditorData);
}

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

namespace ImGui {
bool InputString(const char *label, std::string *strPtr, size_t bufferSize,
                 ImGuiInputTextFlags flags) {
  if (strPtr == nullptr) {
    return false;
  }

  char buffer[256];
  strncpy(buffer, strPtr->c_str(), bufferSize);
  buffer[bufferSize - 1] = '\0'; // Ensure null-terminated

  if (ImGui::InputText(label, buffer, bufferSize, flags)) {
    *strPtr = buffer;
    return true;
  }
  return false;
}
} // namespace ImGui

void imguiDataPanel(PropertyData data) {
  if (data.type == typeid(float)) {
    ImGui::InputFloat(std::format("##{}float", data.name).c_str(),
                      static_cast<float *>(data.value));
  }

  else if (data.type == typeid(Vector2f)) {
    Vector2f *vector = static_cast<Vector2f *>(data.value);

    ImGui::Text("X");
    ImGui::SameLine();
    ImGui::InputFloat(std::format("##{}vecx", data.name).c_str(),
                      static_cast<float *>(&vector->x));
    ImGui::Text("Y");
    ImGui::SameLine();
    ImGui::InputFloat(std::format("##{}vecy", data.name).c_str(),
                      static_cast<float *>(&vector->y));
  }

  else if (data.type == typeid(Box)) {
    Box *box = static_cast<Box *>(data.value);
    Vector2f *position = &box->position;
    Vector2f *size = &box->size;

    ImGui::Text("X");
    ImGui::SameLine();
    ImGui::InputFloat(std::format("##{}boxx", data.name).c_str(),
                      static_cast<float *>(&position->x));
    ImGui::Text("Y");
    ImGui::SameLine();
    ImGui::InputFloat(std::format("##{}boxy", data.name).c_str(),
                      static_cast<float *>(&position->y));
    ImGui::Text("W");
    ImGui::SameLine();
    ImGui::InputFloat(std::format("##{}boxw", data.name).c_str(),
                      static_cast<float *>(&size->x));
    ImGui::Text("H");
    ImGui::SameLine();
    ImGui::InputFloat(std::format("##{}boxh", data.name).c_str(),
                      static_cast<float *>(&size->y));
  }

  else if (data.type == typeid(bool)) {
    bool *checkbox = static_cast<bool *>(data.value);
    ImGui::Checkbox(std::format("##{}bool", data.name).c_str(), checkbox);
  }

  else if (data.type == typeid(std::string)) {
    std::string *string = static_cast<std::string *>(data.value);
    ImGui::InputString(std::format("##{}stringinputprop", data.name).c_str(),
                       string);
  }

  else if (data.type == typeid(int)) {
    int *value = static_cast<int *>(data.value);
    ImGui::InputInt(std::format("##{}intinputprop", data.name).c_str(), value);
  }

  else if (data.type == typeid(Uint8)) {
    Uint8 *value = static_cast<Uint8 *>(data.value);
    int *valueInt = static_cast<int *>(data.value);

    ImGui::InputInt(std::format("##{}intU8inputprop", data.name).c_str(),
                    valueInt);
  }

  else if (data.type == typeid(Image)) {
    Image *image = static_cast<Image *>(data.value);
    std::string imagePathString;
    std::filesystem::path imagePath = image->path;
    std::filesystem::path prevImagePath = imagePath;

    if (ImGui::Button("Open File Dialog")) {
      IGFD::FileDialogConfig config;
      config.path = projectFolderpath + "/img";
      ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File",
                                              ".png", config);
    }
    // display
    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey",
                                             ImGuiWindowFlags_NoCollapse,
                                             ImVec2(1000, 700))) {
      if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
        imagePath = ImGuiFileDialog::Instance()->GetFilePathName();
        imagePath = std::filesystem::absolute(imagePath).lexically_relative(
            projectFolderpath);
        imagePathString = imagePath.string();
        imagePath = imagePathString.substr(3);
      }

      // close
      ImGuiFileDialog::Instance()->Close();
    }

    imagePathString = imagePath.string();
    ImGui::InputString(std::format("##{}images", data.name).c_str(),
                       &imagePathString);
    imagePath = imagePathString;

    if (imagePath.string() != prevImagePath.string()) {
      std::cout << "New Image\n";
      Image newImage = Image(imagePath.string());
      *image = newImage;
    }
  }

  // else if (data.type == typeid(Font)) {
  //   Font *font = static_cast<Font *>(data.value);
  //   std::string fontPath = font->filepath;
  //   std::string prevFontPath = fontPath;
  //
  //   if (ImGui::Button(std::format("Open File Dialog###{}fontselect", data.name)
  //                         .c_str())) {
  //     IGFD::FileDialogConfig config;
  //     config.path = projectFolderpath + "/img";
  //     ImGuiFileDialog::Instance()->OpenDialog("ChooseFont", "Choose File",
  //                                             ".ttf", config);
  //   }
  //   // display
  //   if (ImGuiFileDialog::Instance()->Display(
  //           "ChooseFont", ImGuiWindowFlags_NoCollapse, ImVec2(1000, 700))) {
  //     if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
  //       fontPath = ImGuiFileDialog::Instance()->GetFilePathName();
  //       fontPath = std::filesystem::absolute(fontPath).lexically_relative(
  //           projectFolderpath);
  //       fontPath = fontPath.substr(3);
  //     }
  //
  //     // close
  //     ImGuiFileDialog::Instance()->Close();
  //   }
  //
  //   ImGui::InputString(std::format("##{}fontpath", data.name).c_str(),
  //                      &fontPath);
  //
  //   int prevSize = font->size;
  //
  //   ImGui::InputInt(std::format("Size###{}fontsizein", data.name).c_str(),
  //                   &font->size);
  //
  //   if (fontPath != prevFontPath || font->size != prevSize) {
  //     std::cout << "New Font\n";
  //     Font newFont = Font(fontPath, font->size);
  //     *font = newFont;
  //   }
  // }

  else if (data.type == typeid(Entity *)) {
    Entity **entityPtr = static_cast<Entity **>(data.value);
    Entity *entity = *entityPtr;

    std::string entityName;
    if (entity != nullptr) {
      entityName = std::format("{} ({})", entity->name, entity->iid);
    } else {
      entityName = "Select an Entity";
    }

    if (ImGui::BeginCombo(std::format("###{}entitySelect", data.name).c_str(),
                          entityName.c_str())) {
      int i = 0;
      for (Entity *otherEntity : GameManager::getAllObjects()) {
        if (otherEntity == selectedEntity)
          continue;
        if (otherEntity->tag[0] == '$')
          continue;

        bool isSelected = false;
        std::string otherTag =
            std::format("{} ({})", otherEntity->name, otherEntity->iid);
        const char *keyc = otherTag.c_str();
        if (ImGui::Selectable(keyc, &isSelected)) {
        }

        if (isSelected) {
          *entityPtr = otherEntity;
        }

        i++;
      }
      ImGui::EndCombo();
    }
  }
}
