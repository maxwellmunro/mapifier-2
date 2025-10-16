#ifndef MAP_H
#define MAP_H

#include <boost/serialization/access.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/unordered_map.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include "node.h"

#include <set>
#include <unordered_map>

class Map {
public:
  Map();

  void addNode(Node* node);
  void removeNode(INDEX_T);
  Node *getNode(INDEX_T index);

  void render(SDL_Renderer *renderer, TTF_Font* mainFont, TTF_Font* smallFont);

  void zoomIn(float fac, float x, float y);

  void saveMap(const std::string &filename);
  void loadMap(const std::string &filename);

  bool isSelected(INDEX_T index);
  std::vector<INDEX_T> getAllSelected();
  Node *getSelected();

  Node *getPressed(int mx, int my);
  
  void toggleSelect(INDEX_T index);
  void select(INDEX_T index);
  void deselect(INDEX_T index);
  void clearSelected();

  bool isOnScreen(Node* node);

  float dx;
  float dy;

  float zoom;

  int WIDTH, HEIGHT;

  std::string name = "untitled";

  bool saved = 0;

private:
  INDEX_T curIndex = 0;

  std::unordered_map<unsigned long, Node> nodes;
  std::set<INDEX_T> selectedNodes;
  INDEX_T selectedNode;

  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive &ar, const unsigned int version);
};

#endif
