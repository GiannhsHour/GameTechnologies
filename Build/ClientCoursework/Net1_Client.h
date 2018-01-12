
#pragma once

#include <ncltech\Scene.h>
#include <ncltech\NetworkBase.h>
#include "MazeRenderer.h"
#include <sstream>

//Basic Network Example

class Net1_Client : public Scene
{
public:
	Net1_Client(const std::string& friendly_name);

	virtual void OnInitializeScene() override;
	virtual void OnCleanupScene() override;
	virtual void OnUpdateScene(float dt) override;
	

	void ProcessNetworkEvent(const ENetEvent& evnt);

protected:
	void SendDataToServer(string data);
	Vector3 ConvertToWorldPos(Vector3 cellpos, int maze_sz, GameObject* obj);
	void CreateGround(int maze_sz,Vector3 halfdims);

	void GenerateMaze(string dt);

	string extractId(enet_uint8* packet) {
		stringstream ss;
		for (int i = 0; i < 4; i++) {
			ss << packet[i];
		}
		string res;
		ss >> res;
		return res;
	}

	string extractData(enet_uint8* packet, int length) {
		stringstream ss;
		for (int i = 4; i < length; i++) {
			ss << packet[i];
		}
		string res;
		getline(ss, res);
		return res.substr(1);
	}

	std::map<string, GameObject*> avatars;
	std::map<string, GameObject*> enemies;

	GameObject* avatar;
	Matrix4 maze_scalar;
	MazeRenderer * maze;

	int maze_size;
	float maze_density;
	int num_enem;
	
	vector<Vector3> path_vec;
	bool draw_path;

	Vector3* start;
	Vector3* end;

	Vector3* grid_position;

	Vector3* curr_avatar_pos;

	NetworkBase network;
	ENetPeer*	serverConnection;
	

};