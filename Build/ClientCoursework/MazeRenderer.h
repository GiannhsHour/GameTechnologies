#pragma once

#include <ncltech\GameObject.h>
#include <ncltech\CommonMeshes.h>
#include "SearchAlgorithm.h"


struct WallDescriptor
{
	uint _xs, _xe;
	uint _ys, _ye;

	WallDescriptor(uint x, uint y) : _xs(x), _xe(x + 1), _ys(y), _ye(y + 1) {}
};

typedef std::vector<WallDescriptor> WallDescriptorVec;


class MazeRenderer : public GameObject
{
public:
	MazeRenderer(bool* walls,  int ms, Mesh* wallmesh = CommonMeshes::Cube());
	virtual ~MazeRenderer();

	//The search history draws from edges because they already store the 'to'
	// and 'from' of GraphNodes.
	void DrawSearchHistory(const SearchHistory& history, float line_width);
	void DrawRoute(vector<Vector3>, float line_width , int maze_size);
	void DrawStartEndNodes(Vector3* start, Vector3* end);
	Vector3 GetHalfDims() { return halfD; }


protected:
	//Turn MazeGenerator data into flat 2D map (3 size x 3 size) of boolean's
	// - True for wall
	// - False for empty
	//Returns uint: Guess at the number of walls required
	uint Generate_FlatMaze(); 

	//Construct a list of WallDescriptors from the flat 2D map generated above.
	void Generate_ConstructWalls();

	//Finally turns the wall descriptors into actual renderable walls ready
	// to be seen on screen.
	void Generate_BuildRenderNodes();

protected:

	bool* walls;
	int num_edges;
	int maze_size;

	Mesh*			mesh;

	bool*	flat_maze;	//[flat_maze_size x flat_maze_size]
	uint	flat_maze_size;

	Vector3 halfD;

	vector<RenderNode*> nodes;

	vector<GameObject*> ground_nodes;

	RenderNode* startNode;

	WallDescriptorVec	wall_descriptors;
};