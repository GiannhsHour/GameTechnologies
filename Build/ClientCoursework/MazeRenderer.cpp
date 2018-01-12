#include "MazeRenderer.h"
#include <ncltech\CommonUtils.h>

const Vector4 wall_color = Vector4(1.f, 0.8f, 0.3f, 1);

MazeRenderer::MazeRenderer(bool* wls, int ms, Mesh* wallmesh) : GameObject("")
, mesh(wallmesh)
, walls(wls)
, maze_size(ms)
, flat_maze(NULL) {
	this->SetRender(new RenderNode());

	if (walls)
	{
		uint num_walls = Generate_FlatMaze();

		wall_descriptors.reserve(num_walls);

		Generate_ConstructWalls();

		Generate_BuildRenderNodes();
	}

}


MazeRenderer::~MazeRenderer()
{
	mesh = NULL;

	if (flat_maze)
	{
		delete[] flat_maze;
		flat_maze = NULL;
	}
}




void MazeRenderer::DrawRoute(vector<Vector3> final, float line_width, int maze_size)
{
	float grid_scalar = 1.0f / (float)maze_size;
	float col_factor = 0.2f / (float)final.size();

	Matrix4 transform = this->Render()->GetWorldTransform();

	float index = 0.0f;
	for (int i = 0; i< final.size()-1 ; i++)
	{
		Vector3 start = transform * Vector3(
			(final[i].x + 0.5f) * grid_scalar,
			0.1f,
			(final[i].y + 0.5f) * grid_scalar);

		Vector3 end = transform * Vector3(
			(final[i+1].x + 0.5f) * grid_scalar,
			0.1f,
			(final[i+1].y + 0.5f) * grid_scalar);

		NCLDebug::DrawThickLine(start + Vector3(0,0.03f,0), end + Vector3(0, 0.03f, 0), line_width, CommonUtils::GenColor(0.8f + index * col_factor));
		index += 1.0f;
	}
}

uint MazeRenderer::Generate_FlatMaze()
{
	//Generates a 3xsize by 3xsize array of booleans, where 
	// a true value corresponds to a solid wall and false to open space.
	// - Each GraphNode is a 2x2 open space with a 1 pixel wall around it.
	uint size = maze_size;
	num_edges = maze_size*(maze_size - 1) * 2;
	GraphEdge* allEdges = new GraphEdge[num_edges];
	for (int i = 0; i < num_edges; i++) {
		allEdges[i]._iswall = walls[i];
	}
	flat_maze_size = size * 3 - 1;

	if (flat_maze) delete[] flat_maze;
	flat_maze = new bool[flat_maze_size * flat_maze_size];
	memset(flat_maze, 0, flat_maze_size * flat_maze_size * sizeof(bool));


	uint base_offset = size * (size - 1);
	uint num_walls = 0;
	//Iterate over each cell in the maze
	for (uint y = 0; y < size; ++y)
	{
		uint y3 = y * 3;
		for (uint x = 0; x < size; ++x)
		{
			int x3 = x * 3;
			
			//Lookup the corresponding edges that occupy that grid cell
			// and if they are walls, set plot their locations on our 2D
			// map.
			//- Yes... it's a horrible branching inner for-loop, my bad! :(
			if (x < size - 1)
			{
				GraphEdge& edgeX = allEdges[(y * (size - 1) + x)];
				if (edgeX._iswall)
				{
					flat_maze[(y * 3) * flat_maze_size + (x * 3 + 2)] = true;
					flat_maze[(y * 3 + 1) * flat_maze_size + (x * 3 + 2)] = true;
					num_walls += 2;
				}
			}
			if (y < size - 1)
			{
				GraphEdge& edgeY = allEdges[base_offset + (x * (size - 1) + y)];
				if (edgeY._iswall)
				{
					flat_maze[(y * 3 + 2) * flat_maze_size + (x * 3)]	= true;
					flat_maze[(y * 3 + 2) * flat_maze_size + (x * 3 + 1)] = true;
					num_walls += 2;
				}
			}

			//As it's now a 3x3 cell for each, and the doorways are 2x1 or 1x2
			// we need to add an extra wall for the diagonals.
			if (x < size - 1 && y < size - 1)
			{
				flat_maze[(y3 + 2) * flat_maze_size + x3 + 2] = true;
				num_walls++;
			}
		}
	}

	return num_walls;
}

void MazeRenderer::Generate_ConstructWalls()
{
//First try and compact adjacent walls down, so we don't
// just end up creating lots of little cube's.

	//Horizontal wall pass
	for (uint y = 0; y < flat_maze_size; ++y)
	{
		for (uint x = 0; x < flat_maze_size; ++x)
		{
			//Is the current cell a wall?
			if (flat_maze[y*flat_maze_size + x])
			{
				WallDescriptor w(x, y);

				uint old_x = x;

				//If we found a wall, keep iterating in the current
				// search direction and see if we can join it with
				// adjacent walls.
				for (++x; x < flat_maze_size; ++x)
				{
					if (!flat_maze[y * flat_maze_size + x])
						break;

					flat_maze[y * flat_maze_size + x] = false;
				}

				w._xe = x;

				//If the wall is only 1x1, ignore it for the vertical-pass.
				if (w._xe - w._xs > 1)
				{
					flat_maze[y * flat_maze_size + old_x] = false;
					wall_descriptors.push_back(w);
				}
			}
		}
	}

	//Vertical wall pass
	for (uint x = 0; x < flat_maze_size; ++x)
	{
		for (uint y = 0; y < flat_maze_size; ++y)
		{
			if (flat_maze[y * flat_maze_size + x])
			{
				WallDescriptor w(x, y);

				for (++y; y < flat_maze_size && flat_maze[y * flat_maze_size + x]; ++y) {}

				w._ye = y;
				wall_descriptors.push_back(w);
			}
		}
	}



}

