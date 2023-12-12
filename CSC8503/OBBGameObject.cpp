
#include "OBBGameObject.h"
#include "TutorialGame.h"
#include "PhysicsObject.h"
#include "RenderObject.h"

NCL::CSC8503::OBBGameObject::OBBGameObject(TutorialGame& gGame, Vector3 position, Vector3 size, float jumpForce, Vector4 color, Mesh* mesh, Texture* texture, Shader* shader) : goatGame(gGame)
{
    this->jumpDir = Vector3(0,1,0);
    this->jumpDir = transform.Up();
    this->jumpForce = jumpForce;

    OBBVolume* volume = new OBBVolume(size);
    SetBoundingVolume((CollisionVolume*)volume);

    transform
            .SetPosition(position)
            .SetScale(size * 2);

    renderObject = new RenderObject(&transform, mesh, texture, shader);
    renderObject->SetColour(color);

    physicsObject = new PhysicsObject(&transform, boundingVolume);
    physicsObject->SetInverseMass(0);
    physicsObject->InitCubeInertia();
}

void NCL::CSC8503::OBBGameObject::OnCollisionBegin(GameObject* otherObject)
{
    if (otherObject == goatGame.player)
        otherObject->GetPhysicsObject()->ApplyLinearImpulse(jumpDir * jumpForce);
}
