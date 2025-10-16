#include "node.h"
#include "map.h"
#include <SDL2/SDL_surface.h>
#include <iostream>

BOOST_CLASS_EXPORT_IMPLEMENT(Node);

void Node::create(Map *map, float x, float y) {
  Node *node = new Node(map, x, y);

  node->bgColor = {37, 232, 250, 255};
  node->textColor = {0, 0, 0, 255};

  map->addNode(node);

  Node *selected = map->getSelected();
  if (selected) {
    node->setParent(selected->index);
  }

  map->select(node->index);
}

void Node::clearMem() {
  if (this->textSurface) {
    SDL_FreeSurface(this->textSurface);
    this->textSurface = nullptr;
  }
  if (this->textTexture) {
    SDL_DestroyTexture(this->textTexture);
    this->textTexture = nullptr;
  }
}

Node::Node() {
  this->x = 0;
  this->y = 0;
  this->map = nullptr;
  this->text = "Null node, please report this";
  this->index = 0;
}

void Node::destruct() {
  for (const auto &index : this->parents) {
    Node *node = this->map->getNode(index);

    if (!node)
      continue;

    node->removeChild(this->index);
  }

  for (const auto &index : this->children) {
    Node *node = this->map->getNode(index);

    if (!node)
      continue;

    node->removeParent(this->index);
  }

  this->map->removeNode(this->index);
}

void Node::render(SDL_Renderer *renderer, TTF_Font *mainFont,
                  TTF_Font *smallFont) {

  this->movedThisFrame = 0;

  if (!this->map)
    return;

  if (!this->map->isOnScreen(this))
    return;

  if (this->radius < 10)
    this->radius = 10;
  if (this->radius > 500)
    this->radius = 500;

  if (this->rectangle && this->textSurface) {
    int w = this->textSurface->w + 10;
    int h = this->textSurface->h + 10;

    if (this->map->isSelected(this->index)) {
      SDL_SetRenderDrawColor(renderer, 0, 255, 0, 100);
      SDL_Rect rect = {static_cast<int>(this->x - w / 2.0 - 10 + this->map->dx),
                       static_cast<int>(this->y - h / 2.0 - 10 + this->map->dy),
                       static_cast<int>(w + 20), static_cast<int>(h + 20)};
      SDL_RenderFillRect(renderer, &rect);
    }

    if (this->map->getSelected() == this) {
      SDL_SetRenderDrawColor(renderer, 0, 255, 0, 100);
      SDL_Rect rect = {static_cast<int>(this->x - w / 2.0 - 20 + this->map->dx),
                       static_cast<int>(this->y - h / 2.0 - 20 + this->map->dy),
                       static_cast<int>(w + 40), static_cast<int>(h + 40)};
      SDL_RenderFillRect(renderer, &rect);
    }

    SDL_SetRenderDrawColor(renderer, this->bgColor.r, this->bgColor.g,
                           this->bgColor.b, 255);
    SDL_Rect rect = {static_cast<int>(this->x - w / 2.0 + this->map->dx),
                     static_cast<int>(this->y - h / 2.0 + this->map->dy),
                     static_cast<int>(w), static_cast<int>(h)};
    SDL_RenderFillRect(renderer, &rect);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &rect);
  } else {
    if (this->map->isSelected(this->index)) {
      filledCircleRGBA(renderer, this->x + this->map->dx,
                       this->y + this->map->dy, this->radius + 10, 0, 255, 0,
                       100);
    }

    if (this->map->getSelected() == this) {
      filledCircleRGBA(renderer, this->x + this->map->dx,
                       this->y + this->map->dy, this->radius + 20, 0, 255, 0,
                       100);
    }
    filledCircleRGBA(renderer, this->x + this->map->dx, this->y + this->map->dy,
                     this->radius, this->bgColor.r, this->bgColor.g,
                     this->bgColor.b, 255);
    aacircleRGBA(renderer, this->x + this->map->dx, this->y + this->map->dy,
                 this->radius, 0, 0, 0, 255);
  }

  if (this->text.length() > 0) {
    if (this->textSurface == nullptr || updateText) {
      this->updateTextTexture(renderer, mainFont, smallFont);
      this->updateText = 0;
    }

    if (this->textSurface) {
      SDL_Rect rect = {static_cast<int>(this->x + this->map->dx -
                                        this->textSurface->w / 2.0),
                       static_cast<int>(this->y + this->map->dy -
                                        this->textSurface->h / 2.0),
                       this->textSurface->w, this->textSurface->h};
      SDL_RenderCopy(renderer, this->textTexture, nullptr, &rect);
    }
  }
}

