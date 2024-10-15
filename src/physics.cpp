#include "physics.h"


void handle_collisions(Player& player, std::vector<Animal>& animals) {
    const float playerRadius = 1.5f; // Assuming the player cube has a side length of 2.0
    const float animalRadius = 0.5f; // From the Animal constructor
    const float tetherRadius = 0.5f; // From the Tether constructor
    const float ropeSegmentRadius = 0.5f; // From the Rope constructor

    // Player cube vs Animals
    for (auto& animal : animals) {
        if (CheckCollisionSpheres(player.pos, playerRadius, animal.pos, animalRadius)) {
            // Handle player-animal collision
            vec3 collisionNormal = Vector3Normalize(Vector3Subtract(animal.pos, player.pos));
            float overlap = playerRadius + animalRadius - Vector3Distance(player.pos, animal.pos);
            player.pos = Vector3Subtract(player.pos, Vector3Scale(collisionNormal, overlap * 0.5f));
            animal.pos = Vector3Add(animal.pos, Vector3Scale(collisionNormal, overlap * 0.5f));
        }
    }

    // Player rope vs Animals
    for (auto& animal : animals) {
        for (int i = 0; i < player.rope.num_points - 1; i++) {
            if (CheckCollisionPointLine(animal.pos, player.rope.points[i], player.rope.points[i+1], ropeSegmentRadius)) {
                // Handle rope-animal collision
                vec3 closestPoint = GetClosestPointOnLineFromPoint(animal.pos, player.rope.points[i], player.rope.points[i+1]);
                vec3 collisionNormal = Vector3Normalize(Vector3Subtract(animal.pos, closestPoint));
                float overlap = ropeSegmentRadius + animalRadius - Vector3Distance(closestPoint, animal.pos);
                animal.targ = Vector3Add(animal.targ, Vector3Scale(collisionNormal, overlap*0.8));
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
