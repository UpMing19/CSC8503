#pragma once
#include "Camera.h"
#include "Transform.h"
#include "GameTechRenderer.h"
#include "GamePlayerObject.h"
namespace NCL
{
	namespace CSC8503
	{
		class GamePlayerFollowCamera : public Camera
		{
		public:
			GamePlayerFollowCamera(GameWorld& gWorld, GamePlayerObject& target);
			~GamePlayerFollowCamera() {};

			virtual void UpdateCamera(float dt) override;

		protected:
			GameWorld& world;

            GamePlayerObject& followTarget;
			Vector3 startOffset;
			Vector3 followOffset = Vector3(0, 20.0f, 30.0f);

			float pitchOffset = 12.0f;

		private:
			float requiredRayDistance;
			float h = 0.0f, v = 0.0f;
		};
	}
}