#include "HANDMAN.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "EntityManager.h"

HANDMAN::HANDMAN(Dialogue dialogueHandman, std::string name, SDL_Texture* texture, const char *tsxPath, Dialogue dialogue, Dialogue hasBought, Dialogue hasNotBought, EntityType entitytype, Dialogue BeatBoss) : NPC(name, texture, tsxPath, dialogue, entitytype)
{
	
	this->name = name;
	this->texture = texture;
	this->tsxPath = tsxPath;
	this->dialogue = dialogue; //dialogo al descubrirlo por primera vez
	pbody = nullptr;
	dialogueHANDMAN = dialogueHandman; //dialogo despues de beat el boss
	this->hasBought = hasBought; //dialogo salir habiendo comprado
	this->hasNotBought = hasNotBought; //dialogo salir sin haber comprado
	this->BeatBoss = BeatBoss;
}

HANDMAN::HANDMAN() : NPC("", nullptr, "", Dialogue(), EntityType::UNKNOWN)
{
	pbody = nullptr;
}

HANDMAN::~HANDMAN() {
	if (pbody != nullptr) {
		Engine::GetInstance().physics->DeletePhysBody(pbody);
		pbody = nullptr;
	}
}

bool HANDMAN::Awake() {
	return true;
}

bool HANDMAN::Start() {


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

bool HANDMAN::Update(float dt)
{
	if (!active) return true;

	anims.Update(dt);



	return true;
}

bool HANDMAN::CleanUp()
{
	LOG("Unloading Coin");
	Engine::GetInstance().textures->UnLoad(texture);
	if (pbody != nullptr) {
		Engine::GetInstance().physics->DeletePhysBody(pbody);
		pbody = nullptr;
	}
	return true;
}
void HANDMAN::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype)
	{
	case ColliderType::PLAYER:
		
		Engine::GetInstance().audio->PlayFx(coinPickupFx); //audio queue
		Player* py = static_cast<Player*>(physB->listener);
		if (firstTime == true && Engine::GetInstance().input->GetKey(SDL_SCANCODE_E) == KEY_DOWN) {

			if (dialogue.hasStarted)break;
			dialogue.BeginDialogue();
			firstTime = false;
			break; //primer cop que el player li parla
		}
		//parlarli i no has beat el boss encara
		if (py->beatBoss == false && Engine::GetInstance().input->GetKey(SDL_SCANCODE_E) == KEY_DOWN) {
			if (dialogueHANDMAN.hasStarted)break;
			dialogueHANDMAN.BeginDialogue();
		}
		//parlarli i has beat el boss primer cop
		if (py->beatBoss && firstTimeBossKill && Engine::GetInstance().input->GetKey(SDL_SCANCODE_E) == KEY_DOWN) {
			if (BeatBoss.hasStarted)break;
			BeatBoss.BeginDialogue();
			firstTimeBossKill = false;
			break; 
		}
		//parlarli (obre store)
		else if(py->beatBoss && Engine::GetInstance().input->GetKey(SDL_SCANCODE_E) == KEY_DOWN){
			isStoreOn=!isStoreOn;
			if (isStoreOn == true)moneyPlayer = py->score;	
			Engine::GetInstance().scene->SetStore(isStoreOn);
			if (isStoreOn == false) {
				if (moneyPlayer == py->score) { //no ha comprat
					if (hasNotBought.hasStarted)break;
					hasNotBought.BeginDialogue();
				}
				else {//h comprat
					if (hasBought.hasStarted)break;
					hasBought.BeginDialogue();
				}
			
			}
		
		
		
		
		}
		
		
		
		
		break;

	
		break;
	}
}
