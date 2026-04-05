#include "Enemy.h"
#include "FireBall.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "EntityManager.h"
#include "Map.h"


FireBall::FireBall() : Entity()
{
	name = "FireBall";
}

FireBall::~FireBall() {

}


bool FireBall::Awake() {
	return true;
}
bool FireBall::Start() {

	texture = Engine::GetInstance().textures->Load("Assets/Textures/PREV/fuego.png");

	texW = 9;
	texH = 9;
	int offsetx = 0;
	ChooseSide(spawnSide);
	if (CheckOwner() == EntityType::PLAYER) {
		if(spawnSide==Side::LEFT)offsetx = -32;
	else
			offsetx = 0;
	
	}
	int offsety = 0;
	//Here i will make the fireball offset depending on the boss messurements 32x50
	if(CheckOwner()==EntityType::FINALBOSS) {
		if (spawnSide == Side::LEFT)offsetx = -32;
		else if(spawnSide==Side::RIGHT) offsetx = 32;
		else if(spawnSide==Side::UP) offsety = -50;
		else if (spawnSide == Side::DOWN) offsety = 50;
		else if(spawnSide==Side::DIAGONALUL) {
			offsetx = -32;
			offsety = -50;
		}
		else if (spawnSide == Side::DIAGONALUR) {
			offsetx = 32;
			offsety = -50;
		}
		else if (spawnSide == Side::DIAGONALDL) {
			offsetx = -32;
			offsety = 50;
		}
		else if (spawnSide == Side::DIAGONALDR) {
			offsetx = 32;
			offsety = 50;
		}
	}
	// CREA EL PBODY
	pbody = Engine::GetInstance().physics->CreateCircle(
		(int)spawnPos.getX() + texW / 2 + offsetx,
		(int)spawnPos.getY() + texH / 2 + offsety,
		texW / 2,
		bodyType::DYNAMIC
	);
	b2Body_SetGravityScale(pbody->body, 0.0f);

	pbody->listener = this;
	pbody->ctype = ColliderType::FIREBALL;

	// APLICA LA DIRECCIÓN ELEGIDA
	 fireballfx = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/PREV/fireball.wav");
	Engine::GetInstance().audio->PlayFx(fireballfx);
	Engine::GetInstance().physics->SetLinearVelocity(pbody, velocity.x, velocity.y);
	storedVelocity = velocity;
	return true;
}

bool FireBall::Update(float dt)
{
	bool isPaused = Engine::GetInstance().scene->isPaused;

	if (isPaused && pbody && !wasPaused) {
		
		Engine::GetInstance().physics->SetLinearVelocity(pbody, 0.0f, 0.0f);
		wasPaused = true;
	}

	if (!isPaused && pbody && wasPaused) {
		Engine::GetInstance().physics->SetLinearVelocity(
			pbody,
			storedVelocity.x,
			storedVelocity.y
		);
		wasPaused = false;
	}

	if (!toDelete) {
		Draw();
		return true;
	}

	if (pbody) {
		Engine::GetInstance().physics->DeletePhysBody(pbody);
		pbody = nullptr;
	}

	return true;
}


void FireBall::GetPhysicsValues() {

	velocity = Engine::GetInstance().physics->GetLinearVelocity(pbody);

	velocity = { velocity.x, velocity.y };

}

void FireBall::ChooseSide(Side NAME) {

	if (NAME == Side::LEFT) {
		velocity.x = -5.0f;
		velocity.y = 0.0f;
	}
	else if(NAME==Side::RIGHT)  {
		velocity.x = 5.0f;
		velocity.y = 0.0f;
	}
	else if (NAME == Side::UP) {
		velocity.x = 0.0f;
		velocity.y = -5.0f;
	}
	else if (NAME == Side::DOWN) {
		velocity.x = 0.0f;
		velocity.y = 5.0f;
	}
	else if (NAME == Side::DIAGONALUL) {
		velocity.x = -3.5f;
		velocity.y = -3.5f;
	}
	else if (NAME == Side::DIAGONALUR) {
		velocity.x = 3.5f;
		velocity.y = -3.5f;
	}
	else if (NAME == Side::DIAGONALDL) {
		velocity.x = -3.5f;
		velocity.y = 3.5f;
	}
	else if (NAME == Side::DIAGONALDR) {
		velocity.x = 3.5f;
		velocity.y = 3.5f;
	}
	
}

void FireBall::Draw() {
	if (toDelete) return;
	int x, y;
	pbody->GetPosition(x, y);
	position.setX((float)x);
	position.setY((float)y);

	Engine::GetInstance().render->DrawTexture(texture, x - texW / 2, y - texH / 2, NULL);

}
void FireBall::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		LOG("Collision PLATFORM");
		toDelete = true;
		
		
		break;

	case ColliderType::PARED:
		LOG("Collision PARED");
		toDelete = true;
		
		
		break;

	case ColliderType::ENEMY:
		LOG("Enemy hit by FIREBALL — dying!");
		toDelete = true;
		
		
		break;
	case ColliderType::PLAYER:
		LOG("End Collision FIREBALL");
		if(ownerType==EntityType::FINALBOSS) {
			LOG("Player hit by FIREBALL — taking damage!");
		}
			toDelete = true;
		
		break;
	case ColliderType::FINALBOSS:
		if(ownerType == EntityType::PLAYER) {
			LOG("Final Boss hit by FIREBALL — taking damage!");
		}
			toDelete = true;
			
		
		break;
	case ColliderType::FIREBALL: {
	
		toDelete = true;
		
	
	}
	default:
		break;
	}
}

void FireBall::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		LOG("End Collision PLATFORM");
		isCollidedFloor = false;
		toDelete = true;
		

		break;

	case ColliderType::PARED:
		LOG("End Collision PARED");
		isCollidedWall = false;
		toDelete = true;

		break;
	case ColliderType::PLAYER:
		LOG("End Collision FIREBALL");
		break;
	default:

		break;
	}
}

bool FireBall::CleanUp()
{
	LOG("Cleanup FireBall");
	Engine::GetInstance().textures->UnLoad(texture);
	return true;
}
void FireBall::SetPosition(Vector2D pos) {
	pbody->SetPosition((int)(pos.getX()), (int)(pos.getY()));
}
Vector2D FireBall::GetPosition() {
	int x, y;
	pbody->GetPosition(x, y);
	// Adjust for center
	return Vector2D((float)x - texW / 2, (float)y - texH / 2);
}
