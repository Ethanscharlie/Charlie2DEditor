#include "imgui_internal.h"

#include "Charlie2D.h"
#include "include_tmp.h"
#include <sstream>

#define PROJECT_PATH _PROJECT_PATH

#ifdef FINAL_BUILD

int main(int argc, char *argv[]) {
  std::filesystem::path projectFolderpath = PROJECT_PATH;
  GameManager::init();

  std::ifstream editorDataFile("EditorData.json");
  json editorJsonData = json::parse(editorDataFile);
  editorDataFile.close();

  std::string scenePath = editorJsonData["scene"];
  std::ifstream file(scenePath);
  json jsonData = json::parse(file);
  file.close();

  deserializeList(jsonData, true);

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
#include "Vector2f.h"
#include "functionUtils.h"
#include <string>
#include "sharedHeader.h"

#include "ImguiPanels.h"
#include "imguiTheme.h"
#include "move2.h"

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

class MoveTool;

int main(int argc, char *argv[]) {
  // std::cout.rdbuf(buffer.rdbuf());

  projectFolderpath = PROJECT_PATH;
  // projectFilepath =
  //     std::filesystem::path(projectFolderpath) / "Project.ch2dscene";

  GameManager::init();

  SetupImGuiStyle();

  GameManager::createEntity("$EntitiesPanel")->add<EntitiesPanel>();
  GameManager::createEntity("$CameraMove")->add<CameraMover>();
  GameManager::createEntity("$MoveTool")->add<MoveTool>();

  GameManager::doUpdateLoop();

  std::exit(EXTCODE_EXIT);
  return 0;
}

#endif
