#pragma once
#include "Charlie2D.h"
#include "SDL_mouse.h"
#include "Vector2f.h"
#include "functionUtils.h"
#include <array>

enum class LockDirection { Right, Left, Top, Bottom, None, Move };

class MoveTool : public Component {
public:
  MoveTool();

  ~MoveTool();
  void start() override;

  Vector2f getCorrectMousePosition();
  void drawBoxLine(Box box, std::array<Uint8, 3> color = {0, 255, 0},
                   Uint8 alpha = 255);
  void drawBox(std::array<bool, 4> selectedSides);
  void drawInsideMoveBox(bool hover, Box box);
  bool checkIfMouseInside(Box box);
  void handleSideDrag(LockDirection lockDirection, bool *resetCursor,
                      std::array<bool, 4> *selectedSides);
  void update(float deltatime) override;

  const int stroke = 5;
  const int insideBoxMargin = 15;

  Vector2f tempMouseOffset;
  Box tempResizeBox;

  std::array<Uint8, 3> color = {0, 255, 0};

  Box leftBox;
  Box rightBox;
  Box topBox;
  Box bottomBox;

  LockDirection lock;
  LockDirection multiLock;
  LockDirection aspectLockLock;

  SDL_Cursor *cursorArrow;
  SDL_Cursor *cursorIBeam;
  SDL_Cursor *cursorWait;
  SDL_Cursor *cursorCrosshair;
  SDL_Cursor *cursorWaitArrow;
  SDL_Cursor *cursorSizeNWSE;
  SDL_Cursor *cursorSizeNESW;
  SDL_Cursor *cursorSizeWE;
  SDL_Cursor *cursorSizeNS;
  SDL_Cursor *cursorSizeAll;
  SDL_Cursor *cursorNo;
  SDL_Cursor *cursorHand;
};
