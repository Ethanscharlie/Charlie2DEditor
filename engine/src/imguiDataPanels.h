#pragma once
#include "ImGuiFileDialog.h"
#include "Vector2f.h"
#include "imgui.h"
#include <Charlie2D.h>
#include <filesystem>
#include <format>
#include <string>

std::string projectFolderpath = "";

Entity *selectedEntity = nullptr;

namespace ImGui {
bool InputString(const char *label, std::string *strPtr,
                 size_t bufferSize = 256, ImGuiInputTextFlags flags = 0) {
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
    std::string imagePath = image->path;
    std::string prevImagePath = imagePath;

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
        imagePath = imagePath.substr(3);
      }

      // close
      ImGuiFileDialog::Instance()->Close();
    }

    ImGui::InputString(std::format("##{}images", data.name).c_str(),
                       &imagePath);

    if (imagePath != prevImagePath) {
      std::cout << "New Image\n";
      Image newImage = Image(imagePath);
      *image = newImage;
    }
  }

  else if (data.type == typeid(Font)) {
    Font *font = static_cast<Font *>(data.value);
    std::string fontPath = font->filepath;
    std::string prevFontPath = fontPath;

    if (ImGui::Button(std::format("Open File Dialog###{}fontselect", data.name)
                          .c_str())) {
      IGFD::FileDialogConfig config;
      config.path = projectFolderpath + "/img";
      ImGuiFileDialog::Instance()->OpenDialog("ChooseFont", "Choose File",
                                              ".ttf", config);
    }
    // display
    if (ImGuiFileDialog::Instance()->Display(
            "ChooseFont", ImGuiWindowFlags_NoCollapse, ImVec2(1000, 700))) {
      if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
        fontPath = ImGuiFileDialog::Instance()->GetFilePathName();
        fontPath = std::filesystem::absolute(fontPath).lexically_relative(
            projectFolderpath);
        fontPath = fontPath.substr(3);
      }

      // close
      ImGuiFileDialog::Instance()->Close();
    }

    ImGui::InputString(std::format("##{}fontpath", data.name).c_str(),
                       &fontPath);

    int prevSize = font->size;

    ImGui::InputInt(std::format("Size###{}fontsizein", data.name).c_str(),
                    &font->size);

    if (fontPath != prevFontPath || font->size != prevSize) {
      std::cout << "New Font\n";
      Font newFont = Font(fontPath, font->size);
      *font = newFont;
    }
  }

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
