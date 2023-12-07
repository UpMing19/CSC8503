#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"

#include "PositionConstraint.h"
#include "OrientationConstraint.h"
#include "StateGameObject.h"


using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame() : controller(*Window::GetWindow()->GetKeyboard(), *Window::GetWindow()->GetMouse()) {
    world = new GameWorld();
#ifdef USEVULKAN
    renderer	= new GameTechVulkanRenderer(*world);
    renderer->Init();
    renderer->InitStructures();
#else
    renderer = new GameTechRenderer(*world);
#endif

    physics = new PhysicsSystem(*world);

    forceMagnitude = 10.0f;
    useGravity = false;
    inSelectionMode = false;

    gameCurrentTime = gameTime;

    world->GetMainCamera().SetController(controller);

    controller.MapAxis(0, "Sidestep");
    controller.MapAxis(1, "UpDown");
    controller.MapAxis(2, "Forward");

    controller.MapAxis(3, "XLook");
    controller.MapAxis(4, "YLook");

    InitialiseAssets();
    // BridgeConstraintTest();
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
    cubeMesh = renderer->LoadMesh("cube.msh");
    sphereMesh = renderer->LoadMesh("sphere.msh");
    coinMesh = renderer->LoadMesh("coin.msh");
    charMesh = renderer->LoadMesh("goat.msh");
    enemyMesh = renderer->LoadMesh("Keeper.msh");
    bonusMesh = renderer->LoadMesh("apple.msh");
    capsuleMesh = renderer->LoadMesh("capsule.msh");
    gooseMesh = renderer->LoadMesh("goose.msh");
    cylinderMesh = renderer->LoadMesh("Cylinder.msh");

    basicTex = renderer->LoadTexture("checkerboard.png");
    basicShader = renderer->LoadShader("scene.vert", "scene.frag");

    InitCamera();
    InitWorld();
}

TutorialGame::~TutorialGame() {
    delete cubeMesh;
    delete sphereMesh;
    delete charMesh;
    delete enemyMesh;
    delete bonusMesh;

    delete basicTex;
    delete basicShader;

    delete physics;
    delete renderer;
    delete world;
}

