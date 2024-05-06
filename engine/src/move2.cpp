#include "move2.h"
#include "Entity.h"
#include "GameManager.h"
#include "SDL_render.h"
#include "SDL_scancode.h"
#include "functionUtils.h"
#include <cstdlib>

MoveTool::MoveTool() {
  typeIsRendering = true;

  cursorArrow = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
  cursorIBeam = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
  cursorWait = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAIT);
  cursorCrosshair = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
  cursorWaitArrow = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAITARROW);
  cursorSizeNWSE = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
  cursorSizeNESW = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
  cursorSizeWE = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
  cursorSizeNS = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
  cursorSizeAll = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
  cursorNo = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NO);
  cursorHand = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
}

MoveTool::~MoveTool() {
  SDL_FreeCursor(cursorArrow);
  SDL_FreeCursor(cursorIBeam);
  SDL_FreeCursor(cursorWait);
  SDL_FreeCursor(cursorCrosshair);
  SDL_FreeCursor(cursorWaitArrow);
  SDL_FreeCursor(cursorSizeNWSE);
  SDL_FreeCursor(cursorSizeNESW);
  SDL_FreeCursor(cursorSizeWE);
  SDL_FreeCursor(cursorSizeNS);
  SDL_FreeCursor(cursorSizeAll);
  SDL_FreeCursor(cursorNo);
  SDL_FreeCursor(cursorHand);
}

void MoveTool::start() { entity->layer = 90; }

Vector2f MoveTool::getCorrectMousePosition() {
  switch (selectedEntity->renderPositionType) {
  case EntityRenderPositionType::World:
    return InputManager::getMouseWorldPosition();
  case EntityRenderPositionType::Screen:
    return InputManager::getMouseUIPosition();
  }
}

void MoveTool::drawBoxLine(Box box, std::array<Uint8, 3> color, Uint8 alpha) {
  Box renderBox;
  Vector2f renderPos;

  switch (selectedEntity->renderPositionType) {
  case EntityRenderPositionType::World:
    renderPos = box.position - Camera::position;
    renderPos = renderPos * Camera::getScale();
    renderPos += GameManager::gameWindowSize / 2;
    renderBox = box;
    renderBox.position = renderPos;
    renderBox.size *= Camera::getScale();
    break;

  case EntityRenderPositionType::Screen:
    renderPos = box.position;
    renderPos += GameManager::gameWindowSize / 2;
    renderBox = box;
    renderBox.position = renderPos;
    renderBox.size *= Camera::getScale();
    break;
  }

  SDL_Rect renderRect = renderBox;

  SDL_SetRenderDrawColor(GameManager::renderer, color[0], color[1], color[2],
                         alpha);
  SDL_RenderFillRect(GameManager::renderer, &renderRect);
}

void MoveTool::drawBox(std::array<bool, 4> selectedSides) {
  // Calculate the scaled stroke once
  int scaledStroke = stroke / Camera::getScale();

  // Get the selectedEntity's bounding box
  Box selectedEntityBox = selectedEntity->box;

  // Calculate the positions and sizes of each side of the box
  int leftStroke = scaledStroke;
  if (selectedSides[0])
    leftStroke *= 2;
  leftBox.position = selectedEntityBox.position - (float)leftStroke / 2;
  leftBox.size.x = leftStroke;
  leftBox.size.y = selectedEntityBox.size.y + leftStroke;

  int rightStroke = scaledStroke;
  if (selectedSides[1])
    rightStroke *= 2;
  rightBox.position =
      selectedEntityBox.getTopRightCorner() - (float)rightStroke / 2;
  rightBox.size.x = rightStroke;
  rightBox.size.y = selectedEntityBox.size.y + rightStroke;

  int topStroke = scaledStroke;
  if (selectedSides[2])
    topStroke *= 2;
  topBox.position = selectedEntityBox.position - (float)topStroke / 2;
  topBox.size.x = selectedEntityBox.size.x + topStroke;
  topBox.size.y = topStroke;

  int bottomStroke = scaledStroke;
  if (selectedSides[3])
    bottomStroke *= 2;
  bottomBox.position =
      selectedEntityBox.getBottomLeftCorner() - (float)bottomStroke / 2;
  bottomBox.size.x = selectedEntityBox.size.x + bottomStroke;
  bottomBox.size.y = bottomStroke;

  // Draw each side of the box
  drawBoxLine(leftBox, color, 255);
  drawBoxLine(rightBox, color, 255);
  drawBoxLine(topBox, color, 255);
  drawBoxLine(bottomBox, color, 255);
}

