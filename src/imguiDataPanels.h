#pragma once
#include "Vector2f.h"
#include "imgui.h"
#include <Charlie2D.h>
#include <format>

struct EditorDataItem {
  EditorDataItem(std::string _name, void *_value, std::type_index _type,
                 std::type_index _componentType)
      : name(_name), value(_value), type(_type), componentType(_componentType) {
  }
  std::string name;
  void *value;
  std::type_index type;
  std::type_index componentType;
};

void imguiDataPanel(EditorDataItem data) {
  if (data.type == typeid(float)) {
    ImGui::InputFloat(std::format("##{}float", data.name).c_str(), static_cast<float *>(data.value));
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
}
