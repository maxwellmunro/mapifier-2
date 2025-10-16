#ifndef NODE_H
#define NODE_H

// 64 bit on LLP64 and LP64
#if defined(_WIN32) || defined(_WIN64)
#define INDEX_T unsigned long long
#else
#define INDEX_T unsigned long
#endif

#include <boost/serialization/access.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <memory>

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>

#include <string>
#include <vector>

class Map;

class Node {
public:
  static void create(Map *map, float x, float y);

  void clearMem();
  
  void destruct();

  void render(SDL_Renderer *renderer, TTF_Font *mainFont, TTF_Font *smallFont);
  void renderLines(SDL_Renderer *renderer);

  void move(float x, float y);
  void moveRec(float x, float y);

  void setParent(INDEX_T index);
  void removeParent(INDEX_T index);
  void toggleParent(INDEX_T index);

  void setChild(INDEX_T index);
  void removeChild(INDEX_T index);

  void appendText(char text[32]);
  void popChar();
  void setText(std::string text);

  void setBgColor(int r, int g, int b);
  void setTxtColor(int r, int g, int b);

  void queueTextUpdate();

  void selectChildren();

  void setMap(Map* map);

  float getX();
  float getY();

  float getRadius();

  unsigned long index = 69;

  bool centeredText = 1;
  bool singleLine = 0;
  bool rectangle = 0;

  Node();

  SDL_Color textColor;
  SDL_Color bgColor;

  SDL_Surface *textSurface = nullptr;
  SDL_Texture *textTexture = nullptr;

private:
  Node(Map *map, float x, float y);

  void updateTextTexture(SDL_Renderer *renderer, TTF_Font *mainFont,
                         TTF_Font *smallFont);
  SDL_Surface *renderMultilineSurface(const char *text, SDL_Color color,
                                      TTF_Font *mainFont, TTF_Font *smallFont);

  std::vector<INDEX_T> children;
  std::vector<INDEX_T> parents;
  
  float x;
  float y;

  float radius = 10;

  std::string text;

  bool movedThisFrame = 0;

  Map *map;

  bool updateText;

  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive &ar, const unsigned int version);
};

#endif
