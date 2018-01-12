#pragma once
#include <enet\enet.h>
#include <nclgl\GameTimer.h>
#include <nclgl\Vector3.h>
#include <nclgl\common.h>
#include <ncltech\PhysicsNode.h>
#include <ncltech\NetworkBase.h>
#include "MazeGenerator.h"
#include "SearchAStar.h"
#include "EvilBall.h"
#include <sstream>


struct Player {
	std::list<const GraphNode*> final_path;
	std::vector<Vector3> avatar_velocities;
	std::vector<Vector3> avatar_cellpos;
	Vector3 cur_cell_pos;
	int avatarIndex = 0;
	PhysicsNode* avatar;
	bool enable_avatar = false;
	string peer_id;
};