void TutorialGame::UpdateGame(float dt) {

    if (menu) {
        world->GetMainCamera().SetPosition(Vector3(0, 0, 0));
        Debug::Print("1. Start Game ", Vector2(30, 40), Debug::GREEN);
        Debug::Print("Exit - Press ESC", Vector2(30, 50), Debug::GREEN);
        physics->Update(dt);


        world->UpdateWorld(dt);
        renderer->Update(dt);
        renderer->Render();

        Debug::UpdateRenderables(dt);
        return;
    }
    //  std::cout<<world->GetMainCamera().GetPosition()<<std::endl;
    //  std::cout<<world->GetMainCamera().GetPitch()<<std::endl;
    //   std::cout<<world->GetMainCamera().GetYaw()<<std::endl;
    if (!inSelectionMode) {
        world->GetMainCamera().UpdateCamera(dt);
    }
    if (lockedObject != nullptr) {
        Vector3 objPos = lockedObject->GetTransform().GetPosition();
        Vector3 camPos = objPos + lockedOffset;

        Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

        Matrix4 modelMat = temp.Inverse();

        Quaternion q(modelMat);
        Vector3 angles = q.ToEuler(); //nearly there now!

        //todo follow camrea
        //world->GetMainCamera().SetPosition(camPos);
        //world->GetMainCamera().SetPitch(angles.x);
        //world->GetMainCamera().SetYaw(angles.y);
    }

    UpdateKeys();


    RayCollision closestCollision;
    if (Window::GetKeyboard()->KeyPressed(KeyCodes::K) && selectionObject) {
        Vector3 rayPos;
        Vector3 rayDir;

        rayDir = selectionObject->GetTransform().GetOrientation() * Vector3(0, 0, -1);

        rayPos = selectionObject->GetTransform().GetPosition();

        Ray r = Ray(rayPos, rayDir);

        if (world->Raycast(r, closestCollision, true, selectionObject)) {
            if (objClosest) {
                objClosest->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
            }
            objClosest = (GameObject *) closestCollision.node;

            objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
        }
    }

    Debug::DrawLine(Vector3(), Vector3(0, 100, 0), Vector4(1, 0, 0, 1));
    Debug::DrawLine(Vector3(), Vector3(100, 0, 0), Vector4(0, 1, 0, 1));
    Debug::DrawLine(Vector3(), Vector3(0, 0, 100), Vector4(0, 0, 1, 1));


    if (!player->win && !player->lose) {
        gameCurrentTime -= dt;
        int minutes = floor(gameCurrentTime / 60.0f);
        int seconds = std::round(std::fmod(gameCurrentTime, 60.0f));
        if (seconds == 60) seconds = 0, minutes++;
        Vector4 timerColor = gameCurrentTime <= 20.0f ? Debug::RED : Debug::YELLOW;
        std::string time = "Time: " + std::to_string(minutes) + ":" + std::to_string(seconds);
        Debug::Print(time, Vector2(90 - time.length(), 5), timerColor);
        if (gameCurrentTime <= 0.0f) {
            gameCurrentTime = 0.0f;
            player->lose = true;
        }

        std::string score = "Score = " + std::to_string(player->score);
        Debug::Print(score, Vector2(90 - time.length(), 10), timerColor);

        std::string itemsHasGet = "ItemsHasGot = " + std::to_string(player->itemsHasGet);
        Debug::Print(itemsHasGet, Vector2(90 - time.length() - 10, 15), timerColor);

        std::string itemsLeft = "ItemsLeft = " + std::to_string(player->itemsLeft);
        Debug::Print(itemsLeft, Vector2(90 - time.length() - 10, 20), timerColor);



        std::string ComeBackMenu = "F3 :ComeBackMenu ";
        Debug::Print(ComeBackMenu, Vector2(0 , 10), Debug::BLUE);

        std::string Remake = "F1 :Remake  ";
        Debug::Print(Remake, Vector2(0 , 20), Debug::BLUE);




    }


    if (player->win || player->lose) {
        world->UpdateWorld(dt);
        renderer->Update(dt);
        physics->Update(dt);
        EndGame();
        renderer->Render();
        return;
    }


    SelectObject();
    MoveSelectedObject();

    if (useGravity) {
        Debug::Print("(G)ravity on", Vector2(5, 95), Debug::RED);
    } else {
        Debug::Print("(G)ravity off", Vector2(5, 95), Debug::RED);
    }

    world->UpdateWorld(dt);
    renderer->Update(dt);
    physics->Update(dt);

    if (testStateObject && cylinderStateObject) {
        //std::cout<<"debug"<<std::endl;
        cylinderStateObject->Update(dt);
        testStateObject->Update(dt);
    }
    if (EnemyObject != nullptr) {
        EnemyObject->Update(dt);
    }


    renderer->Render();
    Debug::UpdateRenderables(dt);
}

void TutorialGame::UpdateKeys() {
    if (Window::GetKeyboard()->KeyPressed(KeyCodes::F1)) {
        InitWorld(); //We can reset the simulation at any time with F1
        selectionObject = nullptr;
    }

    if (Window::GetKeyboard()->KeyPressed(KeyCodes::F2)) {
        InitCamera(); //F2 will reset the camera to a specific default place
    }

    if (Window::GetKeyboard()->KeyPressed(KeyCodes::G)) {
        useGravity = !useGravity; //Toggle gravity!
        physics->UseGravity(useGravity);
    }
    //Running certain physics updates in a consistent order might cause some
    //bias in the calculations - the same objects might keep 'winning' the constraint
    //allowing the other one to stretch too much etc. Shuffling the order so that it
    //is random every frame can help reduce such bias.
    if (Window::GetKeyboard()->KeyPressed(KeyCodes::F9)) {
        world->ShuffleConstraints(true);
    }
    if (Window::GetKeyboard()->KeyPressed(KeyCodes::F10)) {
        world->ShuffleConstraints(false);
    }

    if (Window::GetKeyboard()->KeyPressed(KeyCodes::F7)) {
        world->ShuffleObjects(true);
    }
    if (Window::GetKeyboard()->KeyPressed(KeyCodes::F8)) {
        world->ShuffleObjects(false);
    }

    if (lockedObject) {
        LockedObjectMovement();
    } else {
        DebugObjectMovement();
    }
}

