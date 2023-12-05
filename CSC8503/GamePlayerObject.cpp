
#include "GamePlayerObject.h"
GamePlayerObject::GamePlayerObject() {

}
GamePlayerObject::~GamePlayerObject() {

}
void GamePlayerObject::OnCollisionBegin(NCL::CSC8503::GameObject *otherObject) {

    if (name == "player" && otherObject->GetName() == "coin") {
        otherObject->SetIsActive(false);
        otherObject->SetBoundingVolume(nullptr);
        score += 10;
        itemsHasGet++;
        itemsLeft--;
    }
    if (name == "player" && otherObject->GetName() == "sphere") {

    }
    if (name == "player" && otherObject->GetName() == "key") {
        otherObject->SetIsActive(false);
        otherObject->SetBoundingVolume(nullptr);
        keyNum++;
    }
}

void GamePlayerObject::OnCollisionEnd(NCL::CSC8503::GameObject *otherObject) {

}

void GamePlayerObject::Update(float dt)
{

}