void rotate(float &x, float &y, float angle) {
  float theta = static_cast<float>(std::atan(y / x) + (x < 0 ? M_PI : 0));
  theta += angle;

  float d = static_cast<float>(sqrt(x * x + y * y));

  x = d * cos(theta);
  y = d * sin(theta);
}

int calcDist(float mx, float my, float nx, float ny, int w, int h) {
  float dx = mx - nx;
  float dy = my - ny;
  float d = static_cast<float>(sqrt(dx * dx + dy * dy));

  for (int i = 0; i < d; i++) {
    float x = i * dx / d;
    float y = i * dy / d;

    if (x > -w / 2.0 && x < w / 2.0 && y > -h / 2.0 && y < h / 2.0)
      continue;

    else
      return i;
  }

  return d;
}

void Node::renderLines(SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  for (int i = 0; i < this->children.size(); i++) {
    int index = this->children[i];

    Node *node = this->map->getNode(index);

    if (!node) {
      this->children.erase(this->children.begin() + i);
      continue;
    }

    SDL_RenderDrawLine(renderer, (this->x + this->map->dx) * this->map->zoom,
                       (this->y + this->map->dy) * this->map->zoom,
                       (node->getX() + this->map->dx) * this->map->zoom,
                       (node->getY() + this->map->dy) * this->map->zoom);

    float dx = node->getX() - this->x;
    float dy = node->getY() - this->y;

    float d = sqrt(dx * dx + dy * dy);

    int dist = node->getRadius();

    if (node->rectangle && node->textSurface)
      dist = calcDist(this->x, this->y, node->getX(), node->getY(),
                      node->textSurface->w + 10, node->textSurface->h + 10);

    float angle = static_cast<float>(std::atan(dy / dx) + (dx < 0 ? M_PI : 0));
    float cx = d - dist;
    float cy = 0;

    float x1 = d - dist - 20;
    float x2 = x1;

    float y1 = -10;
    float y2 = 10;

    rotate(x1, y1, angle);
    rotate(x2, y2, angle);
    rotate(cx, cy, angle);

    x1 += this->x;
    x2 += this->x;

    y1 += this->y;
    y2 += this->y;

    cx += this->x;
    cy += this->y;

    SDL_RenderDrawLine(renderer, (cx + this->map->dx) * this->map->zoom,
                       (cy + this->map->dy) * this->map->zoom,
                       (x1 + this->map->dx) * this->map->zoom,
                       (y1 + this->map->dy) * this->map->zoom);
    SDL_RenderDrawLine(renderer, (cx + this->map->dx) * this->map->zoom,
                       (cy + this->map->dy) * this->map->zoom,
                       (x2 + this->map->dx) * this->map->zoom,
                       (y2 + this->map->dy) * this->map->zoom);
  }
}

void Node::move(float x, float y) {
  this->x = x;
  this->y = y;
}

void Node::moveRec(float x, float y) {
  if (this->movedThisFrame)
    return;

  this->movedThisFrame = 1;

  for (const auto &index : this->children) {
    Node *node = map->getNode(index);

    node->moveRec(node->getX() - this->x + x, node->getY() - this->y + y);
  }

  this->x = x;
  this->y = y;
}

void Node::setParent(INDEX_T index) {
  for (const auto &node : this->parents)
    if (node == index)
      return;

  this->parents.push_back(index);
  this->map->getNode(index)->setChild(this->index);
}

void Node::removeParent(INDEX_T index) {
  for (int i = 0; i < this->parents.size(); i++) {
    if (this->parents[i] == index) {
      this->parents.erase(this->parents.begin() + i);
      this->map->getNode(index)->removeChild(this->index);
      return;
    }
  }
}

void Node::toggleParent(INDEX_T index) {
  for (const auto &node : this->parents)
    if (node == index) {
      this->removeParent(index);
      return;
    }

  this->setParent(index);
}

