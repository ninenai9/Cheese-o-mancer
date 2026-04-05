#include "coins.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "EntityManager.h"

Coins::Coins() : Entity(EntityType::COIN)
{
	name = "Coin";
	pbody = nullptr;
}

Coins::~Coins() {
	if (pbody != nullptr) {
		Engine::GetInstance().physics->DeletePhysBody(pbody);
		pbody = nullptr;
	}
}

bool Coins::Awake() {
	return true;
}

bool Coins::Start() {

	std::unordered_map<int, std::string> aliases = { {0, "idle"} };
	anims.LoadFromTSX("Assets/Textures/PREV/coin_sprite.tsx", aliases);
	coinPickupFx = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/PREV/coin-collision-sound-342335.wav");
	anims.SetCurrent("idle");

	texture = Engine::GetInstance().textures->Load("Assets/Textures/PREV/coin_sprite.png");

	
		texW = 32;
		texH = 32;
	
	
	if (pbody == nullptr) {
		position.setX(xInicial);
		position.setY(yInicial);
		pbody = Engine::GetInstance().physics->CreateRectangleSensor(
			(int)position.getX() + texW / 2,
			(int)position.getY() + texH / 2 - 16,
			texW / 2,
			texH/2,
			bodyType::DYNAMIC
		);
		b2Body_SetGravityScale(pbody->body, 0.0f);
	
		pbody->listener = this;
		pbody->ctype = ColliderType::COIN;



		
	}
	

	/*if (pbody == nullptr) {
		pbody = Engine::GetInstance().physics->CreateRectangleSensor(
			(int)position.getX() + texW / 2 + xOffset,
			(int)position.getY() + texH / 2 + yOffset,
			texW,
			texH,.
			bodyType::STATIC
		);
		pbody->ctype = ColliderType::COIN;
		pbody->listener = this;

	}*/

	return true;
}

bool Coins::Update(float dt)
{
	if (!active) return true;

	anims.Update(dt);

	if (pbody != nullptr) {
		int x = 0;
		int y =0;
		pbody->GetPosition(x, y);
		

		const SDL_Rect& currentFrame = anims.GetCurrentFrame();
		Engine::GetInstance().render->DrawTexture(texture, x - texW / 2, y - texH / 2 , &currentFrame);
	}

	return true;
}

bool Coins::CleanUp()
{
	LOG("Unloading Coin");
	Engine::GetInstance().textures->UnLoad(texture);
	if (pbody != nullptr) {
		Engine::GetInstance().physics->DeletePhysBody(pbody);
		pbody = nullptr;
	}
	return true;
}

void Coins::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype)
	{
	case ColliderType::PLAYER:
		LOG("Coin picked up!");
		Engine::GetInstance().audio->PlayFx(coinPickupFx);

		toDelete = true;
		Engine::GetInstance().physics->DeletePhysBody(pbody);
		pbody = nullptr;
		break;

	default:
		break;
	}
}