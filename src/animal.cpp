#include "animal.hpp"

Species::Species(SpeciesType type) : type(type) {
  switch (type) {
    case SpeciesType::NULL_SPECIES:
      color = BLUE;
      name = "Null Species";
      radius = 0.7;
      break;
    case SpeciesType::WOLF:
      color = GRAY;
      name = "Wolf";
      radius = 0.7;
      break;
    case SpeciesType::SHEEP:
      color = WHITE;
      name = "Sheep";
      radius = 0.7;
      break;
    case SpeciesType::COW:
      color = BROWN;
      name = "Cow";
      radius = 0.7;
      break;
  }
}

SpeciesType getRandomSpecies() {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<> dis(0, 2);

  int randomNumber = dis(gen);
  switch (randomNumber) {
    case 0:
      return SpeciesType::WOLF;
    case 1:
      return SpeciesType::SHEEP;
    case 2:
      return SpeciesType::COW;
    default:
      return SpeciesType::SHEEP;  // Fallback, should never happen
  }
}

Animal::Animal(vec3 pos, float speed, Shader shader)
    : pos(pos),
      speed(speed),
      shader(shader),
      lastUpdateTime(0),
      species(getRandomSpecies()) {
  targ = pos;
  model = LoadModelFromMesh(GenMeshSphere(species.radius, 20, 20));
  model.materials[0].shader = shader;
}

void Animal::setNewRandomTarget() {
  // Define the range for random movement (e.g., [-1.0, 1.0])
  float rangep = 1.0f;

  // Generate a random value within the range for both x and z coordinates
  float rangeX = ((float)GetRandomValue(-1000, 1000) / 1000.0f) * rangep;
  float rangeZ = ((float)GetRandomValue(-1000, 1000) / 1000.0f) * rangep;

  // Update the target position with the new random values
  targ.x = targ.x + rangeX;
  targ.z = targ.z + rangeZ;
}

void Animal::update() {
  double currentTime = GetTime();
  if (currentTime - lastUpdateTime >= 1.0) {
    setNewRandomTarget();
    lastUpdateTime = currentTime;
  }

  pos = lerp3D(pos, targ, 0.03);
  model.transform = MatrixTranslate(pos.x, pos.y, pos.z);
}

void Animal::draw() { DrawModel(model, Vector3Zero(), 1.0f, species.color); }

std::vector<std::unique_ptr<Animal>> CreateAnimals(
    const rl::Shader &shadowShader, int count) {
  std::vector<std::unique_ptr<Animal>> animals;
  for (int i = 0; i < count; i++) {
    animals.push_back(std::make_unique<Animal>(
        vec3{GetRandomFloat(-25, 25), 1.0f, GetRandomFloat(-25, 25)}, 5.0f,
        shadowShader));
  }
  return animals;
}