void TutorialGame::LockedObjectMovement() {

    Matrix4 view = world->GetMainCamera().BuildViewMatrix();
    Matrix4 camWorld = view.Inverse();

    Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

    //forward is more tricky -  camera forward is 'into' the screen...
    //so we can take a guess, and use the cross of straight up, and
    //the right axis, to hopefully get a vector that's good enough!

    Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
    fwdAxis.y = 0.0f;
    fwdAxis.Normalise();
    if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE) && selectionObject->GetName() == "player") {
        selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 1, 0) * 16.0f * 50.0f);
    }

    if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) {
        Quaternion pigOrientation = selectionObject->GetPhysicsObject()->GetTransForm()->GetOrientation();

        Vector3 forwardDirection = pigOrientation * Vector3(0, 0, -1);

        forwardDirection.Normalise();

        float forceMagnitude = 15.0f;
        selectionObject->GetPhysicsObject()->AddForce(forwardDirection * forceMagnitude);
    }


    if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) {
        Quaternion pigOrientation = selectionObject->GetPhysicsObject()->GetTransForm()->GetOrientation();
        Vector3 backwardDirection = pigOrientation * Vector3(0, 0, 1);
        backwardDirection.Normalise();

        float forceMagnitude = 15.0f;
        selectionObject->GetPhysicsObject()->AddForce(backwardDirection * forceMagnitude);
    }

    if (Window::GetKeyboard()->KeyDown(KeyCodes::LEFT)) {
        Quaternion pigOrientation = selectionObject->GetPhysicsObject()->GetTransForm()->GetOrientation();
        Vector3 leftDirection = pigOrientation * Vector3(-1, 0, 0);
        leftDirection.Normalise();
        float forceMagnitude = 15.0f;
        selectionObject->GetPhysicsObject()->AddForce(leftDirection * forceMagnitude);
    }

    if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
        Quaternion pigOrientation = selectionObject->GetPhysicsObject()->GetTransForm()->GetOrientation();
        Vector3 rightDirection = pigOrientation * Vector3(1, 0, 0);
        rightDirection.Normalise();
        float forceMagnitude = 15.0f;
        selectionObject->GetPhysicsObject()->AddForce(rightDirection * forceMagnitude);
    }


    if (Window::GetKeyboard()->KeyDown(KeyCodes::A)) {
        selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 1, 0));
    }

    if (Window::GetKeyboard()->KeyDown(KeyCodes::D)) {
        selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -1, 0));
    }


}

void TutorialGame::DebugObjectMovement() {
//If we've selected an object, we can manipulate it with some key presses
    if (inSelectionMode && selectionObject) {
        //Twist the selected object!
        if (Window::GetKeyboard()->KeyDown(KeyCodes::LEFT)) {
            selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
        }

        if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
            selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
        }

        if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM7)) {
            selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
        }

        if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM8)) {
            selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
        }

        if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
            selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
        }

        if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) {
            selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
        }

        if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) {
            selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
        }

        if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM5)) {
            selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
        }
    }
}

void TutorialGame::InitCamera() {
    world->GetMainCamera().SetNearPlane(0.1f);
    world->GetMainCamera().SetFarPlane(5000.0f);
    world->GetMainCamera().SetPitch(-45);
    world->GetMainCamera().SetYaw(180.0f);
    world->GetMainCamera().SetPosition(Vector3(20, 190, -90));

    lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
    world->ClearAndErase();
    physics->Clear();

    menu = false;


    // InitMixedGridWorld(15, 15, 3.5f, 3.5f);

    //InitGameExamples();
    InitDefaultFloor();
    InitMazeWorld();
    InitGamePlayerObject();
    InitGameToolsObject();
    //  EnemyObject = AddGameEnemyObject(Vector3(100, -15, 150));
    testStateObject = AddStateObjectToWorld(Vector3(70, -10, 100));
    cylinderStateObject = AddStateObjectToWorld(Vector3(300, -10, 280),cylinderMesh);
//    AddCubeToWorld(Vector3(20,20,10),Vector3(1,1,1),10,"cubetest");
//    AddSphereToWorld(Vector3(20,20,30),1,10,"spheretest");
//    AddEnemyToWorld(Vector3(20,20,50),"enemyTest");
//    AddBonusToWorld(Vector3(20,20,70),"bonusTest");

}

