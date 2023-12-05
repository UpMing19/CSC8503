
#include "GameObject.h"

using namespace NCL;
using namespace CSC8503;

class GamePlayerObject : public GameObject {

public:
    GamePlayerObject();

    ~GamePlayerObject();

    void OnCollisionBegin(GameObject *otherObject) override;

    void OnCollisionEnd(GameObject *otherObject) override;

    virtual void Update(float dt);

public:
    int itemsHasGet = 0;
    int itemsLeft = 16;
    int keyNum = 0;
    int score = 0;
    bool win = false;
    bool lose = false;
};


