#pragma once

#include "NPC.h"
#include "Animation.h"
#include <box2d/box2d.h>
#include <SDL3/SDL.h>
#include "Pathfinding.h"
#include "Dialogue.h"

struct SDL_Texture;

class HANDMAN : public NPC
{
public:
	HANDMAN(Dialogue dialogueHandman, std::string name, SDL_Texture* texture, const char* tsxPath, Dialogue dialogue, Dialogue hasBought, Dialogue hasNotBought, EntityType entity);
	virtual ~HANDMAN();

	bool Awake();
	bool Start();
	bool Update(float dt);
	bool CleanUp();
	void OnCollision(PhysBody* physA, PhysBody* physB);


public:
	bool isPicked = false;

private:

	void GetPhysicsValues();
	
	void ApplyPhysics();
	void Draw(float dt);
	void Jump();
	


private:
	SDL_Texture* texture;
	AnimationSet anims;
	PhysBody* pbody;
	int texW, texH;
	const char* tsxPath;

public:
	int coinFx;
	int coinPickupFx;
	int xInicial;
	int yInicial;	
	Dialogue dialogueHANDMAN;
	Dialogue hasBought;
	Dialogue hasNotBought;
	bool hasBeenKilled = false;
	bool wantsBuy = false;
	bool isStoreOn = false;

	
};