/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject *TutorialGame::AddFloorToWorld(const Vector3 &position, std::string name) {
    GameObject *floor = new GameObject(name);

    Vector3 floorSize = Vector3(200, 2, 200);
    AABBVolume *volume = new AABBVolume(floorSize);
    floor->SetBoundingVolume((CollisionVolume *) volume);
    floor->GetTransform()
            .SetScale(floorSize * 2)
            .SetPosition(position);

    floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
    floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

    floor->GetPhysicsObject()->SetInverseMass(0);
    floor->GetPhysicsObject()->InitCubeInertia();

    world->AddGameObject(floor);

    return floor;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple' 
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject *TutorialGame::AddSphereToWorld(const Vector3 &position, float radius, float inverseMass, std::string name) {
    GameObject *sphere = new GameObject(name);

    Vector3 sphereSize = Vector3(radius, radius, radius);
    SphereVolume *volume = new SphereVolume(radius);
    sphere->SetBoundingVolume((CollisionVolume *) volume);

    sphere->GetTransform()
            .SetScale(sphereSize)
            .SetPosition(position);

    sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
    sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

    sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
    sphere->GetPhysicsObject()->InitSphereInertia();

    world->AddGameObject(sphere);

    return sphere;
}

GameObject *
TutorialGame::AddCoinToWorldWithColor(const Vector3 &position, float radius, float inverseMass, std::string name,
                                      Vector4 color) {
    GameObject *sphere = new GameObject(name);

    Vector3 sphereSize = Vector3(radius, radius, radius);
    SphereVolume *volume = new SphereVolume(radius);
    sphere->SetBoundingVolume((CollisionVolume *) volume);

    sphere->GetTransform()
            .SetScale(sphereSize)
            .SetPosition(position);

    sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), coinMesh, basicTex, basicShader));
    sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

    sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
    sphere->GetPhysicsObject()->InitSphereInertia();

    sphere->GetRenderObject()->SetColour(color);
    world->AddGameObject(sphere);

    return sphere;
}

GameObject *
TutorialGame::AddCubeToWorld(const Vector3 &position, Vector3 dimensions, float inverseMass, std::string name) {
    GameObject *cube = new GameObject(name);

    AABBVolume *volume = new AABBVolume(dimensions);
    cube->SetBoundingVolume((CollisionVolume *) volume);

    cube->GetTransform()
            .SetPosition(position)
            .SetScale(dimensions * 2);

    cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
    cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

    cube->GetPhysicsObject()->SetInverseMass(inverseMass);
    cube->GetPhysicsObject()->InitCubeInertia();

    world->AddGameObject(cube);

    return cube;
}

GameObject *TutorialGame::AddPlayerToWorld(const Vector3 &position, std::string name) {
    float meshSize = 1.0f;
    float inverseMass = 0.5f;

    GameObject *character = new GameObject(name);
    SphereVolume *volume = new SphereVolume(1.0f);

    character->SetBoundingVolume((CollisionVolume *) volume);

    character->GetTransform()
            .SetScale(Vector3(meshSize, meshSize, meshSize))
            .SetPosition(position);

    character->SetRenderObject(new RenderObject(&character->GetTransform(), charMesh, nullptr, basicShader));
    character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

    character->GetPhysicsObject()->SetInverseMass(inverseMass);
    character->GetPhysicsObject()->InitSphereInertia();

    world->AddGameObject(character);

    return character;
}

