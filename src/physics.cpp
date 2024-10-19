#include "physics.hpp"


void handle_collisions(
    GameState& GameState,
    int& substeps,
    std::vector<std::unique_ptr<Pen>>& pens
) {
    const float playerRadius = 1.5f; // Assuming the player cube has a side length of 2.0
    const float animalRadius = 0.5f; // From the Animal constructor
    const float tetherRadius = 0.5f; // From the Tether constructor
    const float ropeSegmentRadius = 0.5f; // From the Rope constructor

    // Player cube vs Animals
    for (int i=0; i<substeps; i++) {
        for (auto& animal : GameState.animals) {
            if (CheckCollisionSpheres(GameState.player->pos, playerRadius, animal->pos, animalRadius)) {
                // Handle player-animal collision
                vec3 collisionNormal = Vector3Normalize(Vector3Subtract(animal->pos, GameState.player->pos));
                float overlap = playerRadius + animalRadius - Vector3Distance(GameState.player->pos, animal->pos);
                GameState.player->pos = Vector3Subtract(GameState.player->pos, Vector3Scale(collisionNormal, overlap * 0.5f));
                animal->pos = Vector3Add(animal->pos, Vector3Scale(collisionNormal, overlap * 0.5f));
            }
        }

        for (auto& animal : GameState.animals) {
            for (int i = 0; i < GameState.player->rope.num_points - 1; i++) {
                if (CheckCollisionPointLine(animal->pos, GameState.player->rope.points[i], GameState.player->rope.points[i+1], ropeSegmentRadius)) {
                    // Handle rope-animal collision
                    vec3 closestPoint = GetClosestPointOnLineFromPoint(animal->pos, GameState.player->rope.points[i], GameState.player->rope.points[i+1]);
                    vec3 collisionNormal = Vector3Normalize(Vector3Subtract(animal->pos, closestPoint));
                    float overlap = ropeSegmentRadius + animalRadius - Vector3Distance(closestPoint, animal->pos);
                    animal->targ = Vector3Add(animal->targ, Vector3Scale(collisionNormal, overlap*0.8));

                    // Displace rope points
                    vec3 displacementVector = Vector3Scale(collisionNormal, overlap * 0.2f);
                    GameState.player->rope.points[i] = Vector3Subtract(GameState.player->rope.points[i], displacementVector);
                    GameState.player->rope.points[i+1] = Vector3Subtract(GameState.player->rope.points[i+1], displacementVector);
                }
            }
        }

        for (auto& animal : GameState.animals) {
            for (int i = 0; i < pens.size(); i++) {
                for (int j = 0; j < pens[i]->points.size() - 1; j++) {  // Fix j++
                    vec3 point = vec2to3(pens[i]->points[j], 1.0f);
                    vec3 point_next = vec2to3(pens[i]->points[j + 1], 1.0f);
                    if (CheckCollisionPointLine(animal->pos, point, point_next, ropeSegmentRadius)) {
                        // Handle rope-animal collision
                        vec3 closestPoint = GetClosestPointOnLineFromPoint(animal->pos, point, point_next);
                        vec3 collisionNormal = Vector3Normalize(Vector3Subtract(animal->pos, closestPoint));
                        float overlap = ropeSegmentRadius + animalRadius - Vector3Distance(closestPoint, animal->pos);
                        animal->targ = Vector3Add(animal->targ, Vector3Scale(collisionNormal, overlap * 0.8f));
                    }
                }
            }
        }
        // Player tether vs Animals
        for (auto& animal : GameState.animals) {
            if (CheckCollisionSpheres(GameState.player->tether.pos, tetherRadius, animal->pos, animalRadius)) {
                // Handle tether-animal collision
                vec3 collisionNormal = Vector3Normalize(Vector3Subtract(animal->pos, GameState.player->tether.pos));
                float overlap = tetherRadius + animalRadius - Vector3Distance(GameState.player->tether.pos, animal->pos);
                animal->pos = Vector3Add(animal->pos, Vector3Scale(collisionNormal, overlap));
            }
        }

        // Animal vs Animal
        for (size_t i = 0; i < GameState.animals.size(); i++) {
            for (size_t j = i + 1; j < GameState.animals.size(); j++) {
                if (CheckCollisionSpheres(GameState.animals[i]->pos, animalRadius, GameState.animals[j]->pos, animalRadius)) {
                    // Handle animal-animal collision
                    vec3 collisionNormal = Vector3Normalize(Vector3Subtract(GameState.animals[j]->pos, GameState.animals[i]->pos));
                    float overlap = 2 * animalRadius - Vector3Distance(GameState.animals[i]->pos, GameState.animals[j]->pos);
                    GameState.animals[i]->pos = Vector3Subtract(GameState.animals[i]->pos, Vector3Scale(collisionNormal, overlap * 0.5f));
                    GameState.animals[j]->pos = Vector3Add(GameState.animals[j]->pos, Vector3Scale(collisionNormal, overlap * 0.5f));
                }
            }
        }
    }
}
