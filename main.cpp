#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_video.h>

#include <cmath>
#include <iostream>
#include <random>

#include "map.h"
#include "node.h"

SDL_Window *window;
SDL_Renderer *renderer;

TTF_Font *mainFont;
TTF_Font *smallFont;

SDL_Surface *filenameSurface = nullptr;

bool running = 1;

bool shiftPressed = 0;
bool ctrlPressed = 0;
bool altPressed = 0;

int mouseX = 0;
int mouseY = 0;

int worldMouseX = 0;
int worldMouseY = 0;

int mouseDownXPos = 0;
int mouseDownYPos = 0;

int nodeDownXPos = 0;
int nodeDownYPos = 0;

int mouseDownDisplacementX = 0;
int mouseDownDisplacementY = 0;

bool rightDown = 0;
bool leftDown = 0;

bool settingParent = 0;
bool typingFilename = 0;

bool moving = 0;

Map map = Map();

// {bg, text}
SDL_Color clipboardColors[2] = {{37, 232, 250, 255}, {0, 0, 0, 255}};

std::string home = std::getenv("HOME");

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<double> dis(0.0, 1.0);

std::uniform_int_distribution<int> col(0, 255);

void handleMousePress(int button) {

  leftDown = button == 1 || leftDown;
  rightDown = button == 3 || rightDown;

  mouseDownXPos = worldMouseX;
  mouseDownYPos = worldMouseY;

  switch (button) {
  case 1: { // left

    if (filenameSurface->w + 10 > mouseX && filenameSurface->h > mouseY) {
      typingFilename = 1;
      break;
    }

    if (map.getSelected() && mouseX > map.WIDTH - 280 && mouseY < 335)
      break;

    Node *node = map.getPressed(worldMouseX, worldMouseY);

    if (!node) {
      map.clearSelected();
      break;
    }

    if (settingParent) {
      std::vector<INDEX_T> nodes = map.getAllSelected();

      for (const auto &index : nodes) {
        if (index == node->index)
          continue;

        map.getNode(index)->toggleParent(node->index);
      }

      settingParent = 0;

      break;
    }

    nodeDownXPos = node->getX();
    nodeDownYPos = node->getY();

    if (shiftPressed) {
      map.toggleSelect(node->index);
    } else {
      map.clearSelected();
      map.select(node->index);
      moving = 1;
    }

    break;
  }

  case 2: { // middle

    break;
  }

  case 3: { // right
    mouseDownDisplacementX = map.dx;
    mouseDownDisplacementY = map.dy;
    rightDown = 1;

    break;
  }

  default:
    break;
  }
}

void handleMouseRelease(int button) {
  leftDown = button != 1 && leftDown;
  rightDown = button != 3 && rightDown;
  moving = 0;
}

