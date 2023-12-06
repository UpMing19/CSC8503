

#include "GameEnemyObject.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "NavigationGrid.h"

using namespace NCL;
using namespace CSC8503;

GameEnemyObject::GameEnemyObject(NavigationGrid *grid, GameObject *gameObject) {

    this->grid = grid;
    this->target = gameObject;
    this->stateMachine = new StateMachine();
    counter = 20.0f;

    State *patrol = new State([&](float dt) -> void {
        Vector3 currentPosition = this->GetTransform().GetPosition();

        // Generate random target position within a certain range
        float minX = -10.0f;  // Minimum x-coordinate of patrol area
        float maxX = 10.0f;   // Maximum x-coordinate of patrol area
        float minZ = -10.0f;  // Minimum z-coordinate of patrol area
        float maxZ = 10.0f;   // Maximum z-coordinate of patrol area

        float targetX = minX + static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX / (maxX - minX));
        float targetZ = minZ + static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX / (maxZ - minZ));

        Vector3 targetPosition(targetX, currentPosition.y, targetZ);

        // Calculate direction towards the target position
        Vector3 direction = targetPosition - currentPosition;
        direction.Normalised();

        // Set linear velocity to move towards the target position
        float patrolSpeed = 5.0f;  // Speed of patrol movement
        this->GetPhysicsObject()->SetLinearVelocity(direction * patrolSpeed);
    });

    State *chase = new State([&](float dt) -> void {
                                 this->MoveToTarget(dt);
                             }
    );
    stateMachine->AddState(patrol);
    stateMachine->AddState(chase);

    stateMachine->AddTransition(new StateTransition(patrol, chase, [&]() -> bool {
        if (this->GetTarget()) {
            return (this->GetTarget()->GetTransform().GetPosition() - this->GetTransform().GetPosition()).Length() <= 25.0f;
        }
    }));

    stateMachine->AddTransition(new StateTransition(chase, patrol, [&]() -> bool {
        if (this->GetTarget()) {
            return (this->GetTarget()->GetTransform().GetPosition() - this->GetTransform().GetPosition()).Length() >= 25.0f;
        }
    }));


}

GameEnemyObject::~GameEnemyObject() {
    delete stateMachine;
}

void GameEnemyObject::Update(float dt) {
    if (counter > 2.0f) {
        CalculatePath();
        counter = 0.0f;
    }
    stateMachine->Update(dt);
    counter += dt;
}


void GameEnemyObject::MoveToTarget(float dt) {
    if (pathToTarget.size() > 0) {
        auto it = pathToTarget.begin();
        Vector3 target = *it;
        Vector3 dir = (target - this->GetTransform().GetPosition());
        dir = Vector3(dir.x, 0, dir.z);
        GetPhysicsObject()->SetLinearVelocity(dir.Normalised() * 1000.0f * dt);

        if (dir.Length() <= 2.0f) {
            pathToTarget.erase(it);
            GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 0));
        }
    }
}

void GameEnemyObject::CalculatePath() {
    pathToTarget.clear();
    NavigationPath outPath;
    if (target == nullptr)return;
    bool found = grid->FindPath(GetTransform().GetPosition(), target->GetTransform().GetPosition(), outPath);
    Vector3 pos;
    while (outPath.PopWaypoint(pos)) {
        pathToTarget.push_back(pos);
    }
}