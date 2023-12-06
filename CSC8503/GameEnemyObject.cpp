

#include "GameEnemyObject.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "NavigationGrid.h"
#include "GamePlayerObject.h"
#include <cstdlib>

using namespace NCL;
using namespace CSC8503;

std::vector<Vector3> patrolPoints = {

};

void InitializePatrolPoints() {
    const int numPoints = 5; // 生成巡逻点的数量
    const float range = 50.0f; // 巡逻点生成的范围

    for (int i = 0; i < numPoints; ++i) {
        float x = (std::rand() / static_cast<float>(RAND_MAX)) * 2 * range - range;
        float z = (std::rand() / static_cast<float>(RAND_MAX)) * 2 * range - range;
        patrolPoints.push_back(Vector3(x, 0, z)); // 假设巡逻点都在y=0平面上
    }
}

// 当前目标巡逻点的索引
int currentPatrolIndex = 0;

GameEnemyObject::GameEnemyObject(NavigationGrid *grid, GamePlayerObject *gameObject) {

    this->grid = grid;
    this->target = gameObject;
    this->stateMachine = new StateMachine();
    counter = 20.0f;

    srand(time(NULL));

    InitializePatrolPoints();
    State *patrol = new State([&](float dt) -> void {
        this->GetPhysicsObject()->ClearForces();
        this->GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 0));
    });


    State *chase = new State([&](float dt) -> void {
                                 this->MoveToTarget(dt);
                             }
    );

    stateMachine->AddState(patrol);
    stateMachine->AddState(chase);

    stateMachine->AddTransition(new StateTransition(patrol, chase, [&]() -> bool {
        if (this->GetTarget()) {
            return (this->GetTarget()->GetTransform().GetPosition() - this->GetTransform().GetPosition()).Length() <=
                   45.0f;
        }
    }));

    stateMachine->AddTransition(new StateTransition(chase, patrol, [&]() -> bool {
        if (this->GetTarget()) {
            return (this->GetTarget()->GetTransform().GetPosition() - this->GetTransform().GetPosition()).Length() >
                   65.0f;
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
    if (target == nullptr) return;
    bool found = grid->FindPath(GetTransform().GetPosition(), target->GetTransform().GetPosition(), outPath);
    Vector3 pos;
    while (outPath.PopWaypoint(pos)) {
        pathToTarget.push_back(pos);
    }
}