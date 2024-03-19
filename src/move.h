#pragma once
#include "Charlie2D.h"
#include <algorithm>

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
          {InputManager::getMouseUIPosition().x,
           horizontalScaleButton->get<entityBox>()->getBox().getCenter().y});
      entity->get<entityBox>()->setScale(
          {ogSize.x + InputManager::getMouseUIPosition().x -
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
           InputManager::getMouseUIPosition().y});
      entity->get<entityBox>()->setScale(
          {ogSize.x, ogSize.y + InputManager::getMouseUIPosition().y -
                         verticalScaleStartPos.y});

      if (!InputManager::mouseHeld) {
        busyverticalScale = false;
        verticalScaleButton->get<entityBox>()->setLocalWithCenter(
            verticalScalePosition);
      }
    }
  }

  void createMoveButton() {
    // entity->remove<UISliceRenderer>();
    moveButton = GameManager::createEntity("$TransformEditMoveButton");
    moveButton->setParent(entity);
    entityBox &box = *moveButton->get<entityBox>();
    box.anchor = 4;
    box.setScale({70, 70});
    box.setLocalWithCenter({0, 0});
    moveButton->add<UISliceRenderer>()->rendererInWorld = true;
    //
    // moveButton->add<UISliceRenderer>()->rendererInWorld = false;
    moveButton->add<UISliceRenderer>()->setColor({0, 0, 255});
    Button &button = *moveButton->add<Button>();
    button.checkInWorld = true;
    button.onHold = [this]() {
      if (busyHorizontalScale)
        return;
      if (busyverticalScale)
        return;
      entity->get<entityBox>()->setWithCenter(
          InputManager::getMouseUIPosition());
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
    horizontalScaleButton->add<UISliceRenderer>()->rendererInWorld = true;

    // horizontalScaleButton->add<UISliceRenderer>()->rendererInWorld = false;
    horizontalScaleButton->add<UISliceRenderer>()->setColor({255, 0, 0});
    Button &button = *horizontalScaleButton->add<Button>();
    button.checkInWorld = true;
    button.onClick = [this]() {
      ogSize = entity->get<entityBox>()->getSize();
      horizontalScaleStartPos = InputManager::getMouseUIPosition();
      busyHorizontalScale = true;
    };

    horizontalScaleButton->layer = 90;
  }

  void createVerticalScaleButton() {
    verticalScaleButton = GameManager::createEntity("$horizontalScaleButton");
    verticalScaleButton->setParent(entity);
    verticalScaleButton->add<UISliceRenderer>()->rendererInWorld = true;
    entityBox &box = *verticalScaleButton->get<entityBox>();
    box.anchor = 4;
    box.setScale({50, 50});
    box.setLocalWithCenter(verticalScalePosition);

    // verticalScaleButton->add<UISliceRenderer>()->rendererInWorld = false;
    verticalScaleButton->add<UISliceRenderer>()->setColor({0, 255, 0});
    Button &button = *verticalScaleButton->add<Button>();
    button.checkInWorld = true;
    button.onClick = [this]() {
      ogSize = entity->get<entityBox>()->getSize();
      verticalScaleStartPos = InputManager::getMouseUIPosition();
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
