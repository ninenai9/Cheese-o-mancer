#pragma once

#include "Entity.h"
#include "Animation.h"
#include <box2d/box2d.h>
#include <SDL3/SDL.h>
#include "FireBall.h"



class Player : public Entity
{
public:

	Player();
	virtual ~Player();

	bool Awake();
	bool Start();
	bool Update(float dt);
	bool CleanUp();
	void Reset();

	// L08 TODO 6: Define OnCollision function for the player. 
	void OnCollision(PhysBody* physA, PhysBody* physB);
	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);
	bool isDead();

	Vector2D GetPosition();
	void SetPosition(Vector2D pos);
	void CameraRender();
	void UpdateFireballs(float dt);
	static bool isPlayerProtectedquestion();
	static bool IsPlayerProtected;

private:

	void GetPhysicsValues();
	void Move();
	void Jump();
	void ApplyPhysics();
	void Draw(float dt);
	void ThrowFireBall(Side side);

public:
	void Player::Setanimation();
	//Declare player parameters
	float speed = 15.0f;
	SDL_Texture* texture = NULL;
	int texW, texH;
	//Audio fx
	int pickCoinFxId;
	//posicion respawn
	b2Vec2 respawnPosition;
	//fx
	int lastStepTime = 0;
	int movefx;
	int jumpfx;	
	int checkpointfx;
	int deathfx;
	// L08 TODO 5: Add physics to the player - declare a Physics body
	PhysBody* pbody;
	float jumpForce = 1100.0f; // The force to apply when jumping
	bool isJumping = false; // Flag to check if the player is currently jumping
	bool isMoving = false;
	bool secondJump = false;
	bool firstJump = true;
	bool godMode = false;
	bool isdead = false;
	bool isWalking = false;
	bool isCollidedWall = false;
	bool isCollidedFloor = false;
	bool facingLeft = false;
	std::vector<std::shared_ptr<FireBall>> fireballs;
	
	bool IsProtected = false;
	static void AddPoints(int points);
	//Score and lives

	static int score;
	int lives = 3;
	bool isDeadDefinitive = false;
	
	/*SDL_Rect& lastFrame;*/
private: 
	b2Vec2 velocity;
	AnimationSet anims;

};