#include <ncltech\Scene.h>
#include <nclgl\Vector4.h>
#include <ncltech\GraphicsPipeline.h>
#include <ncltech\PhysicsEngine.h>
#include <ncltech\DistanceConstraint.h>
#include <ncltech\SceneManager.h>
#include <ncltech\CommonUtils.h>

#include "RenderNodeParticles.h"
#include "CudaCollidingParticles.cuh"

using namespace CommonUtils;

class Scene_CollisionHandling : public Scene
{
public:
	Scene_CollisionHandling(const std::string& friendly_name)
		: Scene(friendly_name)
	{
	}

	virtual ~Scene_CollisionHandling()
	{
		delete projectile;
	}


	virtual void OnInitializeScene() override
	{
		rdy = false;
		//<--- SCENE CREATION --->
		//Create Ground
		this->AddGameObject(BuildCuboidObject("Ground", Vector3(0.0f, -1.0f, 0.0f), Vector3(15.0f, 1.0f, 15.0f), true, 0.0f, true, false, Vector4(0.0f, 1.0f, 0.3f, 1.0f)));

		cudaParticleProg = new CudaCollidingParticles();

		//The dam size (<value> * PARTICLE_RADIUS * 2) must be smaller than the simulation world size!
		cudaParticleProg->InitializeParticleDam(40, 20, 40);

		uint num_particles = cudaParticleProg->GetNumParticles();

		RenderNodeParticles* rnode = new RenderNodeParticles();
		rnode->SetParticleRadius(PARTICLE_RADIUS);
		rnode->SetColor(Vector4(1.f, 0.f, 0.0f, 1.f));
		rnode->GeneratePositionBuffer(num_particles, NULL);

		const float half_grid_world_size = PARTICLE_GRID_SIZE * PARTICLE_GRID_CELL_SIZE * 0.5f;
		rnode->SetTransform(Matrix4::Translation(Vector3(-half_grid_world_size, -half_grid_world_size, -half_grid_world_size)));

		//We don't need any game logic, or model matrices, just a means to render our
		// particles to the screen.. so this is just a wrapper  to our actual
		// vertex buffer that holds each particles world position.
		this->AddGameObject(new GameObject("", rnode, NULL));

		cudaParticleProg->InitializeOpenGLVertexBuffer(rnode->GetGLVertexBuffer());
	}

	void spawn() {
		{ 
			rdy = true;
			//Create a projectile
			Camera * camera = GraphicsPipeline::Instance()->GetCamera();
			float yaw = camera->GetYaw();
			float pitch = camera->GetPitch();
			float fx, fz, fy;
			fx = -sin(DegToRad(yaw));
			fz = -cos(DegToRad(yaw));

			if (projectile) {
				this->RemoveGameObject(projectile);
			}
			projectile = CommonUtils::BuildSphereObject("spawned_sphere",
				GraphicsPipeline::Instance()->GetCamera()->GetPosition(),
				0.5f,									//Radius
				true,									//Has Physics Object
				1.0f / 6.0f,							//Inverse Mass
				true,									//Has Collision Shape
				true,									//Dragable by the user
				Vector4(0, 1, 0, 1));		//Color

			projectile->Physics()->SetLinearVelocity((Vector3(fx * 16, 5 + pitch * 0.6, fz * 16)));
			projectile->Physics()->SetElasticity(0.5f);
			this->AddGameObject(projectile);
			
		}
	}

	virtual void OnCleanupScene() override
	{
		Scene::OnCleanupScene();
		delete cudaParticleProg;
	}

	virtual void OnUpdateScene(float dt) override
	{
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "--- Info ---");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "  Example broadphase using cuda thrust library. The thrust library");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "  is the GPU equivalent of the C++ STL and makes things easier ");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "  with vector's, sorting, iterators and array manipulation.");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "  No. Particles: %d", cudaParticleProg->GetNumParticles());

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_J))
		{    
			spawn();
		}

		if (rdy) {
			Vector3* v = new Vector3(projectile->Physics()->GetLinearVelocity());
			cudaParticleProg->UpdateParticles(dt, projectile->Physics()->GetPosition(),v , projectile->Physics()->GetInverseMass(), projectile->Physics()->GetColRadius());
			projectile->Physics()->SetLinearVelocity(*v);
		}
		
	
	}


protected:
	CudaCollidingParticles* cudaParticleProg;
	GameObject* projectile;
	bool rdy;
};