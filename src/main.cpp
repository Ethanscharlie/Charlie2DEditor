#include "Charlie2D.h"
#include "imguiDataPanels.h"
#include "include_tmp.h"
#include <sstream>
#include <vector>

#define PROJECT_PATH _PROJECT_PATH

#ifdef FINAL_BUILD

int main(int argc, char *argv[]) {
  projectFolderpath = PROJECT_PATH;
  projectFilepath =
      std::filesystem::path(projectFolderpath) / "Project.ch2dscene";

  GameManager::init();

  std::cout << "Hello World\n";

  std::ifstream file("Project.ch2dscene");
  json jsonData = json::parse(file);
  file.close();

  std::cout << std::setw(2) << jsonData << std::endl;

  for (json entityGroup : jsonData["Scene"]) {
    for (json entityJson : entityGroup) {
      Entity *dentity = deserialize(entityJson);

      for (auto &[type, component] : dentity->components) {
        component->start();
      }
    }
  }

  GameManager::doUpdateLoop();
  return 0;
}

#else
#include "Component.h"
#include "Entity.h"
#include "ExtendedComponent.h"
#include "GameManager.h"
#include "ImGuiFileDialog.h"
#include "InputManager.h"
#include "LDTKEntity.h"
#include "Serializer.h"
#include "Vector2f.h"
#include "imgui.h"
#include "move.h"
#include <cstdlib>
#include <format>
#include <fstream>
#include <string>
#include <typeindex>

#include "imguiTheme.h"

Entity *selectedEntity = nullptr;
json editorTempRunSave;
bool inRunState = false;
std::vector<Entity *> usedChildren;

json serializeAllEntities() {
  json entitiesListJson;
  for (Entity *entity : GameManager::getAllObjects()) {
    if (entity->tag[0] == '$')
      continue;
    entitiesListJson["Scene"][entity->tag].push_back(serialize(entity));
  }

  return entitiesListJson;
}

void writePrevProject() {
  std::ofstream file("../prevProject.txt");
  file << projectFolderpath << std::endl;
  file.close();
}

void changeSelectedEntity(Entity *entity) {
  if (entity->tag[0] == '$')
    return;
  if (selectedEntity != nullptr) {
    selectedEntity->remove<TransformEdit>();
  }
  selectedEntity = entity;
  selectedEntity->add<TransformEdit>();
}

void makeEntitySelectabe(Entity *entity) {
  ImGui::SameLine();
  std::string text = std::format("{} ({})", entity->tag, entity->iid);
  if (ImGui::Selectable(text.c_str(), entity == selectedEntity)) {
    changeSelectedEntity(entity);
  }
}

void makeEntityTreeNode(Entity *entity) {
  if (entity->tag[0] == '$')
    return;

  if (ImGui::TreeNode(std::format("###treelabel{}", entity->iid).c_str())) {
    makeEntitySelectabe(entity);
    for (Entity *child : entity->getChildren()) {
      makeEntityTreeNode(child);
      usedChildren.push_back(child);
    }
    ImGui::TreePop();
  } else {
    makeEntitySelectabe(entity);
  }
}

class ClickOnEntityListener : public ExtendedComponent {
public:
  void update() override {
    Vector2f mousePos = InputManager::getMouseWorldPosition();
    Box box = entity->box->getBox();
    if (InputManager::rightClick && mousePos.x < box.getRight() &&
        mousePos.x > box.getLeft() && mousePos.y > box.getTop() &&
        mousePos.y < box.getBottom()) {
      changeSelectedEntity(entity);
    }
  }
};

class EntitiesPanel : public ExtendedComponent {
public:
  void start() override {
    entity->useLayer = true;
    entity->layer = 100;
  }

  void collectChildren(Entity *entity) {
    if (entity->getParent() != nullptr) {
      usedChildren.push_back(entity);
    }
    for (Entity *child : entity->getChildren()) {
      collectChildren(child);
    }
  }

