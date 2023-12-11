#pragma once
#include "GameObject.h"
#include "Window.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "GamePlayerObject.h"
#include "GameTechRenderer.h"
#include "PhysicsSystem.h"

#include "GameObject.h"
#include "NavigationGrid.h"
#include "NavigationPath.h"

#include "Transform.h"
#include "CollisionVolume.h"

using namespace NCL;
using namespace CSC8503;

class GameDoorObject : public GameObject {

public:
    GameDoorObject( GamePlayerObject *player);

    ~GameDoorObject();

    void OnCollisionBegin(GameObject *otherObject) override;

    void OnCollisionEnd(GameObject *otherObject) override;

     void Update(float dt);

public:
    GamePlayerObject *player;
};


