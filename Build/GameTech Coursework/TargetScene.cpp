#include "TargetScene.h"

#include <nclgl\Vector4.h>
#include <ncltech\GraphicsPipeline.h>
#include <ncltech\PhysicsEngine.h>
#include <ncltech\DistanceConstraint.h>
#include <ncltech\SceneManager.h>
#include <ncltech\CommonUtils.h>
using namespace CommonUtils;
std::map<PhysicsNode*, bool> TargetScene::col_check_map;
void TargetScene::OnInitializeScene() 
{   
	scores.clear();
	scoresTime.clear();
	acum_time = 0;

	const Vector3 floor_pos = Vector3(0, 1.5f, 0);
	
	PhysicsEngine::Instance()->SetDampingFactor(1.0f);		

	for (int i = 0; i < objects.size(); i++) {
		objects[i].clear();
	}
	objects.clear();

	ballTex = SOIL_load_OGL_texture(TEXTUREDIR"basketball.PNG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_COMPRESS_TO_DXT);
	courtTex = SOIL_load_OGL_texture(TEXTUREDIR"wood.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_COMPRESS_TO_DXT);
	ironTex = SOIL_load_OGL_texture(TEXTUREDIR"iron.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_COMPRESS_TO_DXT);
	// texture from http://naveensabesan.blogspot.co.uk/2012/08/cosco.html
	boardTex = SOIL_load_OGL_texture(TEXTUREDIR"board.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_COMPRESS_TO_DXT);

	//Create basket net.
	const int dims = 30;
	const int dimsx = dims;
	const int dimsz = dims  * 0.25f;
	float parts = 360.0f / (float)dimsx ;
	auto create_ball_cloth = [&](const Vector3& offset, const Vector3& scale, float ballsize)
	{

		float m;
		for (int x = 0; x < dimsx; ++x)
		{
			float rad = DegToRad(x * parts);
			objects.push_back(std::vector<GameObject*>());
			for (int z = 0; z < dimsz; ++z)
			{	
				
				if (z == 0)  m = 0.0f;
				else m = 150.0f;
				Vector4 col = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
				//	Vector3 pos = offset + Vector3(scale.x * x * 0.5, 0.0f, scale.z * z * 0.5);
				Vector3 pos = offset + Vector3(cos(rad)*(1-(float)z/ (float)(dimsz + 6)), -scale.y * z * 0.6, sin(rad)*(1 - (float)z / (float)(dimsz + 6)));
				GameObject* sphere = CommonUtils::BuildNonRenderObject(
					"",					// Optional: Name
					pos,				// Position
					ballsize,			// Half-Dimensions
					true,				// Physics Enabled?
					m,				// Physical Mass (must have physics enabled)
					true,				// Physically Collidable (has collision shape)
					false,				// Dragable by user?
					col);// Render color
				this->AddGameObject(sphere);
				objects[x].push_back(sphere);
				sphere->Physics()->SetIsSoft(true);
			}
		}
	};
	create_ball_cloth(floor_pos + Vector3(0, 5, -5.5f), Vector3(0.5f, 0.5f, 0.5f), 0.05f);
	basket = AABB(floor_pos + Vector3(0, 4.5, -5.5f), 0.1f);

	//Create dinstance constraints for basket net.
	for (int i = 0; i < dimsx; i++) {
		for (int j = 0; j < dimsz; j++) {
			int indexi = i - 1;
			for (int c = 0; c < 3; c++) {
				int indexj = j - 1 + c;
				if (indexi < dimsx && indexi > -1 && indexj < dimsz && indexj > -1) {
					DistanceConstraint* constraint = new DistanceConstraint(
						objects[i][j]->Physics(),					//Physics Object A
						objects[indexi][indexj]->Physics(),					//Physics Object B
						objects[i][j]->Physics()->GetPosition(),	//Attachment Position on Object A	-> Currently the centre
						objects[indexi][indexj]->Physics()->GetPosition());	//Attachment Position on Object B	-> Currently the centre  
					PhysicsEngine::Instance()->AddConstraint(constraint);
				}

			}
			indexi = i;
			for (int c = 0; c < 3; c += 2) {
				int indexj = j - 1 + c;
				if (indexi < dimsx && indexi > -1 && indexj < dimsz && indexj > -1) {
					DistanceConstraint* constraint = new DistanceConstraint(
						objects[i][j]->Physics(),					//Physics Object A
						objects[indexi][indexj]->Physics(),					//Physics Object B
						objects[i][j]->Physics()->GetPosition(),	//Attachment Position on Object A	-> Currently the centre
						objects[indexi][indexj]->Physics()->GetPosition());	//Attachment Position on Object B	-> Currently the centre  
					PhysicsEngine::Instance()->AddConstraint(constraint);
				}
			}
			indexi = i + 1;
			for (int c = 0; c < 3; c++) {
				int indexj = j - 1 + c;
				if (indexi < dimsx && indexi > -1 && indexj < dimsz && indexj > -1) {
					DistanceConstraint* constraint = new DistanceConstraint(
						objects[i][j]->Physics(),					//Physics Object A
						objects[indexi][indexj]->Physics(),					//Physics Object B
						objects[i][j]->Physics()->GetPosition(),	//Attachment Position on Object A	-> Currently the centre
						objects[indexi][indexj]->Physics()->GetPosition());	//Attachment Position on Object B	-> Currently the centre  
					PhysicsEngine::Instance()->AddConstraint(constraint);
				}
			}
		}
	}

	for (int i = 0; i < dimsz; i++) {
		DistanceConstraint* constraint = new DistanceConstraint(
			objects[0][i]->Physics(),					//Physics Object A
			objects[dimsx - 1][i]->Physics(),					//Physics Object B
			objects[0][i]->Physics()->GetPosition(),	//Attachment Position on Object A	-> Currently the centre
			objects[dimsx - 1][i]->Physics()->GetPosition());	//Attachment Position on Object B	-> Currently the centre  
		PhysicsEngine::Instance()->AddConstraint(constraint);
	}

	
	this->AddGameObject(CommonUtils::BuildCuboidObjectNoTexture("c1",
		floor_pos,	//Position leading to 0.25 meter overlap on faces, and more on diagonals
		Vector3(5.0f, 0.2f, 7.0f),				//Half dimensions
		true,									//Has Physics Object
		0.0f,									//Infinite Mass
		true,									//Has Collision Shape
		false,									//Dragable by the user
		Vector4(1.0f, 1.0f, 1.0f, 1.0f)));	//Color
	GameObject* c1 = this->FindGameObject("c1");
	(*c1->Render()->GetChildIteratorStart())->GetMesh()->SetTexture(courtTex);

	this->AddGameObject(CommonUtils::BuildCuboidObjectNoTexture("c2",
		floor_pos + Vector3(0, 5, -7),								//Position
		Vector3(3.0f, 0.2f, 2.5f),				//Half dimensions
		true,									//Has Physics Object
		0.0f,									//Infinite Mass
		true,									//Has Collision Shape
		false,									//Dragable by the user
		Vector4(1.0f, 1.0f, 1.0f, 1.0f)));	//Color
	GameObject* c2 = this->FindGameObject("c2");
	c2->Physics()->SetFriction(1.0f);
	c2->Physics()->SetElasticity(0.8f);
	c2->Physics()->SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(1.0f, 0.0f, 0.0f), 90.0f));
	(*c2->Render()->GetChildIteratorStart())->GetMesh()->SetTexture(boardTex);
	c2->Physics()->SetOnCollisionCallback(&TargetScene::boardCollisionCheck);

	this->AddGameObject(CommonUtils::BuildCuboidObjectNoTexture("c3",
		floor_pos + Vector3(0, 2.5, -8.5),								//Position
		Vector3(0.7f, 0.7f, 3.0f),				//Half dimensions
		true,									//Has Physics Object
		0.0f,									//Infinite Mass
		true,									//Has Collision Shape
		false,									//Dragable by the user
		Vector4(0.1f, 0.8f, 0.1f, 1.0f)));	//Color
	GameObject* c3 = this->FindGameObject("c3");
	c3->Physics()->SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(1.0f, 0.0f, 0.0f), 90.0f));
	(*c3->Render()->GetChildIteratorStart())->GetMesh()->SetTexture(ironTex);

}