  void update() override {
    SDL_RenderSetLogicalSize(GameManager::renderer,
                             GameManager::currentWindowSize.x,
                             GameManager::currentWindowSize.y);

    // ImGui_ImplSDLRenderer2_NewFrame();
    // ImGui_ImplSDL2_NewFrame();
    // ImGui::NewFrame();
    // ImGui::ShowDemoWindow();
    // ImGui::Render();
    // ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());

    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();

    ImGui::NewFrame();
    ImGui::SetNextWindowSize(ImVec2(600, 1000), ImGuiCond_FirstUseEver);
    ImGui::Begin("Entities");

    if (ImGui::BeginMainMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Recompile")) {
          std::exit(42);
        }
        if (ImGui::MenuItem("New")) {
          IGFD::FileDialogConfig config;
          config.path = projectFolderpath;
          ImGuiFileDialog::Instance()->OpenDialog("NewProject", "Choose Folder",
                                                  nullptr, config);
        }
        if (ImGui::MenuItem("Save") && !inRunState) {
          json jsonData = serializeAllEntities();

          std::ofstream file(projectFilepath);
          file << std::setw(2) << jsonData << std::endl;
          file.close();
        }
        if (ImGui::MenuItem("Open") && !inRunState) {
          IGFD::FileDialogConfig config;
          config.path = projectFolderpath;
          ImGuiFileDialog::Instance()->OpenDialog(
              "ChooseProject", "Choose File", ".ch2dscene", config);
        }
        if (ImGui::MenuItem("Exit")) {
          std::exit(1);
        }
        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Play")) {
        if (ImGui::MenuItem("Run")) {
          if (!inRunState) {
            inRunState = true;
            selectedEntity = nullptr;
            for (TransformEdit *component :
                 GameManager::getComponents<TransformEdit>()) {
              component->entity->remove<TransformEdit>();
            }

            editorTempRunSave = serializeAllEntities();

            for (Entity *dentity : GameManager::getAllObjects()) {
              if (dentity->tag[0] == '$')
                continue;
              for (auto &[type, component] : dentity->components) {
                component->start();
                component->standardUpdate = true;
              }
            }
          }
        }
        if (ImGui::MenuItem("Stop")) {
          if (inRunState) {
            inRunState = false;
            selectedEntity = nullptr;
            for (Entity *entity : GameManager::getAllObjects()) {
              if (entity->tag[0] == '$')
                continue;
              entity->toDestroy = true;
            }
            for (json entityGroup : editorTempRunSave["Scene"]) {
              for (json entityJson : entityGroup) {
                Entity *dentity = deserialize(entityJson);

                for (auto &[type, component] : dentity->components) {
                  if (!component->typeIsRendering)
                    component->standardUpdate = false;
                }
              }
            }
          }
        }
        ImGui::EndMenu();
      }