void handleKeyPress(SDL_Keycode key) {
  shiftPressed = key == SDLK_LSHIFT || shiftPressed;
  ctrlPressed = key == SDLK_LCTRL || ctrlPressed;
  altPressed = key == SDLK_LALT || altPressed;

  if (key == SDLK_RETURN) {
    typingFilename = 0;
  }

  if (key == SDLK_ESCAPE) {
    settingParent = 0;
    typingFilename = 0;
  }

  if (ctrlPressed && key == SDLK_n) {
    Node::create(&map, worldMouseX, worldMouseY);
  }

  if (key == SDLK_BACKSPACE) {
    if (typingFilename) {
      if (!map.name.empty())
        map.name.pop_back();
    } else {
      Node *node = map.getSelected();

      if (node) {
        if (ctrlPressed)
          node->setText("");
        else
          node->popChar();
      }
    }
  }

  if (ctrlPressed && key == SDLK_r) {
    std::vector<INDEX_T> nodes = map.getAllSelected();

    for (const auto &index : nodes)
      map.getNode(index)->setBgColor(col(gen), col(gen), col(gen));
  }

  if (ctrlPressed && key == SDLK_s) {
    settingParent = 1;
  }

  if (ctrlPressed && key == SDLK_EQUALS) {
    std::vector<INDEX_T> nodes = map.getAllSelected();

    for (const auto &index : nodes) {
      map.getNode(index)->selectChildren();
    }
  }

  if (ctrlPressed && key == SDLK_1) {
    std::vector<INDEX_T> nodes = map.getAllSelected();

    for (const auto &index : nodes) {
      map.getNode(index)->singleLine = !map.getNode(index)->singleLine;
      map.getNode(index)->queueTextUpdate();
    }
  }

  if (ctrlPressed && key == SDLK_2) {
    std::vector<INDEX_T> nodes = map.getAllSelected();

    for (const auto &index : nodes)
      map.getNode(index)->rectangle = !map.getNode(index)->rectangle;
  }

  if (ctrlPressed && key == SDLK_c) {
    std::vector<INDEX_T> nodes = map.getAllSelected();

    for (const auto &index : nodes) {
      map.getNode(index)->centeredText = !map.getNode(index)->centeredText;
      map.getNode(index)->queueTextUpdate();
    }
  }

  if (key == SDLK_DELETE) {
    std::vector<INDEX_T> nodes = map.getAllSelected();

    for (const auto &index : nodes) {
      map.getNode(index)->destruct();
    }
  }

  if (ctrlPressed && key == SDLK_w) {
    map.saveMap(home + "/.mapifier/" + map.name + ".map");
  }

  if (ctrlPressed && key == SDLK_o) {
    map.loadMap(home + "/.mapifier/" + map.name + ".map");
  }

  if (ctrlPressed && key == SDLK_a && map.getSelected()) {
    Node *node = map.getSelected();

    clipboardColors[0] = node->bgColor;
    clipboardColors[1] = node->textColor;
  }

  if (ctrlPressed && key == SDLK_z) {
    std::vector<INDEX_T> nodes = map.getAllSelected();

    for (const auto &index : nodes) {
      Node *node = map.getNode(index);
      if (!node)
        continue;

      node->bgColor = clipboardColors[0];
      node->textColor = clipboardColors[1];
      node->queueTextUpdate();
    }
  }

  if (ctrlPressed && shiftPressed && altPressed && key == SDLK_q) {
    running = 0;
  }

  if (key == SDLK_UP || key == SDLK_DOWN || key == SDLK_LEFT || key == SDLK_RIGHT) {
    std::vector<INDEX_T> nodes = map.getAllSelected();

    for (const auto &index : nodes) {
      Node* node = map.getNode(index);

      if (!node)
        continue;

      if (key == SDLK_UP)
        node->move(node->getX(), node->getY() - (shiftPressed ? 10 : 1) * (ctrlPressed ? 10 : 1));
      if (key == SDLK_DOWN)
        node->move(node->getX(), node->getY() + (shiftPressed ? 10 : 1) * (ctrlPressed ? 10 : 1));
      if (key == SDLK_LEFT)
        node->move(node->getX() - (shiftPressed ? 10 : 1) * (ctrlPressed ? 10 : 1), node->getY());
      if (key == SDLK_RIGHT)
        node->move(node->getX() + (shiftPressed ? 10 : 1) * (ctrlPressed ? 10 : 1), node->getY());
    }
  }
}

void handleKeyRelease(SDL_Keycode key) {
  shiftPressed = key != SDLK_LSHIFT && shiftPressed;
  ctrlPressed = key != SDLK_LCTRL && ctrlPressed;
  altPressed = key != SDLK_LALT && altPressed;
}

void rotate(float *x, float *y, float angle) {
  float dx = *x - worldMouseX;
  float dy = *y - worldMouseY;
  float d = static_cast<float>(sqrt(dx * dx + dy * dy));
  float curAngle = static_cast<float>(std::atan(dy / dx)) + (dx < 0 ? M_PI : 0);

  curAngle += angle;

  *x = worldMouseX + d * cos(curAngle);
  *y = worldMouseY + d * sin(curAngle);
}

