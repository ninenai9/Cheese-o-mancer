#include "Checkpoint.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Physics.h"

Checkpoint::Checkpoint() : Entity(EntityType::CHECKPOINT)
{
	name = "Checkpoint";
}

Checkpoint::~Checkpoint() {}

bool Checkpoint::Start() {
	texture = Engine::GetInstance().textures->Load("Assets/Textures/PREV/checkpoint-Sheet.png");

	fxId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/PREV/checkpoint.wav");


	idleAnim.AddFrame({ 0, 0, 90, 90 }, 100);


	for (int i = 0; i < 6; i++) {
		activateAnim.AddFrame({ i * 90, 0, 90, 90 }, 100);
	}
	activateAnim.SetLoop(false);
	currentAnim = &idleAnim;

	pbody = Engine::GetInstance().physics->CreateRectangleSensor(
		(int)position.getX() + 16,
		(int)position.getY() + 16,
		32, 32,
		bodyType::STATIC
	);
	pbody->ctype = ColliderType::SAVE;
	pbody->listener = this;

	return true;
}

bool Checkpoint::Update(float dt)
{
	if (currentAnim != nullptr)
		currentAnim->Update(dt);

	int drawX = (int)position.getX() - (90 - 32) / 2;
	int drawY = (int)position.getY() - (90 - 32);

	Engine::GetInstance().render->DrawTexture(texture, drawX, drawY, &currentAnim->GetCurrentFrame());

	return true;
}

void Checkpoint::OnCollision(PhysBody* physA, PhysBody* physB)
{

	if (physB->ctype == ColliderType::PLAYER && !isActivated) {
		isActivated = true;
		currentAnim = &activateAnim;
		currentAnim->Reset();
		Engine::GetInstance().audio->PlayFx(fxId);
	}
}

bool Checkpoint::CleanUp()
{
	Engine::GetInstance().textures->UnLoad(texture);
	if (pbody != nullptr) {
		Engine::GetInstance().physics->DeletePhysBody(pbody);
		pbody = nullptr;
	}
	return true;
}