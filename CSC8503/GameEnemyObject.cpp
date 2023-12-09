

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
    const int numPoints = 4;

    patrolPoints.push_back(Vector3(340, -12, 250));
    patrolPoints.push_back(Vector3(380, -12, 120));
    patrolPoints.push_back(Vector3(380, -12, 250));
    patrolPoints.push_back(Vector3(340, -12, 120));


}

int currentPatrolIndex = 0;

GameEnemyObject::GameEnemyObject(NavigationGrid *grid, GamePlayerObject *gameObject) {

    this->grid = grid;
    this->target = gameObject;
    this->stateMachine = new StateMachine();
    counter = 20.0f;

    srand(time(NULL));

    InitializePatrolPoints();

    State *patrol = new State([&](float dt) -> void {
        auto physObject = this->GetPhysicsObject();
        auto transform = this->GetTransform();

        // 判断是否达到当前目标点
        Vector3 currentPos = transform.GetPosition();
        Vector3 targetPos = patrolPoints[currentPatrolIndex];
        if ((targetPos - currentPos).LengthSquared() < 1.0f) { // 使用LengthSquared来避免开方操作，提高性能

            currentPatrolIndex++;// 获取一个随机的路线点索引
            currentPatrolIndex%=4;
            targetPos = patrolPoints[currentPatrolIndex]; // 更新目标点
        }

        // 计算从当前位置到目标位置的方向
        Vector3 direction = (targetPos - currentPos).Normalised();

        // 设置物理对象的速度朝向目标点
        float patrolSpeed = 50.0f; // 根据需要调整巡逻速度
        physObject->SetLinearVelocity(direction * patrolSpeed);

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
        GetPhysicsObject()->SetLinearVelocity(dir.Normalised() * 500.0f * dt);

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