void handleMouseScroll(SDL_MouseWheelEvent event) {

  if (altPressed) {
    std::vector<INDEX_T> nodes = map.getAllSelected();

    float angle = (shiftPressed ? 10 : 1) * (event.preciseY < 0 ? 1 : -1) * 1 * M_PI / 180;

    for (const auto &index : nodes) {
      Node* node = map.getNode(index);

      if (!node)
        continue;

      float x = node->getX();
      float y = node->getY();

      rotate(&x, &y, angle);

      node->move(x, y);
    }

    return;
  }

  if (event.preciseY > 0 && map.zoom < 1)
    map.zoomIn(.1, mouseX, mouseY);
  if (event.preciseY < 0 && map.zoom > 0.1)
    map.zoomIn(-.1, mouseX, mouseY);
}

void handleTextInput(char text[32]) {
  if (typingFilename) {
    map.name += text;
    return;
  }

  Node *node = map.getSelected();

  if (node) {
    node->appendText(text);
  }
}

int main() {

  std::cout << "--- MOUSE ---\n";
  std::cout << "Left: select / move\n";
  std::cout << "Shift+Left: toggle select / mulltiselect\n";
  std::cout << "Alt+Left: move children\n";
  std::cout << "Right: pan\n";
  std::cout << "Scroll: zoom\n";

  std::cout << "\n--- KEYS ---\n";
  std::cout << "Return: stop typing filename\n";
  std::cout << "Escape: stop typing filename and stop setting parent(s)\n";
  std::cout << "Ctrl+N: \033[4mN\033[0mew node\n";
  std::cout << "Ctrl+Backspace: clear nodes text\n";
  std::cout << "Ctrl+R: set node(s) \033[4mR\033[0mandom colours\n";
  std::cout << "Ctrl+S: \033[4mS\033[0met node(s) parents\n";
  std::cout << "Ctrl+=: select children\n";
  std::cout << "Ctrl+1: toggle text wrapping\n";
  std::cout << "Ctrl+2: toggle node shape\n";
  std::cout << "Ctrl+C: toggle \033[4mC\033[0mentered text\n";
  std::cout << "Delete: \033[4mDELETE\033[0m node\n";
  std::cout << "Ctrl+W: \033[4mW\033[0mrite (save) current map\n";
  std::cout << "Ctrl+O: \033[4mO\033[0mpen current map\n";
  std::cout << "Ctrl+A: copy colour\n";
  std::cout << "Ctrl+Z: paste colour\n";
  std::cout << "Ctrl+Shift+Alt+Q: force \033[4mQ\033[0muit\n\n";
  std::cout << "Arrow Keys: Move node\n";
  std::cout << "Shift/Ctrl + Arrow: Move node faster\n";
  
  SDL_Init(SDL_INIT_EVERYTHING);
  TTF_Init();

  window = SDL_CreateWindow("Mapifier 2.0", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, 500, 500, SDL_WINDOW_SHOWN);
  renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  SDL_StartTextInput();

  mainFont = TTF_OpenFont((home + "/.mapifier/res/mainFont.ttf").c_str(), 18);
  smallFont = TTF_OpenFont((home + "/.mapifier/res/mainFont.ttf").c_str(), 12);

  SDL_SetWindowResizable(window, SDL_TRUE);

  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT: {
        if (map.saved)
          running = 0;
        break;
      }

      case SDL_KEYDOWN: {
        handleKeyPress(event.key.keysym.sym);
        break;
      }

      case SDL_KEYUP: {
        handleKeyRelease(event.key.keysym.sym);
        break;
      }

      case SDL_MOUSEBUTTONDOWN: {
        handleMousePress(event.button.button);
        break;
      }

      case SDL_MOUSEBUTTONUP: {
        handleMouseRelease(event.button.button);
        break;
      }

      case SDL_MOUSEWHEEL: {
        handleMouseScroll(event.wheel);
        break;
      }

      case SDL_TEXTINPUT: {
        handleTextInput(event.text.text);
        break;
      }
      }
    }

    SDL_GetMouseState(&mouseX, &mouseY);
    SDL_GetWindowSize(window, &map.WIDTH, &map.HEIGHT);

    worldMouseX = static_cast<int>(mouseX / map.zoom - map.dx);
    worldMouseY = static_cast<int>(mouseY / map.zoom - map.dy);

    if (leftDown && !moving && mouseX > map.WIDTH - 280 && mouseY >= 20 && mouseY <= 275 &&
        map.getSelected()) {
      Node *selected = map.getSelected();

      if (mouseX > map.WIDTH - 260 && mouseX < map.WIDTH - 240)
        selected->textColor.r = 275 - mouseY;
      if (mouseX > map.WIDTH - 220 && mouseX < map.WIDTH - 200)
        selected->textColor.g = 275 - mouseY;
      if (mouseX > map.WIDTH - 180 && mouseX < map.WIDTH - 160)
        selected->textColor.b = 275 - mouseY;

      if (mouseX > map.WIDTH - 120 && mouseX < map.WIDTH - 100)
        selected->bgColor.r = 275 - mouseY;
      if (mouseX > map.WIDTH - 80 && mouseX < map.WIDTH - 60)
        selected->bgColor.g = 275 - mouseY;
      if (mouseX > map.WIDTH - 40 && mouseX < map.WIDTH - 20)
        selected->bgColor.b = 275 - mouseY;

      selected->queueTextUpdate();
    }

    if (leftDown && moving) {
      Node *node = map.getSelected();

      if (!node)
        continue;

      if (altPressed)
        node->moveRec(nodeDownXPos + worldMouseX - mouseDownXPos,
                      nodeDownYPos + worldMouseY - mouseDownYPos);
      else
        node->move(nodeDownXPos + worldMouseX - mouseDownXPos,
                   nodeDownYPos + worldMouseY - mouseDownYPos);
    }

    if (rightDown) {
      map.dx = mouseDownDisplacementX +
               (mouseX / map.zoom - mouseDownDisplacementX) - mouseDownXPos;
      map.dy = mouseDownDisplacementY +
               (mouseY / map.zoom - mouseDownDisplacementY) - mouseDownYPos;
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    map.render(renderer, mainFont, smallFont);

    SDL_RenderSetScale(renderer, 1, 1);

    if (filenameSurface) {
      SDL_FreeSurface(filenameSurface);
      filenameSurface = nullptr;
    }

    filenameSurface = TTF_RenderText_Blended(
        mainFont, (" File: " + map.name + ".mind").c_str(),
        SDL_Color{0, 0, 0, 255});
    SDL_Texture *filenameTexture =
        SDL_CreateTextureFromSurface(renderer, filenameSurface);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    if (typingFilename)
      SDL_SetRenderDrawColor(renderer, 120, 255, 120, 255);
    SDL_Rect filenameBgRect = {0, 0, filenameSurface->w + 10,
                               filenameSurface->h};
    SDL_RenderFillRect(renderer, &filenameBgRect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &filenameBgRect);
    filenameBgRect.w -= 10;
    SDL_RenderCopy(renderer, filenameTexture, nullptr, &filenameBgRect);
    SDL_DestroyTexture(filenameTexture);

    Node *selectedNode = map.getSelected();

    if (selectedNode) {
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
      SDL_Rect colorChangeBgRect = {map.WIDTH - 280, 0, 280, 335};
      SDL_RenderFillRect(renderer, &colorChangeBgRect);
      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
      SDL_Rect rect = {map.WIDTH - 280, 0, 141, 335};
      SDL_RenderDrawRect(renderer, &rect);
      rect = {map.WIDTH - 140, 0, 140, 335};
      SDL_RenderDrawRect(renderer, &rect);

      for (int i = 0; i < 255; i++) {

        SDL_Rect rect = {map.WIDTH - 260, 274 - i, 20, 1};

        int bgr = selectedNode->bgColor.r;
        int bgg = selectedNode->bgColor.g;
        int bgb = selectedNode->bgColor.b;

        int tr = selectedNode->textColor.r;
        int tg = selectedNode->textColor.g;
        int tb = selectedNode->textColor.b;

        SDL_SetRenderDrawColor(renderer, i, tg, tb, 255);
        SDL_RenderFillRect(renderer, &rect);

        rect.x += 40;
        SDL_SetRenderDrawColor(renderer, tr, i, tb, 255);
        SDL_RenderFillRect(renderer, &rect);

        rect.x += 40;
        SDL_SetRenderDrawColor(renderer, tr, tg, i, 255);
        SDL_RenderFillRect(renderer, &rect);

        SDL_Rect txtColorRect = {map.WIDTH - 260, 295, 100, 20};
        SDL_SetRenderDrawColor(renderer, tr, tg, tb, 255);
        SDL_RenderFillRect(renderer, &txtColorRect);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &txtColorRect);

        txtColorRect = {map.WIDTH - 260, 20, 20, 255};
        SDL_RenderDrawRect(renderer, &txtColorRect);

        txtColorRect.x += 40;
        SDL_RenderDrawRect(renderer, &txtColorRect);

        txtColorRect.x += 40;
        SDL_RenderDrawRect(renderer, &txtColorRect);

        rect.x += 60;
        SDL_SetRenderDrawColor(renderer, i, bgg, bgb, 255);
        SDL_RenderFillRect(renderer, &rect);

        rect.x += 40;
        SDL_SetRenderDrawColor(renderer, bgr, i, bgb, 255);
        SDL_RenderFillRect(renderer, &rect);

        rect.x += 40;
        SDL_SetRenderDrawColor(renderer, bgr, bgg, i, 255);
        SDL_RenderFillRect(renderer, &rect);

        SDL_Rect bgColorRect = {map.WIDTH - 120, 295, 100, 20};
        SDL_SetRenderDrawColor(renderer, bgr, bgg, bgb, 255);
        SDL_RenderFillRect(renderer, &bgColorRect);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &bgColorRect);

        bgColorRect = {map.WIDTH - 120, 20, 20, 255};
        SDL_RenderDrawRect(renderer, &bgColorRect);

        bgColorRect.x += 40;
        SDL_RenderDrawRect(renderer, &bgColorRect);

        bgColorRect.x += 40;
        SDL_RenderDrawRect(renderer, &bgColorRect);
      }

      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
      rect = {map.WIDTH - 265, 274 - selectedNode->textColor.r, 30, 3};
      SDL_RenderFillRect(renderer, &rect);

      rect.x += 40;
      rect.y = 274 - selectedNode->textColor.g;
      SDL_RenderFillRect(renderer, &rect);

      rect.x += 40;
      rect.y = 274 - selectedNode->textColor.b;
      SDL_RenderFillRect(renderer, &rect);

      rect.x += 60;
      rect.y = 274 - selectedNode->bgColor.r;
      SDL_RenderFillRect(renderer, &rect);

      rect.x += 40;
      rect.y = 274 - selectedNode->bgColor.g;
      SDL_RenderFillRect(renderer, &rect);

      rect.x += 40;
      rect.y = 274 - selectedNode->bgColor.b;
      SDL_RenderFillRect(renderer, &rect);
    }

    if (!map.saved) {
      SDL_Surface *unsavedSurf = TTF_RenderText_Blended(
          mainFont, "!! UNSAVED !!", SDL_Color{255, 0, 0, 255});
      SDL_Texture *unsavedText =
          SDL_CreateTextureFromSurface(renderer, unsavedSurf);
      SDL_Rect unsavedRect = [unsavedSurf]() {
        SDL_Rect r = {10, 20, unsavedSurf->w, unsavedSurf->h};

        if (unsavedSurf->w + 10 < filenameSurface->w)
          r.x = (filenameSurface->w - unsavedSurf->w) / 2;

        return r;
      }();

      SDL_RenderCopy(renderer, unsavedText, nullptr, &unsavedRect);

      SDL_DestroyTexture(unsavedText);
      SDL_FreeSurface(unsavedSurf);
    }

    SDL_RenderPresent(renderer);
  }

  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);

  SDL_Quit();
}
