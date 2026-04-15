#include "NPC.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "EntityManager.h"

NPC::NPC(std::string name, SDL_Texture* texture, const char *tsxPath, Dialogue dialogue, EntityType entityType) : Entity(entityType)
{
	
	this->name = name;
	this->texture = texture;
	this->tsxPath = tsxPath;
	this->dialogue = dialogue;
	pbody = nullptr;
}

NPC::NPC()
{
}

NPC::~NPC() {
	if (pbody != nullptr) {
		Engine::GetInstance().physics->DeletePhysBody(pbody);
		pbody = nullptr;
	}
}

bool NPC::Awake() {
	return true;
}

bool NPC::Start() {


	std::unordered_map<int, std::string> aliases = { {0, "idle"} };
	anims.LoadFromTSX(tsxPath, aliases);
	/*coinPickupFx = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/PREV/coin-collision-sound-342335.wav");*/
	anims.SetCurrent("idle");

	texture = Engine::GetInstance().textures->Load("Assets/Textures/PREV/coin_sprite.png");

	//32 sujeto a cambio, el tile del tsx es de 32x32 en el ejemplo, luego hare que sea algo que viene de constructor o algo asi
		texW = 32;
		texH = 32;
	
		pbody = Engine::GetInstance().physics->CreateRectangleSensor(
			(int)position.getX() + 16,
			(int)position.getY() + 16,
			32, 32,
			bodyType::STATIC
		);
	
	

	return true;
}

bool NPC::Update(float dt)
{
	if (!active) return true;

	anims.Update(dt);



	return true;
}

bool NPC::CleanUp()
{
	LOG("Unloading Coin");
	Engine::GetInstance().textures->UnLoad(texture);
	if (pbody != nullptr) {
		Engine::GetInstance().physics->DeletePhysBody(pbody);
		pbody = nullptr;
	}
	return true;
}
void NPC::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype)
	{
	case ColliderType::PLAYER:
		
		Engine::GetInstance().audio->PlayFx(coinPickupFx); //audio queue

		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_E) == KEY_DOWN) {
			if (dialogue.hasStarted)break;
			dialogue.BeginDialogue();
		}
		break;

	default:
		break;
	}
}
