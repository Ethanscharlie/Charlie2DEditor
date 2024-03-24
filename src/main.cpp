#include "Camera.h"
#include "Charlie2D.h"
#include "Component.h"
#include "Entity.h"
#include "ExtendedComponent.h"
#include "GameManager.h"
#include "InputManager.h"
#include "LDTKEntity.h"
#include "Serializer.h"
#include "Vector2f.h"
#include "imgui.h"
#include "move.h"
#include <any>
#include <cstdlib>
#include <format>
#include <fstream>
#include <string>
#include <typeindex>

#include "imguiDataPanels.h"
#include "imguiTheme.h"

#include "include_tmp.h"

Entity *selectedEntity = nullptr;

json serializeAllEntities() {
  json entitiesListJson;
  for (Entity *entity : GameManager::getAllObjects()) {
    if (entity->tag[0] == '$')
      continue;
    entitiesListJson["Scene"][entity->tag].push_back(serialize(entity));
  }

  return entitiesListJson;
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
  void start() override {
    entity->useLayer = true;
    entity->layer = 100;
  }

  void update() override {
    SDL_RenderSetLogicalSize(GameManager::renderer,
                             GameManager::currentWindowSize.x,
                             GameManager::currentWindowSize.y);

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
        if (ImGui::MenuItem("Save")) {
          json jsonData = serializeAllEntities();

          std::ofstream file("../Project.ch2dscene");
          file << std::setw(2) << jsonData << std::endl;
          file.close();
        }
        if (ImGui::MenuItem("Open")) {
          std::ifstream file("../Project.ch2dscene");
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
              deserialize(entityJson);
            }
          }
        }
        if (ImGui::MenuItem("Exit")) {
          std::exit(1);
        }
        ImGui::EndMenu();
      }
      ImGui::EndMainMenuBar();
    }

    if (ImGui::Button("Create")) {
      GameManager::createEntity("New Entity");
    }

    ImGui::BeginChild("Items", ImVec2(150, 0),
                      ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);

    int elementIndex = 0;
    auto all = GameManager::getAllObjects();
    for (auto it = all.begin(); it != all.end(); it++) {
      Entity *entity = *it;
      const std::string &tag =
          std::format("{} {}", entity->tag, std::to_string(elementIndex));

      if (tag[0] == '$')
        continue;

      elementIndex++;
      if (ImGui::Selectable(tag.c_str(), selectedEntity == entity)) {
        changeSelectedEntity(entity);
      }
    }

    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginGroup();
    if (selectedEntity != nullptr) {
      Box box = selectedEntity->box->getBox();
      ImGui::Text(selectedEntity->tag.c_str());

      ImGui::SameLine();
      if (ImGui::Button("Destroy")) {
        selectedEntity->toDestroy = true;
        selectedEntity = nullptr;
        ImGui::EndGroup();
        goto FRAME_END;
      }

      ImGui::Text(std::format("Center {}, {}\nSize {}, {}", box.getCenter().x,
                              box.getCenter().y, box.size.x, box.size.y)
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

        if (nameWithoutType == "TransformEdit") continue;
        if (nameWithoutType == "entityBox") continue;
        if (nameWithoutType == "ClickOnEntityListener") continue;

        if (ImGui::CollapsingHeader(nameWithoutType.c_str(),
                                    ImGuiTreeNodeFlags_DefaultOpen)) {
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
            GameManager::componentRegistry[key](selectedEntity);
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
    Camera::position +=
        InputManager::checkAxis() * speed * GameManager::deltaTime;
  }

  const float speed = 100.0f;
};

class Player;

int main() {
  GameManager::init();

  SetupImGuiStyle();

  GameManager::createEntity("$EntitiesPanel")->add<EntitiesPanel>();
  GameManager::createEntity("$CameraMove")->add<CameraMover>();

  Entity *player = GameManager::createEntity("Player");
  player->add<Sprite>()->loadTexture("img/duck.png");
  player->get<entityBox>()->setWithCenter({0, 0});
  player->useLayer = true;
  player->layer = 5;
  player->add<Player>();

  Entity *background = GameManager::createEntity("Background");
  background->add<Sprite>()->loadTexture("img/backgroundwall.png");
  background->get<entityBox>()->setWithCenter({0, 0});
  background->get<entityBox>()->setScale(GameManager::gameWindowSize);
  background->useLayer = true;
  background->layer = -10;

  Entity *gun = GameManager::createEntity("Gun");
  gun->box->setScale({10, 10});

  Entity *enemy = GameManager::createEntity("Enemy");
  enemy->box->setScale({20, 20});

  for (Entity *entity : GameManager::getAllObjects()) {
    entity->add<ClickOnEntityListener>();
    entity->add<Sprite>()->showBorders = true;
  }

  GameManager::doUpdateLoop();
  return 0;
}
