#include "physics.hpp"


void handle_collisions(
    Player& player,
    std::vector<Animal>& animals,
    int& substeps,
    std::vector<std::unique_ptr<Pen>>& pens
) {
    const float playerRadius = 1.5f; // Assuming the player cube has a side length of 2.0
    const float animalRadius = 0.5f; // From the Animal constructor
    const float tetherRadius = 0.5f; // From the Tether constructor
    const float ropeSegmentRadius = 0.5f; // From the Rope constructor

    // Player cube vs Animals
    for (int i=0; i<substeps; i++) {
        for (auto& animal : animals) {
            if (CheckCollisionSpheres(player.pos, playerRadius, animal.pos, animalRadius)) {
                // Handle player-animal collision
                vec3 collisionNormal = Vector3Normalize(Vector3Subtract(animal.pos, player.pos));
                float overlap = playerRadius + animalRadius - Vector3Distance(player.pos, animal.pos);
                player.pos = Vector3Subtract(player.pos, Vector3Scale(collisionNormal, overlap * 0.5f));
                animal.pos = Vector3Add(animal.pos, Vector3Scale(collisionNormal, overlap * 0.5f));
            }
        }

        for (auto& animal : animals) {
            for (int i = 0; i < player.rope.num_points - 1; i++) {
                if (CheckCollisionPointLine(animal.pos, player.rope.points[i], player.rope.points[i+1], ropeSegmentRadius)) {
                    // Handle rope-animal collision
                    vec3 closestPoint = GetClosestPointOnLineFromPoint(animal.pos, player.rope.points[i], player.rope.points[i+1]);
                    vec3 collisionNormal = Vector3Normalize(Vector3Subtract(animal.pos, closestPoint));
                    float overlap = ropeSegmentRadius + animalRadius - Vector3Distance(closestPoint, animal.pos);
                    animal.targ = Vector3Add(animal.targ, Vector3Scale(collisionNormal, overlap*0.8));

                    // Displace rope points
                    vec3 displacementVector = Vector3Scale(collisionNormal, overlap * 0.2f);
                    player.rope.points[i] = Vector3Subtract(player.rope.points[i], displacementVector);
                    player.rope.points[i+1] = Vector3Subtract(player.rope.points[i+1], displacementVector);
                }
            }
        }

        for (auto& animal : animals) {
            for (int i = 0; i < pens.size(); i++) {
                for (int j = 0; j < pens[i]->points.size() - 1; j++) {  // Fix j++
                    vec3 point = vec2to3(pens[i]->points[j], 1.0f);
                    vec3 point_next = vec2to3(pens[i]->points[j + 1], 1.0f);
                    if (CheckCollisionPointLine(animal.pos, point, point_next, ropeSegmentRadius)) {
                        // Handle rope-animal collision
                        vec3 closestPoint = GetClosestPointOnLineFromPoint(animal.pos, point, point_next);
                        vec3 collisionNormal = Vector3Normalize(Vector3Subtract(animal.pos, closestPoint));
                        float overlap = ropeSegmentRadius + animalRadius - Vector3Distance(closestPoint, animal.pos);
                        animal.targ = Vector3Add(animal.targ, Vector3Scale(collisionNormal, overlap * 0.8f));
                    }
                }
            }
        }
        // Player tether vs Animals
        for (auto& animal : animals) {
            if (CheckCollisionSpheres(player.tether.pos, tetherRadius, animal.pos, animalRadius)) {
                // Handle tether-animal collision
                vec3 collisionNormal = Vector3Normalize(Vector3Subtract(animal.pos, player.tether.pos));
                float overlap = tetherRadius + animalRadius - Vector3Distance(player.tether.pos, animal.pos);
                animal.pos = Vector3Add(animal.pos, Vector3Scale(collisionNormal, overlap));
            }
        }

        // Animal vs Animal
        for (size_t i = 0; i < animals.size(); i++) {
            for (size_t j = i + 1; j < animals.size(); j++) {
                if (CheckCollisionSpheres(animals[i].pos, animalRadius, animals[j].pos, animalRadius)) {
                    // Handle animal-animal collision
                    vec3 collisionNormal = Vector3Normalize(Vector3Subtract(animals[j].pos, animals[i].pos));
                    float overlap = 2 * animalRadius - Vector3Distance(animals[i].pos, animals[j].pos);
                    animals[i].pos = Vector3Subtract(animals[i].pos, Vector3Scale(collisionNormal, overlap * 0.5f));
                    animals[j].pos = Vector3Add(animals[j].pos, Vector3Scale(collisionNormal, overlap * 0.5f));
                }
            }
        }
    }
}
