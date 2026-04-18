#pragma once

#include "Entity.h"
#include "Animation.h"
#include <box2d/box2d.h>
#include <SDL3/SDL.h>
#include "FireBall.h"
#include "CheeseBall.h"

enum PlayerState {
	JUMPING,
	FALLING,
	ATTACKING,
	RUNNING,
	ONCHEESE,
	IDLE,
	DEFAULT

};

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
	void Attack();

	void HandleAttack();

	void StartAttack(int combo);

	void UpdateAttackHitbox();

	void SpawnCheeseBall();

	void HandleMountedMovement();

	void DismountAndLaunch();

private:

	void GetPhysicsValues();
	void Move();
	void Jump();
	void ApplyPhysics();
	void Draw(float dt);
	void ThrowFireBall(Side side);

	void ChangeCurrentAnimation();

public:
	PlayerState state;
	PlayerState lastState;
	bool hasMap1 = false;
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
	float jumpForce = 950.0f; // The force to apply when jumping
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

	//MARC ESTOS SON LOS BOOLS DE EL PLAYER PARA CONVERSACIONES

	bool hasCheese = false;
	bool beatBoss = false;
	bool hasTalkedWithMagician = false; //abrir puerta para poder seguir / desbloquea la vida UI
	
	/*SDL_Rect& lastFrame;*/
private: 
	b2Vec2 velocity;
	AnimationSet anims2x3;
	AnimationSet anims3x3;
	AnimationSet anims3x4;
	AnimationSet anims4x4;
	AnimationSet anims5x5;
	AnimationSet* currentAnimSet = nullptr;
	SDL_Texture* texture2x3 = NULL;
	SDL_Texture* texture3x3 = NULL;
	SDL_Texture* texture3x4 = NULL;
	SDL_Texture* texture4x4 = NULL;
	SDL_Texture* texture5x5 = NULL;
	std::string currentAnimName;

	int attackCombo = 0;
	bool attackRequested = false;
	bool isAttacking = false;
	bool bufferedAttack = false;
	Uint32 bufferTime = 200; // ms
	Uint32 bufferStart = 0;

	PhysBody* attackHitbox = nullptr;

	bool hitboxActive = false;
	bool hasHit = false;
	bool playerInHitbox = false;

	int attackDuration = 20;
	int attackTimer = 0;

	// frames donde golpea (ajústalos)
	int hitboxStart = 5;
	int hitboxEnd = 12;

	// offset respecto al player
	int offsetAttackHitboxX = 160;
	int offsetAttackHitboxY = -50;

	std::shared_ptr<CheeseBall> mountedBall = nullptr;
	bool isMounted = false;
	int spawnOffset = 30;
};