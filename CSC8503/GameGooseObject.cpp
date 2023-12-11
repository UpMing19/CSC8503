

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

    patrolPoints.push_back(Vector3(220, -11, 240));
    patrolPoints.push_back(Vector3(130, -11, 240));
    patrolPoints.push_back(Vector3(130, -11, 140));
    patrolPoints.push_back(Vector3(220, -11, 140));


}



GameGooseObject::GameGooseObject(NavigationGrid *grid, GamePlayerObject *gameObject) {

    this->grid = grid;
    this->target = gameObject;
    this->stateMachine = new StateMachine();
    counter = 20.0f;

    InitializePatrolPoints();
    currentPatrolIndex = 0;

    State *patrol = new State([&](float dt) -> void {
        auto physObject = this->GetPhysicsObject();
        auto transform = this->GetTransform();

        Vector3 currentPos = transform.GetPosition();
        Vector3 targetPos = patrolPoints[currentPatrolIndex];
        if ((targetPos - currentPos).LengthSquared() < 1.0f) {

            currentPatrolIndex++;
            currentPatrolIndex %= 4;
            targetPos = patrolPoints[currentPatrolIndex];
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
        return (target->keyNum==0);
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
   //     std::cout<<"this->GetTransform().GetPosition():"<<this->GetTransform().GetPosition()<<std::endl;
        dir = Vector3(dir.x, 0, dir.z);
        GetPhysicsObject()->SetLinearVelocity(dir.Normalised() * 1000.0f * dt);
        pathToTarget.erase(it);
//        if (dir.Length() <= 2.0f) {
//            pathToTarget.erase(it);
//            GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 0));
//        }
    }
}

void GameGooseObject::CalculatePath() {
 //   Vector3 dir = this->GetTransform().GetPosition();
  //  std::cout<<"this->GetTransform().GetPosition():"<<this->GetTransform().GetPosition()<<std::endl;
    pathToTarget.clear();
    NavigationPath outPath;
    bool found = grid->FindPath(GetTransform().GetPosition(), target->GetTransform().GetPosition(), outPath);
 //   bool found = grid->FindPath(GetTransform().GetPosition(), Vector3(220,-11,240), outPath);
    Vector3 pos;
    while (outPath.PopWaypoint(pos)) {
        pathToTarget.push_back(pos);
    }

    for (int i = 1; i < pathToTarget.size(); ++i) {
        Vector3 a = pathToTarget[i - 1];
        Vector3 b = pathToTarget[i];

        Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
    }
}