

#include "MenuStates.h"

#include "PushdownMachine.h"
#include "PushdownState.h"

using namespace NCL;
using namespace CSC8503;

PushdownState::PushdownResult MenuState::OnUpdate(float dt, NCL::CSC8503::PushdownState **newState) {

    if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM1)) {
        *newState = new InitWorldState(g);
        return PushdownResult::Push;
    }
    if (Window::GetKeyboard()->KeyDown(KeyCodes::ESCAPE)) {
        return PushdownResult::Pop;
    }
    return PushdownResult::NoChange;
}

void MenuState::OnAwake() {
    g->InitMenu();
}

PushdownState::PushdownResult InitWorldState::OnUpdate(float dt, PushdownState **newState) {
    if (Window::GetKeyboard()->KeyDown(KeyCodes::F1))
        g->InitWorld();
    if (Window::GetKeyboard()->KeyDown(KeyCodes::F3))
        return PushdownResult::Pop;

    if (Window::GetKeyboard()->KeyDown(KeyCodes::ESCAPE))
        return PushdownResult::Pop;

    return PushdownResult::NoChange;
}

void InitWorldState::OnAwake() {
    g->InitWorld();
}