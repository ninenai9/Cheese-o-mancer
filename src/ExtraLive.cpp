#include "ExtraLive.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "EntityManager.h"

ExtraLive::ExtraLive() : Entity(EntityType::EXTRALIVE)
{
	name = "ExtraLive";
	pbody = nullptr;
}

ExtraLive::~ExtraLive() {}

bool ExtraLive::Awake() {
	return true;
}

bool ExtraLive::Start() {

	

	//Initialize Enemy parameters
	texture = Engine::GetInstance().textures->Load("Assets/Textures/PREV/ExtraLive.png");

	
	// L08 TODO 4: Add a physics to an item - initialize the physics body
	Engine::GetInstance().textures.get()->GetSize(texture, texW, texH);
	if (pbody == nullptr) {
		position.setX(xInicial);
		position.setY(yInicial);
		pbody = Engine::GetInstance().physics->CreateRectangleSensor(
			(int)position.getX() + texW / 2,
			(int)position.getY() + texH / 2 - 16,
			texW / 2,
			texH / 2,
			bodyType::DYNAMIC
		);
		b2Body_SetGravityScale(pbody->body, 0.0f);

		pbody->listener = this;
		



	pbody->ctype = ColliderType::EXTRALIVE;
	pbody->listener = this;   // so Begin/EndContact can call back to Item

	}

	// L08 TODO 7: Assign collider type

	// Set this class as the listener of the pbody

	return true;
}

bool ExtraLive::Update(float dt)
{
	if (!active) return true;



	if (pbody != nullptr) {
		int x = 0;
		int y = 0;
		pbody->GetPosition(x, y);


		
		Engine::GetInstance().render->DrawTexture(texture, x - texW / 2, y - texH / 2);
	}

	return true;
}

bool ExtraLive::CleanUp()
{
	LOG("Unloading Coin");
	Engine::GetInstance().textures->UnLoad(texture);
	if (pbody != nullptr) {
		Engine::GetInstance().physics->DeletePhysBody(pbody);
		pbody = nullptr;
	}
	return true;
}

bool ExtraLive::Destroy()
{
	LOG("Destroying item");
	active = false;
	Engine::GetInstance().entityManager->DestroyEntity(shared_from_this());
	return true;
}
void ExtraLive::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype)
	{
	case ColliderType::PLAYER:
		LOG("Coin picked up!");
		

		toDelete = true;
		Engine::GetInstance().physics->DeletePhysBody(pbody);
		pbody = nullptr;
		break;

	default:
		break;
	}
}