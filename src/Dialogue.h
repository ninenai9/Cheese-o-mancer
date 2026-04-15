#pragma once

#include "Entity.h"
#include "Animation.h"
#include <box2d/box2d.h>
#include <SDL3/SDL.h>
#include "Pathfinding.h"

struct SDL_Texture;

class Dialogue 
{
public:
	Dialogue();
	virtual ~Dialogue();

	bool Awake();
	bool Start();
	bool Update(float dt);
	bool CleanUp();

	void OnCollision(PhysBody* physA, PhysBody* physB);
	void NextDialogue();



private:

	void GetPhysicsValues();
	
	void ApplyPhysics();
	void Draw(float dt);
	


private:
	//Vector de texturas
	std::vector<SDL_Texture*> dialogue;
	std::vector<SDL_Texture*> dialogueHelper;

public:
	
	int xInicial;
	int yInicial;	
	int lenght = 0;
	void AddDialogue(SDL_Texture* texture);
	void BeginDialogue();
	bool HasEnded(bool name);

	bool hasEnded = false;
	bool hasStarted = false;
};
