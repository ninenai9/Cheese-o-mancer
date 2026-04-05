#include "Protection.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "EntityManager.h"

Protection::Protection() : Entity(EntityType::PROTECTION)
{
	name = "Protection";
	pbody = nullptr;
}

Protection::~Protection() {
	if (pbody != nullptr) {
		Engine::GetInstance().physics->DeletePhysBody(pbody);
		pbody = nullptr;
	}
}

bool Protection::Awake()
{
	return true;
}

bool Protection::Start()
{
	// initialize textures

	std::unordered_map<int, std::string> aliases = { {0, "idle"} };
	anims.LoadFromTSX("Assets/Textures/PREV/heart_prtection_anim.tsx", aliases);
	texture = Engine::GetInstance().textures->Load("Assets/Textures/PREV/heart_prtection.png");
	anims.SetCurrent("idle");


	texW = 32;
	texH = 32;

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
	pbody->ctype = ColliderType::PROTECTION;



	}






	return true;
}

bool Protection::Update(float dt)
{
	if (!active) return true;

	anims.Update(dt); 

	if (pbody != nullptr) {
		int x = 0;
		int y = 0;
		pbody->GetPosition(x, y);


		const SDL_Rect& currentFrame = anims.GetCurrentFrame();
		Engine::GetInstance().render->DrawTexture(texture, x - texW / 2, y - texH / 2, &currentFrame);
	}
	
	return true;
}

bool Protection::CleanUp()
{
	Engine::GetInstance().textures->UnLoad(texture);
	if (pbody != nullptr) {
		Engine::GetInstance().physics->DeletePhysBody(pbody);
		pbody = nullptr;
	}
	return true;
}

bool Protection::Destroy()
{
	LOG("Destroying protection");
	active = false;
	Engine::GetInstance().entityManager->DestroyEntity(shared_from_this());
	return true;
}
void Protection::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype)
	{
	case ColliderType::PLAYER:
		LOG("PROTECTION picked up!");


		toDelete = true;
		Engine::GetInstance().physics->DeletePhysBody(pbody);
		pbody = nullptr;
		break;

	default:
		break;
	}
}
