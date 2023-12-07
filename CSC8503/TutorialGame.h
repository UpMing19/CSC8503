#include "../NCLCoreClasses/KeyboardMouseController.h"

#pragma once

#include "GameTechRenderer.h"

#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif

#include "PhysicsSystem.h"
#include "NavigationGrid.h"
#include "StateGameObject.h"
#include "GamePlayerObject.h"
#include "Vector4.h"
#include "GameEnemyObject.h"
#include "CylinderStateGameObject.h"

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

            GameObject *AddCoinToWorldWithColor(const Vector3 &position, float radius, float inverseMass = 10.0f,
                                                  std::string name = "uname", Vector4 color = Vector4(1, 1, 1, 1));

            GameObject *AddSphereToWorld(const Vector3 &position, float radius, float inverseMass = 10.0f,
                                         std::string name = "uname");

            GameObject *AddCubeToWorld(const Vector3 &position, Vector3 dimensions, float inverseMass = 10.0f,
                                       std::string name = "uname");

            GameObject *AddPlayerToWorld(const Vector3 &position, std::string name = "uname");

            GameObject *AddEnemyToWorld(const Vector3 &position, std::string name = "uname");

            GameObject *AddBonusToWorld(const Vector3 &position, std::string name = "uname");

            StateGameObject *AddStateObjectToWorld(const Vector3 &position);
            CylinderStateGameObject *AddStateObjectToWorld(const Vector3 &position,Mesh* mesh);

            StateGameObject *testStateObject = nullptr;
            CylinderStateGameObject *cylinderStateObject = nullptr;


            void BridgeConstraintTest();

            void InitMazeWorld();

            void InitGamePlayerObject();

            void InitGameToolsObject();

            void EndGame();

            GameEnemyObject *AddGameEnemyObject(Vector3 position);

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
            Vector3 lockedOffset = Vector3(0, 14, 20);

            void LockCameraToObject(GameObject *o) {
                lockedObject = o;
            }

            GameObject *objClosest = nullptr;

            NavigationGrid *grid;

            bool menu;

            float gameCurrentTime = 0;
            float gameTime = 300.0f;
            GamePlayerObject *player = nullptr;


            GameEnemyObject *EnemyObject = nullptr;

        };
    }
}

