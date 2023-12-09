

#include "GameGooseObject.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "NavigationGrid.h"
#include "GamePlayerObject.h"
#include <cstdlib>

using namespace NCL;
using namespace CSC8503;


void GameGooseObject::InitializePatrolPoints() {
    const int numPoints = 4;

    patrolPoints.push_back(Vector3(210, -11, 220));
    patrolPoints.push_back(Vector3(140, -11, 220));
    patrolPoints.push_back(Vector3(140, -11, 150));
    patrolPoints.push_back(Vector3(210, -11, 150));


}



GameGooseObject::GameGooseObject(NavigationGrid *grid, GamePlayerObject *gameObject) {

    this->grid = grid;
    this->target = gameObject;
    this->stateMachine = new StateMachine();
    counter = 20.0f;

    srand(time(NULL));

    InitializePatrolPoints();

    State *patrol = new State([&](float dt) -> void {
        auto physObject = this->GetPhysicsObject();
        auto transform = this->GetTransform();

        Vector3 currentPos = transform.GetPosition();
        Vector3 targetPos = patrolPoints[currentPatrolIndex];
        if ((targetPos - currentPos).LengthSquared() < 1.0f) {

            currentPatrolIndex++;// 获取一个随机的路线点索引
            currentPatrolIndex %= 4;
            targetPos = patrolPoints[currentPatrolIndex]; // 更新目标点
        }
        Vector3 direction = (targetPos - currentPos).Normalised();

        float patrolSpeed = 50.0f;
        physObject->SetLinearVelocity(direction * patrolSpeed);

    });


    State *chase = new State([&](float dt) -> void {
                                 this->MoveToTarget(dt);
                             }
    );

    stateMachine->AddState(patrol);
    stateMachine->AddState(chase);

    stateMachine->AddTransition(new StateTransition(patrol, chase, [&]() -> bool {
        return target->keyNum;
    }));

    stateMachine->AddTransition(new StateTransition(chase, patrol, [&]() -> bool {
        return target->keyNum;
    }));


}

GameGooseObject::~GameGooseObject() {
    delete stateMachine;
}

void GameGooseObject::Update(float dt) {
    if (counter > 2.0f) {
        CalculatePath();
        counter = 0.0f;
    }
    stateMachine->Update(dt);
    counter += dt;
}


void GameGooseObject::MoveToTarget(float dt) {

    if (pathToTarget.size() > 0) {
        auto it = pathToTarget.begin();
        Vector3 target = *it;
        Vector3 dir = (target - this->GetTransform().GetPosition());
        dir = Vector3(dir.x, 0, dir.z);
        GetPhysicsObject()->SetLinearVelocity(dir.Normalised() * 500.0f * dt);

        if (dir.Length() <= 2.0f) {
            pathToTarget.erase(it);
            GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 0));
        }
    }
}

void GameGooseObject::CalculatePath() {
    pathToTarget.clear();
    NavigationPath outPath;
    bool found = grid->FindPath(GetTransform().GetPosition(), Vector3(210,-11,220), outPath);
    Vector3 pos;
    while (outPath.PopWaypoint(pos)) {
        pathToTarget.push_back(pos);
    }
}