void MazeRenderer::Generate_BuildRenderNodes()
{
	//Turn compacted walls into RenderNodes
	RenderNode *cube, *root = Render();

//Turn walls into 3D Cuboids
	const float scalar = 1.f / (float)flat_maze_size;
	halfD = Vector3(scalar * 2, 1.0f, scalar * 2);
	for (const WallDescriptor& w : wall_descriptors)
	{
		Vector3 start = Vector3(
			float(w._xs),
			0.0f,
			float(w._ys));

		Vector3 size = Vector3(
			float(w._xe - w._xs),
			0.0f,
			float(w._ye - w._ys)
		);


		start = start * scalar;
		Vector3 end = start + size * scalar;
		end.y = 0.75f;

		Vector3 centre = (end + start) * 0.5f;
		Vector3 halfDims = centre - start;
		cube = new RenderNode(mesh, wall_color);
		cube->SetTransform(Matrix4::Translation(centre) * Matrix4::Scale(halfDims));
		root->AddChild(cube);

	}

//Add bounding edge walls to the maze
	cube = new RenderNode(mesh, wall_color);
	cube->SetTransform(Matrix4::Translation(Vector3(-scalar*0.5f, 0.25f, 0.5)) * Matrix4::Scale(Vector3(scalar*0.5f, 0.25f, scalar + 0.5f)));
	root->AddChild(cube);

	cube = new RenderNode(mesh, wall_color);
	cube->SetTransform(Matrix4::Translation(Vector3(1.f + scalar*0.5f, 0.25f, 0.5)) * Matrix4::Scale(Vector3(scalar*0.5f, 0.25f, scalar + 0.5f)));
	root->AddChild(cube);

	cube = new RenderNode(mesh, wall_color);
	cube->SetTransform(Matrix4::Translation(Vector3(0.5, 0.25f, -scalar*0.5f)) * Matrix4::Scale(Vector3(0.5f, 0.25f, scalar*0.5f)));
	root->AddChild(cube);

	cube = new RenderNode(mesh, wall_color);
	cube->SetTransform(Matrix4::Translation(Vector3(0.5, 0.25f, 1.f + scalar*0.5f)) * Matrix4::Scale(Vector3(0.5f, 0.25f, scalar*0.5f)));
	root->AddChild(cube);


//Finally - our start/end goals
	/*GraphNode* start = maze->GetStartNode();
	GraphNode* end = maze->GetGoalNode();

	Vector3 cellpos = Vector3(
		start->_pos.x * 3,
		0.0f,
		start->_pos.y * 3
	) * scalar;
	Vector3 cellsize = Vector3(
		scalar * 2,
		1.0f,
		scalar * 2
	);

	cube = new RenderNode(mesh, Vector4(0.0f, 1.0f, 0.0f, 1.0f));
	cube->SetTransform(Matrix4::Translation(cellpos + cellsize * 0.5f) * Matrix4::Scale(cellsize * 0.5f));
	root->AddChild(cube);

	cellpos = Vector3(
		end->_pos.x * 3,
		0.0f,
		end->_pos.y * 3
	) * scalar;
	cube = new RenderNode(mesh, Vector4(1.0f, 0.0f, 0.0f, 1.0f));
	cube->SetTransform(Matrix4::Translation(cellpos + cellsize * 0.5f) * Matrix4::Scale(cellsize * 0.5f));
	root->AddChild(cube);*/

	this->SetRender(root);
}

void MazeRenderer::DrawStartEndNodes(Vector3* start, Vector3* end) {
	//Finally - our start/end goals
	
	RenderNode *cube, *root = Render();
	for (int i = 0; i < nodes.size(); i++) {
		root->RemoveChild(nodes[i]);
	}
	nodes.clear();

	const float scalar = 1.f / (float)flat_maze_size;
	Vector3 cellpos = Vector3(start->x * 3, 0.0f, start->y * 3) * scalar;
	Vector3 cellsize = Vector3(scalar * 2, 1.0f, scalar * 2);

	cube = new RenderNode(mesh, Vector4(0.0f, 1.0f, 0.0f, 1.0f));
	cube->SetTransform(Matrix4::Translation(cellpos + cellsize * 0.5f) * Matrix4::Scale(cellsize * 0.5f));
	root->AddChild(cube);
	nodes.push_back(cube);
	startNode = cube;


	cellpos = Vector3(end->x * 3, 0.0f, end->y * 3) * scalar;
	cube = new RenderNode(mesh, Vector4(1.0f, 0.0f, 0.0f, 1.0f));
	cube->SetTransform(Matrix4::Translation(cellpos + cellsize * 0.5f) * Matrix4::Scale(cellsize * 0.5f));
	root->AddChild(cube);
	nodes.push_back(cube);
}