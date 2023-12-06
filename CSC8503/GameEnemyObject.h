#pragma once

#include "GameObject.h"
#include"PhysicsSystem.h"
#include"PhysicsObject.h"
#include "StateMachine.h"
#include "NavigationGrid.h"
#include "StateTransition.h"
#include "State.h"
#include "StateMachine.h"
#include "NavigationGrid.h"
#include "GamePlayerObject.h"

namespace NCL {
    namespace CSC8503 {
        class GameEnemyObject : public GameObject {
        public:
            GameEnemyObject(NavigationGrid *grid, GamePlayerObject *target);

            ~GameEnemyObject();

            GameObject *GetTarget() {
                return target;
            }

            void SetTarget(GamePlayerObject *object) {
                target = object;
            }

            void MoveToTarget(float dt);

            void CalculatePath();

            std::vector<Vector3> GetPath() { return pathToTarget; }

            virtual void Update(float dt);


            GamePlayerObject *target; //todo PlayerObject?
            NavigationGrid *grid;
            std::vector<Vector3> pathToTarget;
            float counter;

            StateMachine *stateMachine;

        };

    }
}