void Node::setChild(INDEX_T index) {
  for (const auto &node : this->children)
    if (node == index)
      return;

  this->children.push_back(index);
  this->map->getNode(index)->setParent(this->index);
}

void Node::removeChild(INDEX_T index) {
  for (int i = 0; i < this->children.size(); i++)
    if (this->children[i] == index) {
      this->children.erase(this->children.begin() + i);
      this->map->getNode(index)->removeParent(this->index);
      return;
    }
}

void Node::appendText(char text[32]) {
  this->text += text;
  updateText = 1;
}

void Node::popChar() {
  if (this->text.empty())
    return;

  this->text.pop_back();
  updateText = 1;
}

void Node::setText(std::string text) {
  this->text = text;

  if (this->text.length() == 0)
    this->radius = 10;

  updateText = 1;
}

void Node::setBgColor(int r, int g, int b) {
  if (r >= 0 && r <= 255)
    this->bgColor.r = r;
  if (g >= 0 && g <= 255)
    this->bgColor.g = g;
  if (b >= 0 && b <= 255)
    this->bgColor.b = b;
}

void Node::setTxtColor(int r, int g, int b) {
  if (r >= 0 && r <= 255)
    this->textColor.r = r;
  if (g >= 0 && g <= 255)
    this->textColor.g = g;
  if (b >= 0 && b <= 255)
    this->textColor.b = b;
}

void Node::queueTextUpdate() { this->updateText = 1; }

void Node::selectChildren() {
  for (const auto &node : this->children) {
    if (this->map->isSelected(node))
      continue;

    this->map->select(node);
  }
}

void Node::setMap(Map *map) { this->map = map; }

float Node::getX() { return this->x; }

float Node::getY() { return this->y; }

float Node::getRadius() { return this->radius; }

Node::Node(Map *map, float x, float y) : map(map), x(x), y(y) {
  this->text = "Insert Text";
}

void freeSurf(SDL_Surface **surf) {
  if (surf && *surf) {
    SDL_FreeSurface(*surf);
    *surf = nullptr;
  }
}

void Node::updateTextTexture(SDL_Renderer *renderer, TTF_Font *mainFont,
                             TTF_Font *smallFont) {
  if (this->textSurface)
    freeSurf(&this->textSurface);

  this->textSurface =
      TTF_RenderText_Solid(mainFont, this->text.c_str(), this->textColor);

  if (!this->textSurface) {
    std::cout << "TTF_RenderText_Solid failed: " << TTF_GetError() << '\n';
    return;
  }

  int area = this->textSurface->w * this->textSurface->h;
  int len = static_cast<int>(sqrt(area));

  std::vector<std::string> words = {};

  std::string curWord = "";
  int width = 0;
  for (int i = 0; i < text.length(); i++) {
    char c = text[i];

    if (c == ' ' && width > len) {
      width = 0;
      words.push_back(curWord);
      curWord = "";
      continue;
    }

    curWord += c;
    int tempWidth, tempHeight;
    char chr[2] = {c, '\0'};
    TTF_SizeText(mainFont, chr, &tempWidth, &tempHeight);
    width += tempWidth;

    if (i + 1 == text.length())
      words.push_back(curWord);
  }

  std::string tempText = "";

  for (int i = 0; i < words.size(); i++) {
    std::string word = words[i];
    tempText += word;
    if (i + 1 != words.size())
      tempText += '\n';
  }

  if (this->textSurface) {
    SDL_FreeSurface(this->textSurface);
    this->textSurface = nullptr;
  }

  this->textSurface = renderMultilineSurface(
      this->singleLine ? this->text.c_str() : tempText.c_str(), this->textColor,
      mainFont, smallFont);

  this->radius = static_cast<float>(
                     std::sqrt(this->textSurface->w * this->textSurface->w +
                               this->textSurface->h * this->textSurface->h)) /
                     2 +
                 10;

  if (!this->textSurface) {
    std::cout << "TTF_RenderText_Solid failed: " << TTF_GetError() << '\n';
    return;
  }

  if (this->textTexture)
    SDL_DestroyTexture(this->textTexture);
  this->textTexture = SDL_CreateTextureFromSurface(renderer, this->textSurface);

  if (!this->textTexture) {
    std::cout << "SDL_CreateTextureFromSurface failed: " << SDL_GetError()
              << '\n';
    return;
  }
}

