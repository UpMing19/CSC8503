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
        class GameGooseObject : public GameObject {
        public:
            GameGooseObject(NavigationGrid *grid, GamePlayerObject *target);

            ~GameGooseObject();
            void InitializePatrolPoints();
            GamePlayerObject *GetTarget() {
                return target;
            }

            void SetTarget(GamePlayerObject *object) {
                target = object;
            }

            void guardOneDoor();
            void MoveToTarget(float dt);

            void CalculatePath();

            virtual void Update(float dt);

            GamePlayerObject *target;
            NavigationGrid *grid;

            float counter;

            StateMachine *stateMachine;
            std::vector<Vector3> pathToTarget;
            int currentPatrolIndex = 0;

            std::vector<Vector3> patrolPoints = {

            };
        };

    }
}


