#pragma once

#include <ncltech\Scene.h>
#include <ncltech\CommonUtils.h>
#include <ncltech\OcTree.h>

//Fully striped back scene to use as a template for new scenes.
class EmptyScene : public Scene
{
public:
	EmptyScene(const std::string& friendly_name) 
		: Scene(friendly_name)
	{
	}

	virtual ~EmptyScene()
	{
	}

	virtual void OnInitializeScene() override
	{
		Scene::OnInitializeScene();
		OcTree *space = new OcTree(new AABB(Vector3(0, 20, 0), 20));

		//Who doesn't love finding some common ground?
		this->AddGameObject(CommonUtils::BuildCuboidObject(
			"Ground",
			Vector3(0.0f, -1.5f, 0.0f),
			Vector3(20.0f, 1.0f, 20.0f),
			false,
			0.0f,
			false,
			false,
			Vector4(0.2f, 0.5f, 1.0f, 1.0f)));

		this->AddGameObject(CommonUtils::BuildCuboidObject("asdf",
			Vector3(-20.0f, 0.f, 20.0f),	//Position leading to 0.25 meter overlap on faces, and more on diagonals
			Vector3(0.5f, 0.5f, 0.5f),				//Half dimensions
			true,									//Has Physics Object
			0.0f,									//Infinite Mass
			true,									//Has Collision Shape
			true,									//Dragable by the user
			CommonUtils::GenColor(0.45f, 0.5f)));	//Color
	}


	virtual void OnUpdateScene(float dt) override
	{
		Scene::OnUpdateScene(dt);

	}
};