SDL_Surface *Node::renderMultilineSurface(const char *text, SDL_Color color,
                                          TTF_Font *mainFont,
                                          TTF_Font *smallFont) {
  char *textCopy = strdup(text);
  char *line = strtok(textCopy, "\n");

  int totalHeight = 0;
  int maxWidth = 0;
  char *lines[128];
  int numLines = 0;

  std::vector<int> widths;

  int normalCharHeight;

  TTF_SizeUTF8(mainFont, "A", nullptr, &normalCharHeight);

  while (line && numLines < 128) {
    lines[numLines++] = strdup(line);

    int w = 0;

    for (char *p = line; *p != '\0'; ++p) {

      if (*p == '_' || *p == '^')
        continue;

      TTF_Font *font = (p > line && (*(p - 1) == '_' || *(p - 1) == '^'))
                           ? smallFont
                           : mainFont;

      int tw, th;

      char lin[2] = {*p, '\0'};
      TTF_SizeUTF8(font, lin, &tw, nullptr);

      w += tw;
    }

    if (w > maxWidth)
      maxWidth = w;
    totalHeight += normalCharHeight;

    widths.push_back(w);

    line = strtok(NULL, "\n");
  }

  SDL_Surface *final =
      SDL_CreateRGBSurface(0, maxWidth, totalHeight, 32, 0x00FF0000, 0x0000FF00,
                           0x000000FF, 0xFF000000);

  int y = 0;

  if (!final) {
    fprintf(stderr, "Failed to create final surface: %s\n", SDL_GetError());
    goto cleanup;
  }

  for (int i = 0; i < numLines; ++i) {

    int x = this->centeredText ? (final->w - widths[i]) / 2 : 0;

    int minH = 0;

    for (char *p = lines[i]; *p != '\0'; ++p) {

      if (*p == '_' || *p == '^')
        continue;

      TTF_Font *font = (p > lines[i] && (*(p - 1) == '_' || *(p - 1) == '^'))
                           ? smallFont
                           : mainFont;

      char lin[2] = {*p, '\0'};
      SDL_Surface *charSurf = TTF_RenderUTF8_Blended(font, lin, color);

      if (!charSurf) {
        fprintf(stderr, "Render failed: %s\n", TTF_GetError());
        continue;
      }
      SDL_Rect dest = {x, y, charSurf->w, charSurf->h};

      if (p > lines[i] && *(p - 1) == '_')
        dest.y += normalCharHeight / 3;

      x += charSurf->w;
      if (charSurf->h > minH)
        minH = charSurf->h;

      SDL_BlitSurface(charSurf, NULL, final, &dest);

      if (charSurf) {
        SDL_FreeSurface(charSurf);
        charSurf = nullptr;
      }
    }
    y += minH;
  }

cleanup:
  for (int i = 0; i < numLines; ++i)
    free(lines[i]);
  free(textCopy);

  return final;
}

template <class Archive>
void Node::serialize(Archive &ar, const unsigned int version) {
  ar &boost::serialization::make_nvp("index", index);
  ar &boost::serialization::make_nvp("children", children);
  ar &boost::serialization::make_nvp("parents", parents);

  ar &boost::serialization::make_nvp("x", x);
  ar &boost::serialization::make_nvp("y", y);

  ar &boost::serialization::make_nvp("radius", radius);

  ar &boost::serialization::make_nvp("text", text);

  ar &boost::serialization::make_nvp("textColor_r", textColor.r);
  ar &boost::serialization::make_nvp("textColor_g", textColor.g);
  ar &boost::serialization::make_nvp("textColor_b", textColor.b);
  ar &boost::serialization::make_nvp("textColor_a", textColor.a);

  ar &boost::serialization::make_nvp("bgColor_r", bgColor.r);
  ar &boost::serialization::make_nvp("bgColor_g", bgColor.g);
  ar &boost::serialization::make_nvp("bgColor_b", bgColor.b);
  ar &boost::serialization::make_nvp("bgColor_a", bgColor.a);

  ar &boost::serialization::make_nvp("centeredText", centeredText);
  ar &boost::serialization::make_nvp("singleLine", singleLine);
  ar &boost::serialization::make_nvp("rectangle", rectangle);
}
