#pragma once
#include <nclgl\Vector3.h>
#include <ncltech\CollisionShape.h>
#include <ncltech\PhysicsNode.h>

struct AABB {
	Vector3 center;
	float halfdim;
	Vector3 corner1;
	Vector3 corner2;

	AABB() {
		center = Vector3(0.0f, 0.0f, 0.0f);
		halfdim = 0.5f;
		corner1 = center + Vector3(halfdim, halfdim, -halfdim);
		corner2 = center - Vector3(halfdim, halfdim, -halfdim);
	}

	AABB(Vector3 cnt, float hlf) {
		center = cnt;
		halfdim = hlf;
		corner1 = center + Vector3(halfdim, halfdim, -halfdim);
		corner2 = center - Vector3(halfdim, halfdim, -halfdim);
	}
	
    //Based on https://stackoverflow.com/questions/4578967/cube-sphere-intersection-test

	inline float squared(float v) { return v * v; }

	bool containsObject(PhysicsNode* obj) {
		Vector3 obj_pos = obj->GetPosition();
		float d_squared = obj->GetColRadius() * obj->GetColRadius();

		if (obj_pos.x > corner1.x) d_squared -= squared(obj_pos.x - corner1.x);
		else if (obj_pos.x < corner2.x) d_squared -= squared(obj_pos.x - corner2.x);

		if (obj_pos.y > corner1.y) d_squared -= squared(obj_pos.y - corner1.y);
		else if (obj_pos.y < corner2.y) d_squared -= squared(obj_pos.y - corner2.y);

		if (obj_pos.z < corner1.z) d_squared -= squared(obj_pos.z - corner1.z);
		else if (obj_pos.z > corner2.z) d_squared -= squared(obj_pos.z - corner2.z);

		return d_squared > 0;
	}
};