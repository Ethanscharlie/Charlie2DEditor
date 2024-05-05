#include "Camera.h"
#include "Component.h"
#include "GameManager.h"
#include "SDL_rect.h"
#include "SDL_render.h"
#include "SDL_stdinc.h"
#include "Serializer.h"
#include <array>

class SimplePanel : public Component {
public:
  SimplePanel() {
    typeIsRendering = true;

    propertyRegister = {
        GET_PROP(rendererInWorld),
    };
  }

  void start() override {}

  void update(float deltaTime) override {
    SDL_Rect renderRect = calculatePosition(rendererInWorld);
    SDL_SetRenderDrawColor(GameManager::renderer, color[0], color[1], color[2],
                           alpha);
    SDL_RenderFillRect(GameManager::renderer, &renderRect);
  }

  void setColor(std::array<Uint8, 3> color, float alpha = 1) {
    this->color = color;
    this->alpha = alpha;
  }

  bool rendererInWorld = false;
  Uint8 alpha;
  std::array<Uint8, 3> color;

private:
  SDL_Rect calculatePosition(bool rendererInWorld) {
    SDL_Rect spriteRect;
    if (!rendererInWorld) {
      Vector2f renderPos =
          entity->box.position + GameManager::gameWindowSize / 2;
      spriteRect.x = renderPos.x; //+ GameManager::camera.getCenter().x;
      spriteRect.y = renderPos.y; //+ GameManager::camera.getCenter().y;
      spriteRect.w = entity->box.size.x;
      spriteRect.h = entity->box.size.y;

    } else {
      Vector2f renderPos = entity->box.position - Camera::getPosition();
      renderPos = renderPos * Camera::getScale();
      renderPos += GameManager::gameWindowSize / 2;
      spriteRect.x = renderPos.x;
      spriteRect.y = renderPos.y;

      spriteRect.w = entity->box.size.x * Camera::getScale();
      spriteRect.h = entity->box.size.y * Camera::getScale();
    }

    return spriteRect;
  }
};
REGISTER_COMPONENT_TYPE(SimplePanel);
