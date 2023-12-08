#pragma once
#include "Camera.h"
#include "Transform.h"
#include "GameTechRenderer.h"
#include "GamePlayerObject.h"
namespace NCL
{
	namespace CSC8503
	{
		class CWFollowCamera : public Camera
		{
		public:
			CWFollowCamera(GameWorld& gWorld, GamePlayerObject& target);
			~CWFollowCamera() {};

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