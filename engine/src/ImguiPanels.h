#pragma once
#include "Component.h"
#include "Entity.h"
#include "ExtendedComponent.h"
#include "GameManager.h"
#include "ImGuiFileDialog.h"
#include "LDTKEntity.h"
#include <string>

#include "Entity.h"
#include "ExtendedComponent.h"
#include "GameManager.h"
#include "ImGuiFileDialog.h"
#include "json.hpp"
#include <string>

using json = nlohmann::json;

enum class ExportTypes;

class EntitiesPanel : public ExtendedComponent {
public:
  void checkHotkeys();
  bool checkEntityIsEngine(Entity *entity);
  bool checkEntityIsEngine(std::string tag);
  void makeEntityList();
  void makeMenuBar();
  void makeTopRowButtons();

  void start() override;
  void update() override;

private:
  bool setupDockspaces = true;
  bool showPanels = true;
  ExportTypes exportType;
};
