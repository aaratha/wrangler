#include "collectables.hpp"

Coin::Coin(std::vector<vec3> bounds) {
  // Triangulate the polygon in the xz plane
  std::vector<std::array<vec2, 3>> triangles = triangulatePolygon(bounds);

  // Randomly select a triangle based on its area
  std::array<vec2, 3> selectedTriangle = selectRandomTriangle(triangles);

  // Generate a random point within the selected triangle
  vec2 randomPoint2D = generateRandomPointInTriangle(selectedTriangle);

  // Extend to vec3, keeping y = 0 (xz plane)
  pos = vec3{randomPoint2D.x, 1.0f, randomPoint2D.y};
}

void Coin::draw() { DrawSphere(pos, 0.2f, YELLOW); }