void MoveTool::drawInsideMoveBox(bool hover, Box box) {
  SDL_SetRenderDrawBlendMode(GameManager::renderer, SDL_BLENDMODE_BLEND);
  Uint8 alpha = hover ? 100 : 200;
  drawBoxLine(box, {0, 240, 100}, alpha);
}

bool MoveTool::checkIfMouseInside(Box box) {
  Vector2f mousePos = getCorrectMousePosition();

  return (box.getLeft() < mousePos.x && box.getRight() > mousePos.x &&
          box.getTop() < mousePos.y && box.getBottom() > mousePos.y);
}

void MoveTool::handleSideDrag(LockDirection lockDirection, bool *resetCursor,
                              std::array<bool, 4> *selectedSides) {
  SDL_PumpEvents();
  const Uint8 *keyboardState = SDL_GetKeyboardState(NULL);

  Box box;

  Vector2f mousePosition = getCorrectMousePosition();

  switch (lockDirection) {
  case LockDirection::Left:
    box = leftBox;
    break;
  case LockDirection::Right:
    box = rightBox;
    break;
  case LockDirection::Top:
    box = topBox;
    break;
  case LockDirection::Bottom:
    box = bottomBox;
    break;
  case LockDirection::None:
    break;
  }

  if (checkIfMouseInside(box) || lock == lockDirection) {
    *resetCursor = false;
    if (lockDirection == LockDirection::Left ||
        lockDirection == LockDirection::Right) {
      SDL_SetCursor(cursorSizeWE);
    } else {
      SDL_SetCursor(cursorSizeNS);
    }

    // Make borders bigger when selected
    switch (lockDirection) {
    case LockDirection::Left:
      selectedSides->at(0) = true;
      if (InputManager::checkInput("dash"))
        selectedSides->at(1) = true;
      break;
    case LockDirection::Right:
      selectedSides->at(1) = true;
      if (InputManager::checkInput("dash"))
        selectedSides->at(0) = true;
      break;
    case LockDirection::Top:
      selectedSides->at(2) = true;
      if (InputManager::checkInput("dash"))
        selectedSides->at(3) = true;
      break;
    case LockDirection::Bottom:
      selectedSides->at(3) = true;
      if (InputManager::checkInput("dash"))
        selectedSides->at(2) = true;
      break;
    case LockDirection::None:
      break;
    }

    if (keyboardState[SDL_SCANCODE_LCTRL]) {
      selectedSides->at(0) = true;
      selectedSides->at(1) = true;
      selectedSides->at(2) = true;
      selectedSides->at(3) = true;
    }

    if (InputManager::mouseHeld) {
      if (lock != lockDirection)
        tempResizeBox = selectedEntity->box;
      lock = lockDirection;

      bool isSE = lock == LockDirection::Bottom || lock == LockDirection::Right;

      Vector2f tempCorner;
      if (isSE) {
        tempCorner = selectedEntity->box.position;
      } else {
        tempCorner = selectedEntity->box.getBottomRightCorner();
      }

      if (InputManager::checkInput("dash")) {
        if (multiLock != lockDirection) {
          multiLock = lockDirection;
          tempResizeBox = selectedEntity->box;
        }

        switch (lock) {
        case LockDirection::Left:
          selectedEntity->box.setScale(
              {(tempResizeBox.position.x - getCorrectMousePosition().x) * 2 +
                   tempResizeBox.size.x,
               selectedEntity->box.size.y});

          break;
        case LockDirection::Right:
          selectedEntity->box.setScale(
              {getCorrectMousePosition().x - selectedEntity->box.position.x,
               selectedEntity->box.size.y});
          break;
        case LockDirection::Top:
          selectedEntity->box.setScale(
              {selectedEntity->box.size.x,
               (tempResizeBox.position.y - getCorrectMousePosition().y) * 2 +
                   tempResizeBox.size.y});
          break;
        case LockDirection::Bottom:
          selectedEntity->box.setScale(
              {selectedEntity->box.size.x,
               getCorrectMousePosition().y - selectedEntity->box.position.y});
          break;
        case LockDirection::None:
          break;
        }
      }

      else if (keyboardState[SDL_SCANCODE_LCTRL]) {
        if (aspectLockLock != lockDirection) {
          aspectLockLock = lockDirection;
          tempResizeBox = selectedEntity->box;
        }

        float newSize;
        switch (lock) {
        case LockDirection::Left:
          newSize =
              (tempResizeBox.position.x - getCorrectMousePosition().x) * 2 +
              tempResizeBox.size.x;
          selectedEntity->box.setScale(
              {newSize, newSize / tempResizeBox.size.x * tempResizeBox.size.y});
          break;
        case LockDirection::Right:
          newSize =
              getCorrectMousePosition().x - selectedEntity->box.position.x;
          selectedEntity->box.setScale(
              {newSize, newSize / tempResizeBox.size.x * tempResizeBox.size.y});
          break;
        case LockDirection::Top:
          newSize =
              (tempResizeBox.position.y - getCorrectMousePosition().y) * 2 +
              tempResizeBox.size.y;
          selectedEntity->box.setScale(
              {newSize / tempResizeBox.size.y * tempResizeBox.size.x, newSize});
          break;
        case LockDirection::Bottom:
          newSize =
              getCorrectMousePosition().y - selectedEntity->box.position.y;
          selectedEntity->box.setScale(
              {newSize / tempResizeBox.size.y * tempResizeBox.size.x, newSize});
          break;
        case LockDirection::None:
          break;
        }
      }

      else {
        switch (lock) {
        case LockDirection::Left:
          selectedEntity->box.size = {(tempResizeBox.position.x -
                                       getCorrectMousePosition().x +
                                       tempResizeBox.size.x),
                                      selectedEntity->box.size.y};
          break;
        case LockDirection::Right:
          selectedEntity->box.size = {mousePosition.x -
                                          tempResizeBox.position.x,
                                      selectedEntity->box.size.y};
          break;
        case LockDirection::Top:
          selectedEntity->box.size = {selectedEntity->box.size.x,
                                      (tempResizeBox.position.y -
                                       getCorrectMousePosition().y +
                                       tempResizeBox.size.y)};
          break;
        case LockDirection::Bottom:
          selectedEntity->box.size = {selectedEntity->box.size.x,
                                      mousePosition.y -
                                          tempResizeBox.position.y};
          break;
        case LockDirection::None:
          break;
        }

        if (isSE) {
          selectedEntity->box.position = (tempCorner);
        } else {
          selectedEntity->box.position =
              (tempCorner - selectedEntity->box.size);
        }
      }
    }
  }
}

