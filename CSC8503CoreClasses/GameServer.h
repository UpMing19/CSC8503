#pragma once
#include "NetworkBase.h"


namespace NCL {
	namespace CSC8503 {
		class GameWorld;
        class TutorialGame;
		class GameServer : public NetworkBase {
		public:
			GameServer(int onPort, int maxClients);
			~GameServer();

			bool Initialise();
			void Shutdown();

			void SetGameWorld(GameWorld &g);
            void SetWorld(TutorialGame *g){world = g;};
			bool SendGlobalPacket(int msgID);
			bool SendGlobalPacket(GamePacket& packet);

			virtual void UpdateServer();
            GameWorld * GetGameWorld(){return gameWorld;};
            TutorialGame * GetWorld(){return world;};
		protected:
			int			port;
			int			clientMax;
			int			clientCount;
			GameWorld*	gameWorld;
            TutorialGame *world;
			int incomingDataRate;
			int outgoingDataRate;
		};
	}
}
