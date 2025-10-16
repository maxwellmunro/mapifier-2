#include "map.h"
#include <cmath>
#include <exception>
#include <fstream>
#include <iostream>

Map::Map() {
  this->dx = 0.0F;
  this->dy = 0.0F;

  this->zoom = 1.0F;

  this->curIndex = 0ULL;
}

void Map::addNode(Node *node) {
  node->index = this->curIndex;
  this->nodes[this->curIndex++] = *node;
  this->saved = 0;
}

void Map::removeNode(INDEX_T node) {
  this->nodes.erase(node);
  this->deselect(node);
  this->saved = 0;
}

Node *Map::getNode(INDEX_T index) { return &this->nodes[index]; }

void Map::render(SDL_Renderer *renderer, TTF_Font *mainFont,
                 TTF_Font *smallFont) {
  SDL_RenderSetScale(renderer, 1, 1);

  for (auto &pair : this->nodes) {
    pair.second.renderLines(renderer);
  }

  SDL_RenderSetScale(renderer, this->zoom, this->zoom);

  for (auto &pair : this->nodes) {
    pair.second.render(renderer, mainFont, smallFont);
  }
}

void Map::zoomIn(float fac, float x, float y) {
  float prex = x / this->zoom;
  float prey = y / this->zoom;

  this->zoom += fac;

  float pstx = x / this->zoom;
  float psty = y / this->zoom;

  this->dx -= prex - pstx;
  this->dy -= prey - psty;

  this->saved = 0;
}

void Map::saveMap(const std::string &filename) {
  std::ofstream ofs(filename, std::ios::binary);
  boost::archive::binary_oarchive oa(ofs);
  oa << *this;
  ofs.close();

  this->saved = 1;
}

void Map::loadMap(const std::string &filename) {
  try {
    for (const auto& pair : this->nodes)
      this->getNode(pair.first)->clearMem();

    this->nodes.clear();
    this->selectedNodes.clear();

    std::ifstream ifs(filename, std::ios::binary);

    if (!ifs) {
      std::cout << "Error opening file!\n";
      return;
    }

    boost::archive::binary_iarchive ia(ifs);
    ia >> *this;
    ifs.close();

    for (const auto &pair : this->nodes) {
      this->getNode(pair.first)->setMap(this);
    }

  } catch (const std::exception &e) {
    std::cout << "Error loading map: " << e.what() << '\n';
  }

  this->saved = 1;
}

bool Map::isSelected(INDEX_T index) { return this->selectedNodes.count(index); }

std::vector<INDEX_T> Map::getAllSelected() {
  std::vector<INDEX_T> ret;

  for (const auto &index : this->selectedNodes)
    ret.push_back(index);

  return ret;
}

Node *Map::getSelected() {
  if (this->selectedNodes.empty() ||
      !this->selectedNodes.count(this->selectedNode))
    return nullptr;

  return this->getNode(this->selectedNode);
}

Node *Map::getPressed(int mx, int my) {
  for (auto &pair : this->nodes) {
    Node node = pair.second;

    this->getNode(pair.first)->queueTextUpdate();

    if (node.rectangle) {
      int w = node.textSurface->w + 10;
      int h = node.textSurface->h + 10;

      if (mx > node.getX() - w / 2.0 && mx < node.getX() + w / 2.0 &&
          my > node.getY() - h / 2.0 && my < node.getY() + h / 2.0) {
        return &pair.second;
      }

      continue;
    }

    float x = pair.second.getX();
    float y = pair.second.getY();

    float dx = mx - x;
    float dy = my - y;

    float d = static_cast<float>(std::sqrt(dx * dx + dy * dy));

    if (d < pair.second.getRadius()) {
      return &pair.second;
    }
  }

  return nullptr;
}

void Map::toggleSelect(INDEX_T index) {
  if (this->isSelected(index)) {
    this->deselect(index);
    return;
  }

  this->select(index);

  this->saved = 0;
}

void Map::select(INDEX_T index) {
  this->selectedNodes.insert(index);
  this->selectedNode = index;

  this->saved = 0;
}

void Map::deselect(INDEX_T index) {
  this->selectedNodes.erase(index);

  if (index == this->selectedNode) {
    this->selectedNode = *this->selectedNodes.begin();
  }

  this->saved = 0;
}

void Map::clearSelected() {
  this->selectedNodes.clear();
  this->saved = 0;
}

bool Map::isOnScreen(Node *node) {
  int left = -this->dx;
  int top = -this->dy;
  int right = this->WIDTH / this->zoom - this->dx;
  int bottom = this->HEIGHT / this->zoom - this->dy;

  return node->getX() + node->getRadius() > left &&
         node->getX() - node->getRadius() < right &&
         node->getY() + node->getRadius() > top &&
         node->getY() - node->getRadius() < bottom;
}

template <class Archive>
void Map::serialize(Archive &ar, const unsigned int version) {
  ar &boost::serialization::make_nvp("dx", dx);
  ar &boost::serialization::make_nvp("dy", dy);

  ar &boost::serialization::make_nvp("zoom", zoom);

  ar &boost::serialization::make_nvp("curIndex", curIndex);

  ar &boost::serialization::make_nvp("nodes", nodes);
}
