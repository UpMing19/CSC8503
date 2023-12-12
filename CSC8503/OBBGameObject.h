
#pragma once

#include "GameObject.h"
#include "GameTechRenderer.h"

namespace NCL
{
    namespace CSC8503
    {
        class TutorialGame;
        class OBBGameObject : public GameObject
        {
        public:
            OBBGameObject(TutorialGame& gGame, Vector3 position, Vector3 size, float jumpForce, Vector4 color, Mesh* mesh, Texture* texture, Shader* shader);

            void OnCollisionBegin(GameObject* otherObject) override;

        protected:
            TutorialGame& goatGame;

            Vector3 jumpDir;
            float jumpForce;
        };
    }
}