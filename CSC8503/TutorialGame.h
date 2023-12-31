#include "../NCLCoreClasses/KeyboardMouseController.h"

#pragma once

#include "GameTechRenderer.h"
#include "GameGooseObject.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "../CSC8503CoreClasses/GameServer.h"
#include "GamePlayerFollowCamera.h"
#include "PhysicsSystem.h"
#include "NavigationGrid.h"
#include "StateGameObject.h"
#include "GamePlayerObject.h"
#include "OBBGameObject.h"
#include "Vector4.h"
#include "GameEnemyObject.h"
#include "CylinderStateGameObject.h"
#include "../CSC8503CoreClasses/GameClient.h"
#include "../CSC8503CoreClasses/PushdownMachine.h"
#include "../CSC8503CoreClasses/PushdownState.h"
namespace NCL {
    namespace CSC8503 {
        class TutorialGame {
        public:
            TutorialGame();

            ~TutorialGame();

            virtual void UpdateGame(float dt);

            void InitMenu();

            void InitWorld();

            void InitCamera();

            GameWorld * GetGameWorld(){return world;};

            void AddClientObject(GameObject *gameObject);

            GameServer *server;
            GameClient *client;
            GamePlayerObject *player = nullptr;
            bool menu;
            void AddOBBGameObject(const Vector3& padPos, const Vector3& padSize, const Vector3& padRotation, const float& padForce, const Vector4& padColor);
        protected:
            void InitialiseAssets();

            void UpdateKeys();

            /*
            These are some of the world/object creation functions I created when testing the functionality
            in the module. Feel free to mess around with them to see different objects being created in different
            test scenarios (constraints, collision types, and so on).
            */
            void InitGameExamples();

            void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);

            void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);

            void
            InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3 &cubeDims);

            void InitDefaultFloor();

            bool SelectObject();

            void MoveSelectedObject();

            void DebugObjectMovement();

            void LockedObjectMovement();

            GameObject *AddFloorToWorld(const Vector3 &position, std::string name = "uname");
            GameObject* AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass = 10.0f);

            GameObject *AddCoinToWorldWithColor(const Vector3 &position, float radius, float inverseMass = 10.0f,
                                                std::string name = "uname", Vector4 color = Vector4(1, 1, 1, 1));

            GameObject *AddSphereToWorld(const Vector3 &position, float radius, float inverseMass = 10.0f,
                                         std::string name = "uname");

            GameObject *AddCubeToWorld(const Vector3 &position, Vector3 dimensions, float inverseMass = 10.0f,
                                       std::string name = "uname");
            GameObject *AddEndPointToWorld(const Vector3 &position, Vector3 dimensions, float inverseMass = 0.0f,
                                       std::string name = "endPoint");
            GameObject *AddPlayerToWorld(const Vector3 &position, std::string name = "uname");

            GameObject *AddEnemyToWorld(const Vector3 &position, std::string name = "uname");

            GameObject *AddBonusToWorld(const Vector3 &position, std::string name = "uname");

            StateGameObject *AddStateObjectToWorld(const Vector3 &position);

            CylinderStateGameObject *AddStateObjectToWorld(const Vector3 &position, Mesh *mesh);

            StateGameObject *testStateObject = nullptr;
            CylinderStateGameObject *cylinderStateObject = nullptr;


            void BridgeConstraintTest();

            void InitMazeWorld();

            void InitGamePlayerObject();


            void InitGameToolsObject();

            void EndGame();

            GameGooseObject *AddGameGooseObject(Vector3 position);
            GameEnemyObject *AddGameEnemyObject(Vector3 position);
            GameObject *AddGameDoorObject(Vector3 position, Vector3 dimensions, float inverseMass = 10.0f,std::string name = "uname");

#ifdef USEVULKAN
            GameTechVulkanRenderer*	renderer;
#else
            GameTechRenderer *renderer;
#endif
            PhysicsSystem *physics;
            GameWorld *world;

            KeyboardMouseController controller;

            bool useGravity;
            bool inSelectionMode;

            float forceMagnitude;

            GameObject *selectionObject = nullptr;

            Mesh *capsuleMesh = nullptr;
            Mesh *cubeMesh = nullptr;
            Mesh *sphereMesh = nullptr;
            Mesh *coinMesh = nullptr;
            Mesh *highrise = nullptr;


            Texture *basicTex = nullptr;
            Shader *basicShader = nullptr;

            //Coursework Meshes
            Mesh *charMesh = nullptr;
            Mesh *enemyMesh = nullptr;
            Mesh *bonusMesh = nullptr;
            Mesh *cylinderMesh = nullptr;
            Mesh *gooseMesh = nullptr;

            //Coursework Additional functionality
            GameObject *lockedObject = nullptr;
            Vector3 lockedOffset = Vector3(0, 20, 30);

            void LockCameraToObject(GameObject *o) {
                lockedObject = o;
            }

            GameObject *objClosest = nullptr;

            NavigationGrid *grid;



            float gameCurrentTime = 0;
            float gameTime = 300.0f;



            GameEnemyObject *EnemyObject = nullptr;
            GameGooseObject *GooseObject = nullptr;
            GameObject *KeyObject = nullptr;

            Camera *cameraMain;
            GamePlayerFollowCamera *cameraFollow;
            Vector3 startCameraPos = Vector3(512.0f, 40.0f, 512.0f);
        };
    }
}

