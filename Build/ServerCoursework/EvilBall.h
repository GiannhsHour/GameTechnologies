
#pragma once
#include <ncltech\GameObject.h>
#include <functional>
#include <algorithm>
#include "MazeGenerator.h"
#include "SearchAStar.h"
#include "MazePlayer.h"



class EvilBall : public GameObject
{
public:
	EvilBall(MazeGenerator* gen, string identity);
	~EvilBall();

	std::string GetStateName() { return state_name; }

	void SetMazePos(Vector3 pos) {avatar->SetPosition(pos); }
	Vector3 GetMazePos() { return avatar->GetPosition(); }

	void SetId(string identity) { id = identity; }
	string GetId() { return id; }

	string GetPositionAsString() { return position; }

	void EnableBall() { enable_avatar = true; }

	void GeneratePath(bool random = true, const Vector3 player = Vector3(0,0,0));
	void UpdatePosition();

	bool isAggresive() { return aggressive; }

	bool isInLineOfSight(Vector3 player);

	static void ToggleChase() { radiusChase = !radiusChase; }

	//States
	void Patrol(vector<Player*>& player);
	void Chase();

protected:
	//Global
	std::list<const GraphNode*> patrol_path;
	GraphNode* start;
	GraphNode* end;

	std::string state_name;
	string position;
	string id;
	


	MazeGenerator* generator;

	Vector3 maze_pos;
	Vector3 target_pos;

	vector<Vector3> avatar_velocities;
	vector<Vector3> avatar_cellpos;
	int avatarIndex;
	PhysicsNode* avatar;
	bool enable_avatar;
	string peer_id;

	//player being chased
	Player* chasing_player;
	
	bool reached_end;
	bool first_path_after_chase;
	bool generate;
	
	float ac_time;

	//chasing or patrolling
	bool aggressive;

	static bool radiusChase;
};