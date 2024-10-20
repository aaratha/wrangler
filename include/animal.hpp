#pragma once

#include "raylib-cpp.hpp"
#include "utils.hpp"

#include <random>

enum class SpeciesType { NULL_SPECIES, WOLF, SHEEP, COW };

class Species {
public:
  SpeciesType type;
  Color color;
  std::string name;

  Species(SpeciesType type);
};

SpeciesType getRandomSpecies();

class Animal {
public:
  vec3 pos;
  vec3 targ;
  float speed;
  Shader shader;
  Model model;
  double lastUpdateTime;
  Species species;

  Animal(vec3 pos, float speed, Shader shader);

  void setNewRandomTarget();
  void update();
  void draw();
};

std::vector<std::unique_ptr<Animal>>
CreateAnimals(const rl::Shader &shadowShader, int count);
