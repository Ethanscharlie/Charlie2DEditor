#include "Camera.h"
#include "Charlie2D.h"
#include "Component.h"
#include "ExtendedComponent.h"
#include "GameManager.h"
#include "InputManager.h"
#include "Vector2f.h"
#include "imgui.h"
#include "move.h"
#include <any>
#include <format>
#include <string>
#include <typeindex>

struct EditorDataItem {
  EditorDataItem(std::string _name, void *_value, std::type_index _type)
      : name(_name), value(_value), type(_type) {}
  std::string name;
  void *value;
  std::type_index type;
};

class EditorData : public ExtendedComponent {
public:
  template <typename T> void registerComponent(std::string name, T *number) {
    data.push_back(EditorDataItem(name, number, typeid(T)));
  }

  std::vector<EditorDataItem> data;
};

Entity *selectedEntity = nullptr;

class Player : public ExtendedComponent {
public:
  void start() override {
    entity->add<EditorData>()->registerComponent("Speed", &speed);
  }

  Vector2f speed = {10, 10};
};

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
    ImGui_ImplSDL2_NewFrame(GameManager::window);

    ImGui::NewFrame();
    ImGui::Begin("Entities");

    ImGui::BeginChild("Items", ImVec2(150, 0),
                      ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);

    for (Entity *entity : GameManager::getAllObjects()) {
      const std::string &tag = entity->tag;

      if (tag[0] == '$')
        continue;
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
      ImGui::Text(std::format("Center {}, {}\nSize {}, {}", box.getCenter().x,
                              box.getCenter().y, box.size.x, box.size.y)
                      .c_str());

      for (auto data : selectedEntity->get<EditorData>()->data) {
        ImGui::BeginChild("Inner Frame", ImVec2(0, 100), true);
        ImGui::Text(data.name.c_str);
        if (data.type == typeid(float)) {
          ImGui::InputFloat(data.name.c_str(),
                            static_cast<float *>(data.value));
        }

        else if (data.type == typeid(Vector2f)) {
          Vector2f *vector = static_cast<Vector2f *>(data.value);
          ImGui::InputFloat("x", static_cast<float *>(&vector->x));
          ImGui::SameLine();
          ImGui::InputFloat("y", static_cast<float *>(&vector->y));
        }
        ImGui::EndChild();
      }
    } else {
      ImGui::Text("No entity selected");
    }
    ImGui::EndGroup();

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

int main() {
  GameManager::init();

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
    entity->add<EditorData>();
  }

  GameManager::doUpdateLoop();
  return 0;
}
