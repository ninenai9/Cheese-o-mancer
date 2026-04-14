#pragma once

#include "Entity.h"
#include "Animation.h"
#include <box2d/box2d.h>
#include <SDL3/SDL.h>
#include "Pathfinding.h"

struct SDL_Texture;

class Enemy : public Entity
{
public:

	Enemy();
	virtual ~Enemy();
	bool Awake();
	bool Start();
	virtual bool Update(float dt);
	bool CleanUp();
	virtual void OnCollision(PhysBody* physA, PhysBody* physB);
	virtual void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	void CreateAttackHitbox(int x, int y, int w, int h);
	
	float CalculateDistance();
	void SetPosition(Vector2D pos);
	Vector2D GetPosition();

	void ResetPathfinding(Vector2D pos);

	virtual void Attack();

protected:
	void PerformPathfinding();
	void GetPhysicsValues();
	virtual void Move();
	void ApplyPhysics();
	void Draw(float dt);

public:

	//Declare enemy parameters
	float speed = 4.0f;
	SDL_Texture* texture = NULL;
	int texW = 215;
	int texH = 384;
	PhysBody* pbody;
	float detectionRange = 400.0f;
	Vector2D lastPlayerTile = { -1, -1 };
	int repathTimer = 0;
	int repathDelay = 100;
	char* texName = "";
	bool isboss = false;
	int attackRange = 0;
	float distanceToPlayer = 0.0f;
	int offsetAttackHitboxX = 0;
	int offsetAttackHitboxY = 0;
protected:
	b2Vec2 velocity;
	AnimationSet anims;
	std::shared_ptr<Pathfinding> pathfinding;
	PhysBody* attackHitbox;
};