      ImGui::EndMainMenuBar();
    }

    if (ImGuiFileDialog::Instance()->Display(
            "NewProject", ImGuiWindowFlags_NoCollapse, ImVec2(1000, 700))) {
      if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
        projectFolderpath = std::filesystem::path(
                                ImGuiFileDialog::Instance()->GetCurrentPath()) /
                            "Project";
        projectFilepath =
            std::filesystem::path(projectFolderpath) / "Project.ch2dscene";

        std::filesystem::create_directory(projectFolderpath);

        std::filesystem::create_directory(
            std::filesystem::path(projectFolderpath) / "img");
        std::filesystem::create_directory(
            std::filesystem::path(projectFolderpath) / "src");

        json jsonData;
        jsonData["Scene"];
        std::ofstream file(projectFilepath);
        file << std::setw(2) << jsonData << std::endl;
        file.close();

        for (Entity *entity : GameManager::getAllObjects()) {
          if (entity->tag[0] == '$')
            continue;
          entity->toDestroy = true;
        }

        writePrevProject();
      }

      ImGuiFileDialog::Instance()->Close();
    }

    if (ImGuiFileDialog::Instance()->Display(
            "ChooseProject", ImGuiWindowFlags_NoCollapse, ImVec2(1000, 700))) {
      if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
        projectFolderpath = ImGuiFileDialog::Instance()->GetCurrentPath();
        projectFilepath =
            std::filesystem::path(projectFolderpath) / "Project.ch2dscene";

        std::ifstream file(projectFilepath);
        json jsonData = json::parse(file);
        file.close();

        std::cout << std::setw(2) << jsonData << std::endl;

        selectedEntity = nullptr;
        for (Entity *entity : GameManager::getAllObjects()) {
          if (entity->tag[0] == '$')
            continue;
          entity->toDestroy = true;
        }
        for (json entityGroup : jsonData["Scene"]) {
          for (json entityJson : entityGroup) {
            Entity *dentity = deserialize(entityJson);

            for (auto &[type, component] : dentity->components) {
              if (!component->typeIsRendering)
                component->standardUpdate = false;
            }
          }
        }

        writePrevProject();
      }

      // close
      ImGuiFileDialog::Instance()->Close();
    }

    if (ImGui::Button("Create")) {
      GameManager::createEntity("New Entity");
    }

    // Full entity list
    ImGui::BeginChild("Items", ImVec2(150, 0),
                      ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);

    int elementIndex = 0;
    auto all = GameManager::getAllObjects();
    usedChildren.clear();

    for (Entity *entity : all) {
      collectChildren(entity);
    }

    for (auto it = all.begin(); it != all.end(); it++) {
      Entity *entity = *it;

      if (entity->tag[0] == '$')
        continue;
      if (std::find(usedChildren.begin(), usedChildren.end(), entity) !=
          usedChildren.end())
        continue;

      const std::string &tag = std::format("{} ({})", entity->tag, entity->iid);

      elementIndex++;
      makeEntityTreeNode(entity);
    }

    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginGroup();
    if (selectedEntity != nullptr) {
      Box box = selectedEntity->box->getBox();

      std::string ctag = selectedEntity->tag;
      std::string prevTag = ctag;
      ImGui::InputString("##entitytag", &ctag);
      if (ctag != prevTag) {
        selectedEntity->changeTag(ctag);
      }

      if (ImGui::Button("Destroy")) {
        selectedEntity->toDestroy = true;
        selectedEntity = nullptr;
        ImGui::EndGroup();
        goto FRAME_END;
      }

      ImGui::SameLine();

      if (ImGui::Button("Duplicate")) {
        json jsonData = serialize(selectedEntity);
        deserialize(jsonData);
      }

      ImGui::InputInt("Layer###EntityLayerInput", &selectedEntity->layer);

      ImGui::Text(
          std::format("Center {}, {}", box.getCenter().x, box.getCenter().y)
              .c_str());

      std::string parentName;
      if (selectedEntity->getParent() != nullptr) {
        parentName = selectedEntity->getParent()->tag;
      } else {
        parentName = "Select a Parent";
      }

      if (ImGui::BeginCombo("##comboparent", parentName.c_str())) {
        int i = 0;
        for (Entity *otherEntity : GameManager::getAllObjects()) {
          if (otherEntity == selectedEntity)
            continue;
          if (otherEntity->tag[0] == '$')
            continue;

          bool isChild = false;
          for (Entity *child : selectedEntity->getChildren()) {
            if (child == otherEntity) {
              isChild = true;
              break;
            }
          }
          if (isChild)
            continue;

          bool isSelected = false;
          std::string otherTag =
              std::format("{} ({})", otherEntity->tag, otherEntity->iid);
          const char *keyc = otherTag.c_str();
          if (ImGui::Selectable(keyc, &isSelected)) {
          }

          if (isSelected) {
            selectedEntity->setParent(otherEntity);
          }

          i++;
        }
        ImGui::EndCombo();
      }

      // ENTIY BOX
      ImGui::BeginChild("Entity Box Frame", ImVec2(0, 150),
                        ImGuiChildFlags_Border);
      ImGui::Text("Entity Box");

      Box ebox = selectedEntity->box->getBox();
      Box prevBox = ebox;

      PropertyData eboxData = {"Entity Box", &ebox};

      imguiDataPanel(eboxData);
      if (ebox.position.x != prevBox.position.x ||
          ebox.position.y != prevBox.position.y ||
          ebox.size.x != prevBox.size.x || ebox.size.y != prevBox.size.y) {
        selectedEntity->box->setPosition(ebox.position);
        selectedEntity->box->setSize(ebox.size);
      }

      ImGui::EndChild();
      // EBOXEND

      for (auto [type, component] : selectedEntity->components) {
        std::string nameWithoutType = getTypeNameWithoutNumbers(type);

        if (nameWithoutType == "TransformEdit")
          continue;
        if (nameWithoutType == "entityBox")
          continue;
        if (nameWithoutType == "ClickOnEntityListener")
          continue;

        if (ImGui::CollapsingHeader(nameWithoutType.c_str(),
                                    ImGuiTreeNodeFlags_DefaultOpen)) {
          if (ImGui::Button(
                  std::format("Remove### {} ComponentRemove", type.name())
                      .c_str())) {
            selectedEntity->remove(type);
            ImGui::EndGroup();
            goto FRAME_END;
          }

          for (PropertyData data : component->propertyRegister) {
            ImGui::Indent();
            ImGui::BeginChild(
                std::format("{} Frame", data.name).c_str(), ImVec2(0, 0),
                ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY);

            ImGui::Text(data.name.c_str());
            imguiDataPanel(data);

            ImGui::EndChild();
            ImGui::Unindent();
          }
        }
      }
      if (ImGui::BeginCombo("##combo", "Add a Component")) {

        for (auto &[key, component] : GameManager::componentRegistry) {
          bool isSelected = false;
          const char *keyc = key.c_str();
          if (ImGui::Selectable(keyc, &isSelected)) {
          }

          if (isSelected) {
            Component *component =
                GameManager::componentRegistry[key](selectedEntity);
            if (!component->typeIsRendering)
              component->standardUpdate = false;
          }
        }
        ImGui::EndCombo();
      }
    } else {
      ImGui::Text("No entity selected");
    }
    ImGui::EndGroup();

  FRAME_END:
    ImGui::End();
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());

    SDL_RenderSetLogicalSize(GameManager::renderer,
                             GameManager::gameWindowSize.x,
                             GameManager::gameWindowSize.y);
  }

  float sliderexample = 0;
};

class CameraMover : public ExtendedComponent {
public:
  void update() override {
    if (!inRunState) {
      Camera::position +=
          InputManager::checkAxis() * speed * GameManager::deltaTime;
    }
  }

  const float speed = 400.0f;
};

int main(int argc, char *argv[]) {
  projectFolderpath = PROJECT_PATH;
  projectFilepath =
      std::filesystem::path(projectFolderpath) / "Project.ch2dscene";

  GameManager::init();

  SetupImGuiStyle();

  GameManager::createEntity("$EntitiesPanel")->add<EntitiesPanel>();
  GameManager::createEntity("$CameraMove")->add<CameraMover>();
  std::ifstream file(projectFilepath);
  json jsonData = json::parse(file);
  file.close();

  std::cout << std::setw(2) << jsonData << std::endl;

  for (json entityGroup : jsonData["Scene"]) {
    for (json entityJson : entityGroup) {
      Entity *dentity = deserialize(entityJson);

      for (auto &[type, component] : dentity->components) {
        if (!component->typeIsRendering)
          component->standardUpdate = false;
      }
    }
  }

  GameManager::doUpdateLoop();
  return 0;
}

#endif