GameObject *TutorialGame::AddEnemyToWorld(const Vector3 &position, std::string name) {
    float meshSize = 3.0f;
    float inverseMass = 0.5f;

    GameObject *character = new GameObject(name);

    AABBVolume *volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
    character->SetBoundingVolume((CollisionVolume *) volume);

    character->GetTransform()
            .SetScale(Vector3(meshSize, meshSize, meshSize))
            .SetPosition(position);

    character->SetRenderObject(
            new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));// todo can change capsule
    character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

    character->GetPhysicsObject()->SetInverseMass(inverseMass);
    character->GetPhysicsObject()->InitSphereInertia();

    world->AddGameObject(character);

    return character;
}

GameObject *TutorialGame::AddBonusToWorld(const Vector3 &position, std::string name) {
    GameObject *apple = new GameObject();

    SphereVolume *volume = new SphereVolume(0.5f);
    apple->SetBoundingVolume((CollisionVolume *) volume);
    apple->GetTransform()
            .SetScale(Vector3(2, 2, 2))
            .SetPosition(position);

    apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
    apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

    apple->GetPhysicsObject()->SetInverseMass(1.0f);
    apple->GetPhysicsObject()->InitSphereInertia();

    world->AddGameObject(apple);

    return apple;
}

void TutorialGame::InitDefaultFloor() {
    AddFloorToWorld(Vector3(200, -20, 200), "floor");
}

void TutorialGame::InitGameExamples() {
    AddPlayerToWorld(Vector3(0, 5, 0));
    AddEnemyToWorld(Vector3(5, 5, 0));
    //AddBonusToWorld(Vector3(10, 5, 0));


}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
    for (int x = 0; x < numCols; ++x) {
        for (int z = 0; z < numRows; ++z) {
            Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
            AddSphereToWorld(position, radius, 1.0f);
        }
    }
    //AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
    float sphereRadius = 1.0f;
    Vector3 cubeDims = Vector3(1, 1, 1);

    for (int x = 0; x < numCols; ++x) {
        for (int z = 0; z < numRows; ++z) {
            Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

            if (rand() % 2) {
                AddCubeToWorld(position, cubeDims);
            } else {
                AddSphereToWorld(position, sphereRadius);
            }
        }
    }
}

void
TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3 &cubeDims) {
    for (int x = 1; x < numCols + 1; ++x) {
        for (int z = 1; z < numRows + 1; ++z) {
            Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
            AddCubeToWorld(position, cubeDims, 1.0f);
        }
    }
}

/*
Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be 
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around. 

*/
bool TutorialGame::SelectObject() {
    if (Window::GetKeyboard()->KeyPressed(KeyCodes::Q)) {
        inSelectionMode = !inSelectionMode;
        if (inSelectionMode) {
            Window::GetWindow()->ShowOSPointer(true);
            Window::GetWindow()->LockMouseToWindow(false);
        } else {
            Window::GetWindow()->ShowOSPointer(false);
            Window::GetWindow()->LockMouseToWindow(true);
        }
    }
    if (inSelectionMode) {
        Debug::Print("Press Q to change to camera mode!", Vector2(5, 85));

        if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::Left)) {
            if (selectionObject) {    //set colour to deselected;
                selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
                selectionObject = nullptr;
            }

            Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());

            RayCollision closestCollision;
            if (world->Raycast(ray, closestCollision, true)) {
                selectionObject = (GameObject *) closestCollision.node;

                {
                    std::string pos = "Position = ";
                    pos.append(std::to_string(selectionObject->GetTransform().GetPosition().x));
                    pos.append(", ");
                    pos.append(std::to_string(selectionObject->GetTransform().GetPosition().y));
                    pos.append(", ");
                    pos.append(std::to_string(selectionObject->GetTransform().GetPosition().z));

                    Debug::Print(pos, Vector2(5, 60));
                }

                selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
                return true;
            } else {
                return false;
            }
        }
        if (Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::L)) {
            if (selectionObject) {
                if (lockedObject == selectionObject) {
                    lockedObject = nullptr;
                } else {
                    lockedObject = selectionObject;
                }
            }
        }
    } else {
        Debug::Print("Press Q to change to select mode!", Vector2(5, 85));
    }
    return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/

