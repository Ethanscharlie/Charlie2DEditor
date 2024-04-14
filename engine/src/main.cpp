#include "imgui_internal.h"

#include "Charlie2D.h"
#include "ResourceManager.h"
#include "imguiDataPanels.h"
#include "include_tmp.h"
#include "nlohmann/json.hpp"
#include <charlie2D/ImGuiFileDialog.h>
#include <complex>
#include <filesystem>
#include <sstream>

#define PROJECT_PATH _PROJECT_PATH

#ifdef FINAL_BUILD

int main(int argc, char *argv[]) {
  projectFolderpath = PROJECT_PATH;
  // projectFilepath =
  //     std::filesystem::path(projectFolderpath) / "Project.ch2dscene";

  // GameManager::init();
  //
  // std::ifstream file("Project.ch2dscene");
  // json jsonData = json::parse(file);
  // file.close();
  //
  // deserializeList(jsonData, true);
  //
  // GameManager::doUpdateLoop();
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

json editorTempRunSave;
bool inRunState = false;
std::vector<Entity *> usedChildren;
std::stringstream buffer;

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
  std::cout << "Set prev path as " << projectFolderpath << "\n";
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
  bool checkEntityIsEngine(Entity *entity) {
    if (entity->tag.size() > 0) {
      if (entity->tag[0] == '$')
        return true;
    }

    return false;
  }

  bool checkEntityIsEngine(std::string tag) {
    if (tag.size() > 0) {
      if (tag[0] == '$')
        return true;
    }

    return false;
  }

  void start() override {
    entity->useLayer = true;
    entity->layer = 100;

    std::ifstream file(std::filesystem::path(projectFolderpath) /
                       getEditorData()["scene"]);
    json jsonData = json::parse(file);
    file.close();

    deserializeList(jsonData, false);
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

  void makeEntityList() {
    // ImGui::BeginChild("Items", ImVec2(150, 0),
    //                   ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX);

    std::map<std::string, std::map<std::string, std::vector<Entity *>>>
        entitiesWithGroups;

    for (auto [tag, entityList] : GameManager::entities) {
      if (checkEntityIsEngine(tag))
        continue;
      if (entityList.size() <= 0)
        continue;

      for (Entity *entity : entityList) {
        entitiesWithGroups[tag][entity->group].push_back(entity);
      }
    }

    for (auto [tag, groups] : entitiesWithGroups) {
      std::string setTag = tag;
      if (tag == "")
        setTag = "None";
      if (ImGui::TreeNodeEx(setTag.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
        for (auto [group, entities] : groups) {
          if (group != "") {
            if (ImGui::TreeNodeEx(group.c_str(),
                                  ImGuiTreeNodeFlags_DefaultOpen)) {
              for (Entity *entity : entities) {
                std::string text =
                    std::format("{} ({})", entity->name, entity->iid);
                if (ImGui::Selectable(text.c_str(), entity == selectedEntity)) {
                  changeSelectedEntity(entity);
                }
              }
              ImGui::TreePop();
            }
          } else {
            for (Entity *entity : entities) {
              std::string text =
                  std::format("{} ({})", entity->name, entity->iid);
              if (ImGui::Selectable(text.c_str(), entity == selectedEntity)) {
                changeSelectedEntity(entity);
              }
            }
          }
        }
        ImGui::TreePop();
      }
    }

    // ImGui::EndChild();
  }

  void makeMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Recompile")) {
          std::exit(42);
        }
        if (ImGui::MenuItem("Refresh Assets")) {
          refreshAssets();
        }
        if (ImGui::MenuItem("New")) {
          IGFD::FileDialogConfig config;
          config.path = "/home";
          ImGuiFileDialog::Instance()->OpenDialog(
              "NewProject", "Choose Project Folder", nullptr, config);
        }
        if (ImGui::MenuItem("Save") && !inRunState) {
          json jsonEntitesData = serializeAllEntities();
          json editorData = getEditorData();

          changeEditorData(editorData);

          std::ofstream file(std::filesystem::path(projectFolderpath) /
                             editorData["scene"]);
          file << std::setw(2) << jsonEntitesData << std::endl;
          file.close();
          std::cout << "Saved to "
                    << std::filesystem::path(projectFolderpath) /
                           editorData["scene"]
                    << "\n";
        }
        if (ImGui::MenuItem("Open") && !inRunState) {
          IGFD::FileDialogConfig config;
          config.path = projectFolderpath;
          ImGuiFileDialog::Instance()->OpenDialog(
              "ChooseProject", "Choose File", nullptr, config);
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
            Camera::resetCamera();
            selectedEntity = nullptr;
            for (TransformEdit *component :
                 GameManager::getComponents<TransformEdit>()) {
              component->entity->remove<TransformEdit>();
            }

            editorTempRunSave = serializeAllEntities();

            for (Entity *dentity : GameManager::getAllObjects()) {
              if (checkEntityIsEngine(dentity))
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
              if (checkEntityIsEngine(entity))
                continue;
              entity->toDestroy = true;
            }
            deserializeList(editorTempRunSave, false);
          }
        }
        ImGui::EndMenu();
      }

      ImGui::EndMainMenuBar();
    }
  }

  void makeTopRowButtons() {
    if (ImGui::Button("Destroy")) {
      selectedEntity->toDestroy = true;
      selectedEntity = nullptr;
    }

    ImGui::SameLine();

    if (ImGui::Button("Duplicate")) {
      json jsonData = serialize(selectedEntity);
      Entity *dentity = deserialize(jsonData);

      std::random_device dev;
      std::mt19937 rng(dev());
      std::uniform_int_distribution<std::mt19937::result_type> dist(1, 100000);
      dentity->iid = dist(rng);

      changeSelectedEntity(dentity);
    }

    ImGui::SameLine();

    if (ImGui::Button("Make Collection")) {
      IGFD::FileDialogConfig config;
      config.path = projectFolderpath;
      ImGuiFileDialog::Instance()->OpenDialog("MakePrefab", "Choose File",
                                              ".ch2dsec", config);
    }

    if (ImGuiFileDialog::Instance()->Display(
            "MakePrefab", ImGuiWindowFlags_NoCollapse, ImVec2(1000, 700))) {
      if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
        std::string filepath = ImGuiFileDialog::Instance()->GetFilePathName();
        std::cout << filepath << "\n";

        json jsonData = serialize(selectedEntity);
        std::ofstream file(filepath);
        file << std::setw(2) << jsonData << std::endl;
        file.close();
      }

      // close
      ImGuiFileDialog::Instance()->Close();
    }
  }

  json getEditorData() {
    std::ifstream file(std::filesystem::path(projectFolderpath) /
                       "EditorData.json");
    json jsonData = json::parse(file);
    file.close();
    return jsonData;
  }

  void changeEditorData(json jsonData) {
    std::ofstream file(std::filesystem::path(projectFolderpath) /
                       "EditorData.json");
    file << std::setw(2) << jsonData << std::endl;
    file.close();
  }

  json getMainScene() {
    std::ifstream file(getEditorData()["scene"]);
    json jsonData = json::parse(file);
    file.close();
    return jsonData;
  }

  void changeMainScene(std::filesystem::path newScenePath) {
    json newEditorData = getEditorData();
    newEditorData["scene"] = newScenePath;
    changeEditorData(newEditorData);
  }

  bool setupDockspaces = true;

  void update() override {
    SDL_RenderSetLogicalSize(GameManager::renderer,
                             GameManager::currentWindowSize.x,
                             GameManager::currentWindowSize.y);

    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();

    ImGui::NewFrame();

    // // Create the dockspace
    ImGui::PushStyleColor(
        ImGuiCol_DockingEmptyBg,
        ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // Transparent background
    ImGui::PushStyleColor(
        ImGuiCol_WindowBg,
        ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // Transparent background for
    ImGui::PushStyleColor(
        ImGuiCol_Border,
        ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // Transparent border color
    ImGuiID dockspaceId =
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
    ImGui::PopStyleColor(3); // Pop the three style colors that were pushed

    if (setupDockspaces) {
      auto dock_id_left = ImGui::DockBuilderSplitNode(
          dockspaceId, ImGuiDir_Left, 0.2f, nullptr, &dockspaceId);
      auto dock_id_right = ImGui::DockBuilderSplitNode(
          dockspaceId, ImGuiDir_Right, 0.2f, nullptr, &dockspaceId);
      auto dock_id_up = ImGui::DockBuilderSplitNode(
          dockspaceId, ImGuiDir_Up, 0.2f, nullptr, &dockspaceId);
      auto dock_id_down = ImGui::DockBuilderSplitNode(
          dockspaceId, ImGuiDir_Down, 0.2f, nullptr, &dockspaceId);

      ImGui::DockBuilderDockWindow("Entities", dock_id_left);
      ImGui::DockBuilderDockWindow("Edit", dock_id_right);
      ImGui::DockBuilderDockWindow("Console", dock_id_down);

      ImGui::DockBuilderFinish(dockspaceId);
      setupDockspaces = false;
    }

    // auto dock_id_left = ImGui::DockBuilderSplitNode(
    //     dockspaceId, ImGuiDir_Left, 0.2f, nullptr, &dockspaceId);
    // auto dock_id_right = ImGui::DockBuilderSplitNode(
    //     dockspaceId, ImGuiDir_Right, 0.2f, nullptr, &dockspaceId);
    // auto dock_id_up = ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Up,
    //                                               0.2f, nullptr,
    //                                               &dockspaceId);
    // auto dock_id_down = ImGui::DockBuilderSplitNode(
    //     dockspaceId, ImGuiDir_Down, 0.2f, nullptr, &dockspaceId);

    // Entities list frame
    ImGui::Begin("Entities");

    makeMenuBar();

    if (ImGuiFileDialog::Instance()->Display(
            "NewProject", ImGuiWindowFlags_NoCollapse, ImVec2(1000, 700))) {
      if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
        projectFolderpath = ImGuiFileDialog::Instance()->GetCurrentPath();
        std::string mainRelativeScenePath = "img/Scenes/main.ch2d";

        std::filesystem::create_directory(projectFolderpath);

        // Create Editor Data file
        json newProjectJsonData;
        newProjectJsonData["name"] = "New Charlie2D Project";
        newProjectJsonData["scene"] = mainRelativeScenePath;
        std::ofstream file(std::filesystem::path(projectFolderpath) /
                           "EditorData.json");
        file << std::setw(2) << newProjectJsonData << std::endl;
        file.close();

        // Create subdirectories
        std::filesystem::create_directory(
            std::filesystem::path(projectFolderpath) / "img");
        std::filesystem::create_directory(
            std::filesystem::path(projectFolderpath) / "src");

        std::filesystem::create_directory(
            std::filesystem::path(projectFolderpath) / "img" / "Scenes");

        // Create the main starting scene file
        json jsonData;
        jsonData["Scene"];
        std::ofstream mainSceneFile(std::filesystem::path(projectFolderpath) /
                                    mainRelativeScenePath);
        mainSceneFile << std::setw(2) << jsonData << std::endl;
        mainSceneFile.close();

        // Remove all current entities
        for (Entity *entity : GameManager::getAllObjects()) {
          if (checkEntityIsEngine(entity))
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

        std::ifstream file(std::filesystem::path(projectFolderpath) /
                           getEditorData()["scene"]);
        json jsonData = json::parse(file);
        file.close();

        selectedEntity = nullptr;
        for (Entity *entity : GameManager::getAllObjects()) {
          if (checkEntityIsEngine(entity))
            continue;
          entity->toDestroy = true;
        }
        deserializeList(jsonData, false);

        writePrevProject();
      }

      // close
      ImGuiFileDialog::Instance()->Close();
    }

    ImGui::Text(projectFolderpath.c_str());

    if (ImGui::Button("Create")) {
      Entity *newEntity = GameManager::createEntity("");
      newEntity->name = "New Entity";
    }

    makeEntityList();

    ImGui::End();

    ImGui::Begin("Edit");

    ImGui::BeginGroup();
    if (selectedEntity != nullptr) {
      Box box = selectedEntity->box->getBox();

      ImGui::Text("Name");
      ImGui::SameLine();
      ImGui::InputString("##entityname", &selectedEntity->name);

      ImGui::Text("Tag");
      ImGui::SameLine();
      std::string ctag = selectedEntity->tag;
      std::string prevTag = ctag;
      ImGui::InputString("##entitytag", &ctag);
      if (ctag != prevTag) {
        selectedEntity->changeTag(ctag);
      }

      ImGui::Text("Group");
      ImGui::SameLine();
      ImGui::InputString("##entitygroup", &selectedEntity->group);

      makeTopRowButtons();
      if (selectedEntity == nullptr) {
        ImGui::EndGroup();
        goto FRAME_END;
      }

      ImGui::InputInt("Layer###EntityLayerInput", &selectedEntity->layer);

      ImGui::Text(
          std::format("Center {}, {}", box.getCenter().x, box.getCenter().y)
              .c_str());

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

    ImGui::Begin("Console");

    ImGui::Text(newlineInvertString(buffer.str()).c_str());

    ImGui::End();

    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());

    SDL_RenderSetLogicalSize(GameManager::renderer,
                             GameManager::gameWindowSize.x,
                             GameManager::gameWindowSize.y);
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

  float sliderexample = 0;
};

class CameraMover : public ExtendedComponent {
public:
  void update() override {
    if (!inRunState) {
      Camera::position +=
          InputManager::checkAxis() * speed * GameManager::deltaTime;

      Camera::scale += InputManager::mouseScroll * 0.10;

      if (InputManager::checkInput("jumpTrigger")) {
        mouseStartOnHold = InputManager::getMouseWorldPosition();
      }
      if (InputManager::checkInput("jump")) {
        if (InputManager::getMouseWorldPosition() != mouseStartOnHold) {
          Camera::position +=
              mouseStartOnHold - InputManager::getMouseWorldPosition();
          mouseStartOnHold = InputManager::getMouseWorldPosition();
        }
      }

      if (InputManager::keys[SDLK_f] && selectedEntity != nullptr) {
        Camera::setPosition(selectedEntity->box->getBox().getCenter());
      }
    }
  }

private:
  const float speed = 400.0f;
  Vector2f mouseStartOnHold;
};

int main(int argc, char *argv[]) {
  std::cout.rdbuf(buffer.rdbuf());

  projectFolderpath = PROJECT_PATH;
  // projectFilepath =
  //     std::filesystem::path(projectFolderpath) / "Project.ch2dscene";

  GameManager::init();

  SetupImGuiStyle();

  GameManager::createEntity("$EntitiesPanel")->add<EntitiesPanel>();
  GameManager::createEntity("$CameraMove")->add<CameraMover>();

  GameManager::doUpdateLoop();
  return 0;
}

#endif