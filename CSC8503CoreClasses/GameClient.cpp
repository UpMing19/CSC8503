#include "GameClient.h"
#include "./enet/enet.h"

using namespace NCL;
using namespace CSC8503;

GameClient::GameClient() {
    netHandle = enet_host_create(nullptr, 1, 1, 0, 0);
}

GameClient::~GameClient() {
    enet_host_destroy(netHandle);
}

bool GameClient::Connect(uint8_t a, uint8_t b, uint8_t c, uint8_t d, int portNum) {
    ENetAddress address;
    address.port = portNum;
    address.host = (d << 24) | (c << 16) | (b << 8) | (a);

    netPeer = enet_host_connect(netHandle, &address, 2, 0);

    return netPeer != nullptr;

}

void GameClient::UpdateClient() {
}

void GameClient::SendPacket(GamePacket &payload) {
    ENetPacket *dataPacket = enet_packet_create(&payload,payload.GetTotalSize(), 0);
    enet_peer_send(netPeer, 0, dataPacket);

}