void MoveTool::update(float deltatime) {
  if (selectedEntity == nullptr)
    return;

  SDL_PumpEvents();
  const Uint8 *keyboardState = SDL_GetKeyboardState(NULL);

  Box insideBox;
  insideBox.position = selectedEntity->box.position + insideBoxMargin;
  insideBox.size = selectedEntity->box.size - insideBoxMargin * 2;

  bool resetCursor = true;

  std::array<bool, 4> selectedSides = {false, false, false, false};

  if (checkIfMouseInside(insideBox) && lock == LockDirection::None) {
    SDL_SetCursor(cursorSizeAll);
    resetCursor = false;

    if (InputManager::checkInput("fire")) {
      tempMouseOffset =
          selectedEntity->box.getCenter() - getCorrectMousePosition();
    }

    if (InputManager::mouseHeld) {
      drawInsideMoveBox(false, insideBox);

      Vector2f prevCenter = selectedEntity->box.getCenter();
      Vector2f setCenter = getCorrectMousePosition() + tempMouseOffset;
      bool horizontalOnly = InputManager::checkInput("dash");
      bool verticalOnly = keyboardState[SDL_SCANCODE_LCTRL];

      if (horizontalOnly) {
        setCenter.y = prevCenter.y;
      } else if (verticalOnly) {
        setCenter.x = prevCenter.x;
      } else {
      }

      selectedEntity->box.setWithCenter(setCenter);
    }

    else {
      drawInsideMoveBox(true, insideBox);
    }
  }

  handleSideDrag(LockDirection::Right, &resetCursor, &selectedSides);
  handleSideDrag(LockDirection::Left, &resetCursor, &selectedSides);
  handleSideDrag(LockDirection::Bottom, &resetCursor, &selectedSides);
  handleSideDrag(LockDirection::Top, &resetCursor, &selectedSides);

  if (!InputManager::mouseHeld) {
    if (lock != LockDirection::None) {
      lock = LockDirection::None;
    }
  }

  if (!InputManager::checkInput("dash")) {
    if (multiLock != LockDirection::None) {
      multiLock = LockDirection::None;
    }
  }

  if (!keyboardState[SDL_SCANCODE_LCTRL]) {
    if (aspectLockLock != LockDirection::None) {
      aspectLockLock = LockDirection::None;
    }
  }

  if (resetCursor) {
    SDL_SetCursor(cursorArrow);
  }

  drawBox(selectedSides);
}
