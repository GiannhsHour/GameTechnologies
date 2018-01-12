#pragma once

#include <nclgl\NCLDebug.h>
#include <ncltech\Scene.h>
#include <ncltech\SceneManager.h>
#include <ncltech\PhysicsEngine.h>
#include <ncltech\DistanceConstraint.h>
#include <ncltech\CommonUtils.h>

//Fully striped back scene to use as a template for new scenes.
class MyScene : public Scene
{
public:
	MyScene(const std::string& friendly_name)
		: Scene(friendly_name)
	{
	}

	virtual ~MyScene()
	{
	}


	float m_RampAngle;
	const Vector3 ss_pos = Vector3(-5.5f, 1.5f, -5.0f);
	const Vector3 pool_pos = Vector3(0, 1.5f, 0);

	virtual void OnInitializeScene() override
	{   
		PhysicsEngine::Instance()->SetPaused(true);


		{
			 // create pool
			this->AddGameObject(CommonUtils::BuildCuboidObject("c1",
				pool_pos,	//Position leading to 0.25 meter overlap on faces, and more on diagonals
				Vector3(5.0f, 0.2f, 7.0f),				//Half dimensions
				true,									//Has Physics Object
				0.0f,									//Infinite Mass
				true,									//Has Collision Shape
				false,									//Dragable by the user
				CommonUtils::GenColor(0.3f, 0.5f)));	//Color


			this->AddGameObject(CommonUtils::BuildCuboidObject("c2",
				pool_pos + Vector3(5, 2, 0),									//Position
				Vector3(3.0f, 0.2f, 7.0f),				//Half dimensions
				true,									//Has Physics Object
				0.0f,									//Infinite Mass
				true,									//Has Collision Shape
				false,									//Dragable by the user
				CommonUtils::GenColor(0.5f, 0.5f)));	//Color
			GameObject* c2 = this->FindGameObject("c2");
			c2->Physics()->SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0.0f, 0.0f, 1.0f), 90.0f));


			this->AddGameObject(CommonUtils::BuildCuboidObject("c3",
				pool_pos + Vector3(-5, 2, 0),									//Position
				Vector3(3.0f, 0.2f, 7.0f),				//Half dimensions
				true,									//Has Physics Object
				0.0f,									//Infinite Mass
				true,									//Has Collision Shape
				false,									//Dragable by the user
				CommonUtils::GenColor(0.1f, 0.5f)));	//Color
			GameObject* c3 = this->FindGameObject("c3");
			c3->Physics()->SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0.0f, 0.0f, 1.0f), 90.0f));

			this->AddGameObject(CommonUtils::BuildCuboidObject("c4",
				pool_pos + Vector3(0, 2, -7),								//Position
				Vector3(5.0f, 0.2f, 3.0f),				//Half dimensions
				true,									//Has Physics Object
				0.0f,									//Infinite Mass
				true,									//Has Collision Shape
				false,									//Dragable by the user
				CommonUtils::GenColor(0.7f, 0.5f)));	//Color
			GameObject* c4 = this->FindGameObject("c4");
			c4->Physics()->SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(1.0f, 0.0f, 0.0f), 90.0f));

			this->AddGameObject(CommonUtils::BuildCuboidObject("c5",
				pool_pos + Vector3(0, 2, 7),									//Position
				Vector3(5.0f, 0.2f, 3.0f),				//Half dimensions
				true,									//Has Physics Object
				0.0f,									//Infinite Mass
				true,									//Has Collision Shape
				false,									//Dragable by the user
				CommonUtils::GenColor(0.9f, 0.5f)));	//Color
			GameObject* c5 = this->FindGameObject("c5");
			c5->Physics()->SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(1.0f, 0.0f, 0.0f), 90.0f));
		}

		auto create_ball_cube = [&](const Vector3& offset, const Vector3& scale, float ballsize)
		{
			const int dims = 10;
			 

			for (int x = 0; x < dims/2; ++x)
			{
				for (int y = 0; y < dims/2; ++y)
				{
					for (int z = 0; z < dims; ++z)
					{
						float randomx = rand() % 10 / 10.0f;
						float randomy = rand() % 10 / 10.0f;
						float randomz = rand() % 10 / 10.0f;
						Vector4 col = Vector4(randomx, randomy, randomz, 1.0f);
						Vector3 pos = offset +Vector3(randomx*2, randomy*2, randomz*2) + Vector3(scale.x *x, scale.y * y, scale.z * z);
						GameObject* sphere = CommonUtils::BuildSphereObject(
							"",					// Optional: Name
							pos,				// Position
							ballsize,			// Half-Dimensions
							true,				// Physics Enabled?
							10.f,				// Physical Mass (must have physics enabled)
							true,				// Physically Collidable (has collision shape)
							false,				// Dragable by user?
							col);// Render color
						this->AddGameObject(sphere);
					}
				}
			}
		};
		create_ball_cube(Vector3(-2.0f, 5.5f,-2.5f), Vector3(0.5f, 0.5f, 0.5f), 0.3f);

	

	}

	float m_AccumTime;
	virtual void OnUpdateScene(float dt) override
	{
		Scene::OnUpdateScene(dt);



		//uint drawFlags = PhysicsEngine::Instance()->GetDebugDrawFlags();


		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "--- Controls ---");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "    Hold [1] to rotate objects");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "		OcTree: %s", OcTree::isEnabled() ? "on" : "off" );
	}

};