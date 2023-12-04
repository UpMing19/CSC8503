#pragma once

#include "TutorialGame.h"
#include "PushdownMachine.h"
#include "PushdownState.h"
#include "Window.h"


namespace NCL {
    namespace CSC8503 {

        class MenuState : public PushdownState {
        public:
            MenuState(TutorialGame *g) { this->g = g; };

            PushdownResult OnUpdate(float dt, PushdownState **newState) override;

            void OnAwake()override;

        protected:
            TutorialGame *g;
        };

        class InitWorldState : public PushdownState {
        public:
            InitWorldState(TutorialGame *g) { this->g = g; };

            PushdownResult OnUpdate(float dt, PushdownState **newState) override;

            void OnAwake()override;

        protected:
            TutorialGame *g;
        };

    }
}


