#pragma once
#include "Charlie2D.h"
#include "SimplePanel.h"

class TransformEdit : public Component {
public:
  ~TransformEdit() {
    moveButton->toDestroy = true;
    horizontalScaleButton->toDestroy = true;
    verticalScaleButton->toDestroy = true;
  }

  void start() override {
    createMoveButton();
    createHorizontalScaleButton();
    createVerticalScaleButton();
  }

  void update(float deltaTime) override {
    if (busyHorizontalScale) {
      horizontalScaleButton->get<entityBox>()->setWithCenter(
          {InputManager::getMouseWorldPosition().x,
           horizontalScaleButton->get<entityBox>()->getBox().getCenter().y});
      entity->get<entityBox>()->setScale(
          {ogSize.x + InputManager::getMouseWorldPosition().x -
               horizontalScaleStartPos.x,
           ogSize.y});

      if (!InputManager::mouseHeld) {
        busyHorizontalScale = false;
        horizontalScaleButton->get<entityBox>()->setLocalWithCenter(
            horizontalScalePosition);
      }
    }

    if (busyverticalScale) {
      verticalScaleButton->get<entityBox>()->setWithCenter(
          {verticalScaleButton->get<entityBox>()->getBox().getCenter().x,
           InputManager::getMouseWorldPosition().y});
      entity->get<entityBox>()->setScale(
          {ogSize.x, ogSize.y + InputManager::getMouseWorldPosition().y -
                         verticalScaleStartPos.y});

      if (!InputManager::mouseHeld) {
        busyverticalScale = false;
        verticalScaleButton->get<entityBox>()->setLocalWithCenter(
            verticalScalePosition);
      }
    }
  }

  void createMoveButton() {
    moveButton = GameManager::createEntity("$TransformEditMoveButton");
    moveButton->setParent(entity);
    entityBox &box = *moveButton->get<entityBox>();
    box.anchor = 4;
    box.setScale({50, 50});
    box.setLocalWithCenter({0, 0});
    moveButton->add<SimplePanel>()->rendererInWorld = true;
    moveButton->get<SimplePanel>()->alpha = 120;
    moveButton->add<SimplePanel>()->setColor({0, 0, 255});
    Button &button = *moveButton->add<Button>();
    button.checkInWorld = true;
    button.onHold = [this]() {
      if (busyHorizontalScale)
        return;
      if (busyverticalScale)
        return;
      entity->get<entityBox>()->setWithCenter(
          InputManager::getMouseWorldPosition());
    };

    moveButton->layer = 90;
  }

  void createHorizontalScaleButton() {
    horizontalScaleButton = GameManager::createEntity("$horizontalScaleButton");
    horizontalScaleButton->setParent(entity);
    entityBox &box = *horizontalScaleButton->get<entityBox>();
    box.anchor = 4;
    box.setScale({50, 50});
    box.setLocalWithCenter(horizontalScalePosition);
    horizontalScaleButton->add<SimplePanel>()->rendererInWorld = true;
    horizontalScaleButton->get<SimplePanel>()->alpha = 120;

    horizontalScaleButton->add<SimplePanel>()->setColor({255, 0, 0});
    Button &button = *horizontalScaleButton->add<Button>();
    button.checkInWorld = true;
    button.onClick = [this]() {
      ogSize = entity->get<entityBox>()->getSize();
      horizontalScaleStartPos = InputManager::getMouseWorldPosition();
      busyHorizontalScale = true;
    };

    horizontalScaleButton->layer = 90;
  }

  void createVerticalScaleButton() {
    verticalScaleButton = GameManager::createEntity("$horizontalScaleButton");
    verticalScaleButton->setParent(entity);
    verticalScaleButton->add<SimplePanel>()->rendererInWorld = true;
    entityBox &box = *verticalScaleButton->get<entityBox>();
    box.anchor = 4;
    box.setScale({50, 50});
    box.setLocalWithCenter(verticalScalePosition);

    verticalScaleButton->add<SimplePanel>()->setColor({0, 255, 0});
    verticalScaleButton->get<SimplePanel>()->alpha = 120;
    Button &button = *verticalScaleButton->add<Button>();
    button.checkInWorld = true;
    button.onClick = [this]() {
      ogSize = entity->get<entityBox>()->getSize();
      verticalScaleStartPos = InputManager::getMouseWorldPosition();
      busyverticalScale = true;
    };

    verticalScaleButton->layer = 90;
  }

  Entity *moveButton;

  Entity *horizontalScaleButton;
  const Vector2f horizontalScalePosition = {70, 0};
  Vector2f horizontalScaleStartPos;
  bool busyHorizontalScale = false;

  Entity *verticalScaleButton;
  const Vector2f verticalScalePosition = {0, 70};
  Vector2f verticalScaleStartPos;
  bool busyverticalScale = false;

  Vector2f ogSize;
};