//Draw basket net (soft body)
void TargetScene::draw() {

	for (int i = 0; i < objects.size(); i++) {
		for (int j = 0; j < objects[i].size() - 1; j++) {
			NCLDebug::DrawThickLine(objects[i][j]->Physics()->GetPosition(), objects[i][j + 1]->Physics()->GetPosition(), 0.05f, Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		}
	}

	for (int i = 0; i < objects.size() - 1; i++) {
		for (int j = 0; j < objects[i].size(); j++) {
			if (j == 0)  NCLDebug::DrawThickLine(objects[i][j]->Physics()->GetPosition(), objects[i + 1][j]->Physics()->GetPosition(), 0.15f, Vector4(1.0f, 0.0f, 0.0f, 1.0f));
			else NCLDebug::DrawThickLine(objects[i][j]->Physics()->GetPosition(), objects[i + 1][j]->Physics()->GetPosition(), 0.05f, Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		}
	}

	for (int j = 0; j < objects[0].size(); j++) {
		if (j == 0)  NCLDebug::DrawThickLine(objects[objects.size() - 1][j]->Physics()->GetPosition(), objects[0][j]->Physics()->GetPosition(), 0.05f, Vector4(1.0f, 0.0f, 0.0f, 1.0f));
		else NCLDebug::DrawThickLine(objects[objects.size() - 1][j]->Physics()->GetPosition(), objects[0][j]->Physics()->GetPosition(), 0.05f, Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	}

}


bool TargetScene::boardCollisionCheck(PhysicsNode* self, PhysicsNode* collidingObject)
{
	col_check_map[collidingObject] = true;
	return false;
}


//Spawn a basketball
void TargetScene::spawn() {
	{
		
		Camera * camera = GraphicsPipeline::Instance()->GetCamera();
		float yaw = camera->GetYaw();
		float pitch = camera->GetPitch();
		float fx, fz, fy;
		fx = -sin(DegToRad(yaw));
		fz = -cos(DegToRad(yaw));

		Vector4 col = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		GameObject* sphere = CommonUtils::BuildSphereObject(
			"",					// Optional: Name
			camera->GetPosition(),				// Position
			0.5f,			// Half-Dimensions
			true,				// Physics Enabled?
			1.0f,				// Physical Mass (must have physics enabled)
			true,				// Physically Collidable (has collision shape)
			false,			// Dragable by user?
			col);// Render color
		sphere->Physics()->SetLinearVelocity((Vector3(fx * 16, 5 + pitch * 0.6, fz * 16)));
		sphere->Physics()->SetAngularVelocity((Vector3(-4*fx, 0, 4*fz)));
		sphere->Physics()->SetElasticity(0.4f);
		(*sphere->Render()->GetChildIteratorStart())->GetMesh()->SetTexture(ballTex);
		this->AddGameObject(sphere);
		basketballs.push_back(sphere->Physics());
		col_check_map[sphere->Physics()] = false;
	}
}

void TargetScene::drawBasket() {
	float d = 2 * basket.halfdim;
	Vector3 c1 = basket.corner1;
	Vector3 c2 = c1 + Vector3(-d, 0, 0);
	Vector3 c3 = c2 + Vector3(0, 0, d);
	Vector3 c4 = c3 + Vector3(d, 0, 0);

	Vector3 c5 = basket.corner2;
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

//Check AABB for intersection with ball (used to determine if scored).
void TargetScene::checkBasketballs() {
	for (int i = 0; i < basketballs.size(); i++) {
		if (basket.containsObject(basketballs[i])) {
			int score;
			score = 100;
			if (col_check_map[basketballs[i]]) {
				score = 50;
			}
			total_score += score;
			basketballs.erase(std::remove(basketballs.begin(), basketballs.end(), basketballs[i]), basketballs.end());
			scoresTime.push_back(1.5f);
			scores.push_back(score);
		}
	}
}


// Draw score in middle of the screen (orthographic)
void TargetScene::drawScore(float time) {
	for(int i = 0; i< scoresTime.size(); i++){
		if (scoresTime[i] > 0) {
			Vector4 textPos = Vector4(0.0f, 0.5f, -0.0f, 1.0f);
			NCLDebug::DrawTextCs(textPos + Vector4(0.0f , -scoresTime[i] *0.2, 0.0f, 1.0f) , 45, std::to_string(scores[i]), TEXTALIGN_LEFT, scores[i] == 100 ? Vector4(0.0f, 1.0f, 0.0f, 1.0f) : Vector4(1.0f, 0.0f, 0.0f, 1.0f));
			scoresTime[i] -= time;
		}
		else {
			scoresTime.erase(std::remove(scoresTime.begin(), scoresTime.end(), scoresTime[i]), scoresTime.end());
			scores.erase(std::remove(scores.begin(), scores.end(), scores[i]), scores.end());
		}
		
	}
}



void TargetScene::OnUpdateScene(float dt) {
	{   
		Scene::OnUpdateScene(dt);
		acum_time += dt;
		draw();
		//	drawBasket();
		checkBasketballs();
		drawScore(dt);
		NCLDebug::DrawTextCs(Vector4(0.0f, 0.0f, 0.0f, 1.0f), 20, "O", TEXTALIGN_CENTRE, Vector4(1.0f, 0.0f, 0.0f, 1.0f));

		//Update Rotating Objects!
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_J))
		{
			spawn();
		}

		NCLDebug::AddStatusEntry(Vector4(1.0f, 1.0f, 1.0f, 1.0f), "Score: %d", total_score);
	}
}