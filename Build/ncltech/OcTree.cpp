#include "OcTree.h"


std::vector<OcTree*> OcTree::leaves = std::vector<OcTree*>();
bool OcTree::isActive = false;
int OcTree::capacity = 5;
bool OcTree::insert(PhysicsNode* p) {

	// Ignore objects that do not belong in this OcTree
	if (!boundary->containsObject(p))
		return false; // object cannot be added

	// If there is space in this OcTree, add the object here
	if (objectsIn.size() < capacity && isLeaf())
	{
		objectsIn.push_back(p);
		return true;
	}

	if (isLeaf()) {
		this->subdivide();
		for (int i = 0; i < objectsIn.size(); i++) {
			for (int j = 0; j < 8; j++) {
				children[j]->insert(objectsIn[i]);
			}
		}
	}


	for (int i = 0; i < 8; i++) {
		(children[i]->insert(p));
	}

	return false;
}


void OcTree::subdivide() {
	Vector3 cur_center = boundary->center;
	float half_cur_dim = boundary->halfdim / 2.0f;
	children.push_back(new OcTree(new AABB(Vector3(cur_center.x - half_cur_dim, cur_center.y - half_cur_dim, cur_center.z - half_cur_dim), half_cur_dim))); //topLeft ( down)
	children.push_back(new OcTree(new AABB(Vector3(cur_center.x + half_cur_dim, cur_center.y - half_cur_dim, cur_center.z - half_cur_dim), half_cur_dim))); //topRight (down)
	children.push_back(new OcTree(new AABB(Vector3(cur_center.x - half_cur_dim, cur_center.y - half_cur_dim, cur_center.z + half_cur_dim), half_cur_dim))); //botLeft (down)
	children.push_back(new OcTree(new AABB(Vector3(cur_center.x + half_cur_dim, cur_center.y - half_cur_dim, cur_center.z + half_cur_dim), half_cur_dim))); //botRight (down)
	children.push_back(new OcTree(new AABB(Vector3(cur_center.x - half_cur_dim, cur_center.y + half_cur_dim, cur_center.z - half_cur_dim), half_cur_dim))); //topLeft (up)
	children.push_back(new OcTree(new AABB(Vector3(cur_center.x + half_cur_dim, cur_center.y + half_cur_dim, cur_center.z - half_cur_dim), half_cur_dim))); //topRight (up)
	children.push_back(new OcTree(new AABB(Vector3(cur_center.x - half_cur_dim, cur_center.y + half_cur_dim, cur_center.z + half_cur_dim), half_cur_dim))); //botLeft (up)
	children.push_back(new OcTree(new AABB(Vector3(cur_center.x + half_cur_dim, cur_center.y + half_cur_dim, cur_center.z + half_cur_dim), half_cur_dim))); //botRight (up)
}

void OcTree::populateLeaves(OcTree* root) {
	if (root->isLeaf()) {
			leaves.push_back(root);
	}
	else {
		for (int i = 0; i < 8; i++) {
			populateLeaves(root->children[i]);
		}
	}
}

void OcTree::deleteTree(OcTree* root) {
	if (root) {
		for (int i = 0; i < root->children.size(); i++) {
			deleteTree(root->children[i]);
		}
		delete root;
	}
}

void OcTree::draw(OcTree* root) {
	if (root) {
		for (int i = 0; i < root->children.size(); i++) {
			draw(root->children[i]);
		}
		float d = 2 * root->boundary->halfdim;
		Vector3 c1 = root->boundary->corner1;
		Vector3 c2 = c1 + Vector3(-d, 0, 0);
		Vector3 c3 = c2 + Vector3(0, 0, d);
		Vector3 c4 = c3 + Vector3(d, 0, 0);

		Vector3 c5 = root->boundary->corner2;
		Vector3 c6 = c5 + Vector3(d, 0, 0);
		Vector3 c7 = c6 + Vector3(0, 0, -d);
		Vector3 c8 = c7 + Vector3(-d, 0, 0);
		
		NCLDebug::DrawThickLine(c1, c2, 0.1f, Vector4(0.0f, 1.0f, 0.0f, 1.0f));
		NCLDebug::DrawThickLine(c2, c3, 0.1f, Vector4(0.0f, 1.0f, 0.0f, 1.0f));
		NCLDebug::DrawThickLine(c3, c4, 0.1f, Vector4(0.0f, 1.0f, 0.0f, 1.0f));
		NCLDebug::DrawThickLine(c1, c4, 0.1f, Vector4(0.0f, 1.0f, 0.0f, 1.0f));

		NCLDebug::DrawThickLine(c5, c6, 0.1f, Vector4(0.0f, 1.0f, 0.0f, 1.0f));
		NCLDebug::DrawThickLine(c6, c7, 0.1f, Vector4(0.0f, 1.0f, 0.0f, 1.0f));
		NCLDebug::DrawThickLine(c7, c8, 0.1f, Vector4(0.0f, 1.0f, 0.0f, 1.0f));
		NCLDebug::DrawThickLine(c5, c8, 0.1f, Vector4(0.0f, 1.0f, 0.0f, 1.0f));

		NCLDebug::DrawThickLine(c1, c7, 0.1f, Vector4(0.0f, 1.0f, 0.0f, 1.0f));
		NCLDebug::DrawThickLine(c4, c6, 0.1f, Vector4(0.0f, 1.0f, 0.0f, 1.0f));
		NCLDebug::DrawThickLine(c2, c8, 0.1f, Vector4(0.0f, 1.0f, 0.0f, 1.0f));
		NCLDebug::DrawThickLine(c3, c5, 0.1f, Vector4(0.0f, 1.0f, 0.0f, 1.0f));
	}
}