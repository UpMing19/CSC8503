#pragma once
#include "GameObject.h"
#include "Window.h"
#include "Vector3.h"
#include "Quaternion.h"

#include "GameTechRenderer.h"
#include "PhysicsSystem.h"

#include "GameObject.h"
#include "NavigationGrid.h"
#include "NavigationPath.h"

#include "Transform.h"
#include "CollisionVolume.h"

using namespace NCL;
using namespace CSC8503;

class GamePlayerObject : public GameObject {

public:
    GamePlayerObject();

    ~GamePlayerObject();

    void OnCollisionBegin(GameObject *otherObject) override;

    void OnCollisionEnd(GameObject *otherObject) override;

     void Update(float dt);

public:
    int itemsHasGet = 0;
    int itemsLeft = 42;
    int keyNum = 0;
    int score = 0;
    bool win = false;
    bool lose = false;
};