void TutorialGame::MoveSelectedObject() {
    // renderer -> DrawString ( " Click Force : " + std :: to_string ( forceMagnitude ) , Vector2 (10 , 20));
    Debug::Print("Click Force:" + std::to_string(forceMagnitude), Vector2(5, 90));
    forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

    if (!selectionObject) {
        return;//we haven't selected anything!
    }
    //Push the selected object!
    if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::Right)) {
        Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());

        RayCollision closestCollision;
        if (world->Raycast(ray, closestCollision, true)) {
            if (closestCollision.node == selectionObject) {
                selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude,
                                                                        closestCollision.collidedAt);
            }
        }
    }
}


void TutorialGame::BridgeConstraintTest() {
    Vector3 cubeSize = Vector3(8, 8, 8);
    float invCubeMass = 5;  // how heavy the middle pieces are
    int numLinks = 10;
    float maxDistance = 30;  // constraint distance
    float cubeDistance = 20;  // distance between links

    Vector3 startPos = Vector3(5, 100, 5);

    GameObject *start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);
    GameObject *end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, 0, 0), cubeSize, 0);

    GameObject *previous = start;

    for (int i = 0; i < numLinks; ++i) {
        GameObject *block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);
        PositionConstraint *constraint = new PositionConstraint(previous, block, maxDistance);
        world->AddConstraint(constraint);
        previous = block;
    }
    PositionConstraint *constraint = new PositionConstraint(previous, end, maxDistance);
    world->AddConstraint(constraint);
}

StateGameObject *TutorialGame::AddStateObjectToWorld(const Vector3 &position) {
    StateGameObject *sphere = new StateGameObject();
    sphere->SetName("MovingItem");
    Vector3 sphereSize = Vector3(10.0f, 10.0f, 10.0f);
    SphereVolume *volume = new SphereVolume(10.0f);
    sphere->SetBoundingVolume((CollisionVolume *) volume);

    sphere->GetTransform()
            .SetScale(sphereSize)
            .SetPosition(position);

    sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
    sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

    sphere->GetRenderObject()->SetColour(Vector4(0, 0.5f, 0.5f, 1));

    sphere->GetPhysicsObject()->SetInverseMass(1.0f);
    sphere->GetPhysicsObject()->InitSphereInertia();

    world->AddGameObject(sphere);

    return sphere;
}
CylinderStateGameObject *TutorialGame::AddStateObjectToWorld(const Vector3 &position,Mesh *mesh) {
    CylinderStateGameObject *sphere = new CylinderStateGameObject();
    sphere->SetName("MovingItem");
    Vector3 sphereSize = Vector3(10.0f, 10.0f, 10.0f);
    SphereVolume *volume = new SphereVolume(10.0f);
    sphere->SetBoundingVolume((CollisionVolume *) volume);

    sphere->GetTransform()
            .SetScale(sphereSize)
            .SetPosition(position);

    sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), mesh, basicTex, basicShader));
    sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

    sphere->GetRenderObject()->SetColour(Vector4(0, 0.5f, 0.5f, 1));

    sphere->GetPhysicsObject()->SetInverseMass(1.0f);
    sphere->GetPhysicsObject()->InitSphereInertia();

    world->AddGameObject(sphere);

    return sphere;
}

void TutorialGame::InitMazeWorld() {

    if (grid == nullptr) {
        grid = new NavigationGrid("TestGrid3.txt");
    }

    int **gridSquare = grid->GetGrid();
    int size = grid->GetSize();

    for (int y = 0; y < grid->GetHeight(); y++) {
        for (int x = 0; x < grid->GetWidth(); x++) {
            if (gridSquare[y][x] == 120) {
                AddCubeToWorld(Vector3(x * size, -10, y * size), Vector3(size / 2, size / 2, size / 2), 0.0f);
            }
        }
    }

}

void TutorialGame::InitMenu() {

    menu = true;
}

