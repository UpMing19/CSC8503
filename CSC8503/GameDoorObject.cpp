
#include "GameDoorObject.h"

GameDoorObject::GameDoorObject(GamePlayerObject *player) {
    this->player = player;
}

GameDoorObject::~GameDoorObject() {

}

void GameDoorObject::OnCollisionBegin(GameObject *otherObject) {

    if (name == "Door" && otherObject->GetName() == "player" && player->keyNum) {
        // otherObject->SetIsActive(false);
        // otherObject->SetBoundingVolume(nullptr);
        //   keyNum++;


    }
}

void GameDoorObject::OnCollisionEnd(NCL::CSC8503::GameObject *otherObject) {

}

void GameDoorObject::Update(float dt) {

}