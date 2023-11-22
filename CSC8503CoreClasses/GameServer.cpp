#include "GameServer.h"
#include "GameWorld.h"
#include "./enet/enet.h"
using namespace NCL;
using namespace CSC8503;

GameServer::GameServer(int onPort, int maxClients)	{
	port		= onPort;
	clientMax	= maxClients;
	clientCount = 0;
	netHandle	= nullptr;
	Initialise();
}

GameServer::~GameServer()	{
	Shutdown();
}

void GameServer::Shutdown() {
	SendGlobalPacket(BasicNetworkMessages::Shutdown);
	enet_host_destroy(netHandle);
	netHandle = nullptr;
}

bool GameServer::Initialise() {
	return false;
}

bool GameServer::SendGlobalPacket(int msgID) {
	return false;
}

bool GameServer::SendGlobalPacket(GamePacket& packet) {
	return false;
}

void GameServer::UpdateServer() {
}

void GameServer::SetGameWorld(GameWorld &g) {
	gameWorld = &g;
}