void TutorialGame::InitGamePlayerObject() {

    float meshSize = 4.0f;
    float inverseMass = 0.5f;
    Vector3 position = Vector3(20, -10, 20);

    player = new GamePlayerObject();
    player->SetName("player");

    SphereVolume *volume = new SphereVolume(2.0f);
    player->SetBoundingVolume((CollisionVolume *) volume);

    player->GetTransform()
            .SetScale(Vector3(meshSize, meshSize, meshSize))
            .SetPosition(position);


    player->SetRenderObject(new RenderObject(&player->GetTransform(), charMesh, nullptr, basicShader));
    player->SetPhysicsObject(new PhysicsObject(&player->GetTransform(), player->GetBoundingVolume()));

    player->GetPhysicsObject()->SetInverseMass(inverseMass);
    player->GetPhysicsObject()->InitSphereInertia();

    world->AddGameObject(player);

}

void TutorialGame::InitGameToolsObject() {
  //  AddCoinToWorldWithColor(Vector3(40, -15, 30), 0.5f, 0, "coinTools", Vector4(1, 1, 0, 1));
  //  AddCoinToWorldWithColor(Vector3(40, -15, 30), 0.5f, 0, "coinTools", Vector4(1, 1, 0, 1));

    for(int i = 350;i<=380 ;i+=10)
        for(int j = 70 ;j<=100 ;j+=10)
            AddCoinToWorldWithColor(Vector3(i, -15, j), 0.5f, 0, "coinTools", Vector4(1, 1, 0, 1));

    for(int i = 130;i<=180 ;i+=10)
        for(int j = 10 ;j<=30 ;j+=10)
            AddCoinToWorldWithColor(Vector3(i, -15, j), 0.5f, 0, "coinTools", Vector4(1, 1, 0, 1));

    for(int i = 30;i<=70 ;i+=10)
        for(int j = 110 ;j<=110 ;j+=10)
            AddCoinToWorldWithColor(Vector3(i, -15, j), 0.5f, 0, "coinTools", Vector4(1, 1, 0, 1));

    for(int i = 350;i<=360 ;i+=10)
        for(int j = 260 ;j<=280 ;j+=10)
            AddCoinToWorldWithColor(Vector3(i, -15, j), 0.5f, 0, "coinTools", Vector4(1, 1, 0, 1));

    for(int i = 10;i<=20 ;i+=10)
        for(int j = 270 ;j<=290 ;j+=10)
            AddCoinToWorldWithColor(Vector3(i, -15, j), 0.5f, 0, "coinTools", Vector4(1, 1, 0, 1));

    for(int i = 50;i<=60 ;i+=10)
        for(int j = 140 ;j<=140 ;j+=10)
            AddCoinToWorldWithColor(Vector3(i, -15, j), 0.5f, 0, "coinTools", Vector4(1, 1, 0, 1));

}

void TutorialGame::EndGame() {

    world->GetMainCamera().SetPosition(Vector3(0, 0, 0));

    std::string score = "Score = ";
    score.append(std::to_string(player->score));
    score.append(";");
    Debug::Print(score, Vector2(30, 50));


    std::string itemLeft = "itemLeft = ";
    itemLeft.append(std::to_string(player->itemsLeft));
    itemLeft.append(";");
    Debug::Print(itemLeft, Vector2(30, 60));

    std::string text = "Play Again(F3);";

    Debug::Print(text, Vector2(30, 70));

    text = "Exit(ESC);";
    Debug::Print(text, Vector2(30, 80));

}

GameEnemyObject *TutorialGame::AddGameEnemyObject(Vector3 position) {
    float meshSize = 7.0f;
    float inverseMass = 5.0f;

    GameEnemyObject *character = new GameEnemyObject(grid, player);

    character->SetName("EnemyPlayer");
    AABBVolume *volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
    character->SetBoundingVolume((CollisionVolume *) volume);

    character->GetTransform()
            .SetScale(Vector3(meshSize, meshSize, meshSize))
            .SetPosition(position);

    character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
    character->GetRenderObject()->SetColour(Vector4(0.75, 0, 0, 1));
    character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

    character->GetPhysicsObject()->SetInverseMass(inverseMass);
    character->GetPhysicsObject()->InitSphereInertia();


    world->AddGameObject(character);
    //enemies.emplace_back(character);

    return character;
}

