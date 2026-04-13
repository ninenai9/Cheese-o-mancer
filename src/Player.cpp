
// =====================
// PLAYER IMPLEMENTATION
// =====================

#include "Player.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Item.h"
#include "Log.h"
#include "Physics.h"
#include "EntityManager.h"
#include "Map.h"
#include "Protection.h"
#include "Scene.h"
#include "Window.h"

// Variables estaticas del jugador
int Player::score = 0;
bool Player:: IsPlayerProtected = false;

// =====================
// CONSTRUCTOR / DESTRUCTOR
// =====================

Player::Player() : Entity(EntityType::PLAYER)
{
	name = "Player";
	pbody = nullptr;
}

Player::~Player() {

}

// =====================
// CICLO DE VIDA
// =====================

bool Player::Awake() {

	//L03: TODO 2: Initialize Player parameters
	position = Vector2D(86, 86);
	return true;
}

bool Player::Start() {

	// load
	movefx = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/PREV/player_walk.wav");
	jumpfx = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/PREV/salto.wav");
	checkpointfx = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/PREV/checkpoint.wav");
	deathfx = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/PREV/player_death.wav");
	std::unordered_map<int, std::string> aliases = { {15,"idle"},{0,"move"},{6,"jump"},{18,"death"}, {24, "walk_protected"}, {30, "jump_protected"}, {39, "idle_protected"}};
	float limitUp = Engine::GetInstance().render->camera.h / 4;
	Engine::GetInstance().render->camera.y = limitUp;

	anims.LoadFromTSX("Assets/Textures/PREV/ghost-export.tsx", aliases);
	anims.SetCurrent("idle");

	//L03: TODO 2: Initialize Player parameters
	texture = Engine::GetInstance().textures->Load("Assets/Textures/placeholder_Jester.png");

	// L08 TODO 5: Add physics to the player - initialize physics body
	texW = 215;
	texH = 384;
	pbody = Engine::GetInstance().physics->CreateRectangle(position.getX(),position.getY(),texW,texH, bodyType::DYNAMIC);
	
	// L08 TODO 6: Assign player class (using "this") to the listener of the pbody. This makes the Physics module to call the OnCollision method
	pbody->listener = this;

	// L08 TODO 7: Assign collider type
	pbody->ctype = ColliderType::PLAYER;

	//initialize audio effect
	pickCoinFxId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/PREV/coin-collision-sound-342335.wav");

	respawnPosition = { PIXEL_TO_METERS(position.getX()), PIXEL_TO_METERS(position.getY()) };

	return true;
}

bool Player::Update(float dt)
{
	bool isPaused = Engine::GetInstance().scene->isPaused;
	const SDL_Rect& animFrame = anims.GetCurrentFrame();
	if (!isPaused) {
		if (animFrame.x == 160 && animFrame.y == 96 && isdead) {
			Reset();
		}
		GetPhysicsValues();
		Move();
		Jump();
		ApplyPhysics();
	}
	Draw(dt);

	CameraRender();
	if (!isPaused) {
		if (IsProtected) {
			//check if the protection has been active for more than 10 seconds
			static Uint32 protectionStartTime = SDL_GetTicks();
			Uint32 currentTime = SDL_GetTicks();
			if (currentTime - protectionStartTime >= 10000) {
				IsProtected = false;
				protectionStartTime = currentTime; //reset timer
				LOG("Protection expired");
			}
		}
		IsPlayerProtected = IsProtected;

		//Miramos si se ha clicado el boton para crear la fireball
		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_RIGHT) == KEY_DOWN) {
			ThrowFireBall(Side::RIGHT);
			LOG("Created Fireball");
		}
		else if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_LEFT) == KEY_DOWN) {
			ThrowFireBall(Side::LEFT);
			LOG("Created Fireball");
		}
		UpdateFireballs(dt);
	}
		
	
	return true;
}

void Player :: UpdateFireballs(float dt) {

	//for (auto it = fireballs.begin(); it != fireballs.end(); ) {

	//	if ((*it)->toDelete) { //si se tiene que borrar la destruye
	//		it = fireballs.erase(it);
	//	}
	//	else {
	//		(*it)->Update(dt);
	//		++it;
	//	}
	//}
}

// =====================
// MOVIMIENTO Y FISICAS
// =====================

void Player::GetPhysicsValues() {
	// Read current velocity
	velocity = Engine::GetInstance().physics->GetLinearVelocity(pbody);
	if(!godMode){ velocity = { 0, velocity.y }; }else{ velocity = { 0, 0 }; }
}

void Player::Move() {
	// Move left/right
	if (!isdead && Engine::GetInstance().scene->IsGamePaused() == false) {
		//speed = 4.0f + (float)score / 1000;

		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT && !isJumping) {
			velocity.x = -speed;
			if(IsProtected){
				anims.SetCurrent("walk_protected");
			}
			else {
				anims.SetCurrent("move");
			}
			isWalking = true;
			facingLeft = true; 
		}
		else if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
			velocity.x = -speed;
			isWalking = true;
			facingLeft = true; 
		}
		else if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT && !isJumping) {
			velocity.x = speed;
			if (IsProtected) {
				anims.SetCurrent("walk_protected");
			}
			else {
				anims.SetCurrent("move");
			}
			isWalking = true;
			facingLeft = false; 
		}
		else if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
			velocity.x = speed;
			isWalking = true;
			facingLeft = false; 

		}
		else { isWalking = false; }
		if (godMode) {

			if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT && !isJumping) {
				velocity.y = -speed;
				if (IsProtected) {
					anims.SetCurrent("walk_protected");
				}
				else {
					anims.SetCurrent("move");
				}
				isWalking = true;
			}
			else if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) {
				velocity.y = -speed;
				isWalking = true;
			}
			else if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT && !isJumping) {
				velocity.y = speed;
				if (IsProtected) {
					anims.SetCurrent("walk_protected");
				}
				else {
					anims.SetCurrent("move");
				}
				isWalking = true;
			}
			else if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) {
				velocity.y = speed;
				isWalking = true;
			}
		}
		if (!isWalking && !isJumping) {
			if (IsProtected) {
				anims.SetCurrent("idle_protected");
			}
			else {

				anims.SetCurrent("idle");
			}	
		}
		if (isJumping || !isCollidedFloor ) {
			if(IsProtected){
				anims.SetCurrent("jump_protected");
			}
			else { anims.SetCurrent("jump"); }
		}
	}
	if (isWalking) {

		int currentTime = (int)SDL_GetTicks();
		if (currentTime - lastStepTime > 350) {

			Engine::GetInstance().audio->PlayFx(movefx);
			lastStepTime = currentTime;
		}
	}
	if (!isWalking) {
		lastStepTime = 0;
	}

}

void Player::Jump() {
	// This function can be used for more complex jump logic if needed
	bool spacePressed = Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN;
	//LOG("Space pressed");

	bool canFirstJump = spacePressed && !isJumping && isCollidedFloor && !godMode; // seguido lo que propuso el profesor, añadido canFirstJump y Can second jump para acortar codigo
	bool canSecondJump = spacePressed && isJumping && !secondJump && !godMode;
	/*if ((Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && isJumping == false && !godMode && (isCollidedFloor)|| Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && isJumping && !secondJump && !godMode)) {
		Engine::GetInstance().audio->PlayFx(jumpfx);
		anims.SetCurrent("jump");
		isJumping = true;
		if (firstJump) {
		
		Engine::GetInstance().physics->ApplyLinearImpulseToCenter(pbody, 0.0f, -jumpForce, true);
			secondJump = false;
		}
		else { secondJump = true;

		Engine::GetInstance().physics->ApplyLinearImpulseToCenter(pbody, 0.0f, -jumpForce+0.1f, true);
		}
		firstJump = false;
		
	}
	*/
	
	if(spacePressed && canFirstJump){
		Engine::GetInstance().audio->PlayFx(jumpfx);
		if (IsProtected) {
			anims.SetCurrent("jump_protected");
		}
		else {
			anims.SetCurrent("jump");
		}
		isJumping = true;
 		Engine::GetInstance().physics->ApplyLinearImpulseToCenter(pbody, 0.0f, -jumpForce, true);
		LOG("Player jumped action");
		secondJump = false;
		firstJump = false;
	}
	else if(spacePressed && canSecondJump){
		Engine::GetInstance().audio->PlayFx(jumpfx);
		if(IsProtected){
			anims.SetCurrent("jump_protected");
		}
		else {
			anims.SetCurrent("jump");
		}
		Engine::GetInstance().physics->ApplyLinearImpulseToCenter(pbody, 0.0f, -jumpForce + 0.1f, true);
		secondJump = true;
	}
	
}

void Player::ApplyPhysics() {
	// Preserve vertical speed while jumping
	if (isJumping == true) {
		velocity.y = Engine::GetInstance().physics->GetYVelocity(pbody);

	}

	// Apply velocity via helper
	Engine::GetInstance().physics->SetLinearVelocity(pbody, velocity);
}

// =====================
// RENDER Y CAMARA
// =====================

void Player::Draw(float dt) {

	
	
	/*const SDL_Rect& animFrame = anims.GetCurrentFrame();
	
	anims.Update(dt);*/

	int x, y;
	pbody->GetPosition(x, y);
	position.setX((float)x);
	position.setY((float)y);

	
	Engine::GetInstance().render->DrawTexture(texture, x - texW/2, y - texH/2, 0, 1, 0, 0, 0, SDL_FLIP_NONE);
	/*SDL_FlipMode flip = facingLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE; 

	Engine::GetInstance().render->DrawTexture(texture, x - texW / 2, y - texH / 2, &animFrame, 1.0f, 0.0, INT_MAX, INT_MAX, flip);*/
}

void Player::CameraRender() {

	Vector2D mapSize = Engine::GetInstance().map->GetMapSizeInPixels();
	float limitLeft = Engine::GetInstance().render->camera.w / 4;
	float limitRight = mapSize.getX() - Engine::GetInstance().render->camera.w;
	int windowX;
	int windowY;
	Engine::GetInstance().window->GetWindowSize(windowX, windowY);
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_F10) == KEY_DOWN) {
		godMode = !godMode;
		if (godMode) { b2Body_SetGravityScale(pbody->body, 0); }
		else { b2Body_SetGravityScale(pbody->body, 1); }
	}

	if (position.getX() < Engine::GetInstance().render->camera.w / 4) {
		Engine::GetInstance().render->camera.x = limitLeft;
	}
	else if (position.getX() > mapSize.getX() - Engine::GetInstance().render->camera.w / 4) {
		Engine::GetInstance().render->camera.x = 3 * Engine::GetInstance().render->camera.w / 4 - mapSize.getX();
	}
	else { Engine::GetInstance().render->camera.x = (int)(-position.getX() + windowX * 1.5); }

	float limitUp = Engine::GetInstance().render->camera.h / 4;
	float limitDown = (3 * Engine::GetInstance().render->camera.h / 4) - mapSize.getY();

	if (position.getY() < Engine::GetInstance().render->camera.h / 4) {
		Engine::GetInstance().render->camera.y = limitUp;
	}
	else if (position.getY() > mapSize.getY() - Engine::GetInstance().render->camera.h / 4) {
		int x = 9;
		Engine::GetInstance().render->camera.y = limitDown;
	}
	else { Engine::GetInstance().render->camera.y = (int)(-position.getY() + windowY * 1.5); }
}

// =====================
// COLISIONES
// =====================

void Player::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM: {
		LOG("Collision PLATFORM");
		//reset the jump flag when touching the ground
		isJumping = false;
		firstJump = true;
		isCollidedFloor = true;

		break;
	}
	case ColliderType::PARED: {
		LOG("Collision PARED");
		isCollidedWall = true;

		break;
	}
	case ColliderType::COIN: {
		LOG("Collision ITEM");
		Engine::GetInstance().audio->PlayFx(pickCoinFxId);
		physB->listener->Destroy();
		AddPoints(10);
		break;
	}

	case ColliderType::SAVE: {
		LOG("Collision SAVE");
		if (physB->objectName != "Player") {
			Engine::GetInstance().audio->PlayFx(checkpointfx);
		}
		if (pbody != nullptr && !B2_IS_NULL(pbody->body)) {
			respawnPosition = b2Body_GetPosition(pbody->body);
		}
		break;
	}
	case ColliderType::DANGER: {


		if (!godMode) {
			if (lives > 0) {
				LOG("Collision DEATH");
				if (!isdead) {
					Engine::GetInstance().audio->PlayFx(deathfx);
				}
				isdead = true;
				anims.SetCurrent("death");
				lives--;

			}
			else {
				LOG("Collision DEATH AND NO MORE LIVES");
				lives--;
				if (!isdead) {
					Engine::GetInstance().audio->PlayFx(deathfx);
				}
				isdead = true;
				isDeadDefinitive = true;

			}



		}


		break;
	}case ColliderType::ENEMY: {



		if (!godMode) {
			if (!IsProtected) {
				if (lives > 0) {
					LOG("Collision DEATH");
					if (!isdead) {
						Engine::GetInstance().audio->PlayFx(deathfx);
					}
					isdead = true;
					anims.SetCurrent("death");
					lives--;
				}
				else {
					LOG("Collision DEATH AND NO MORE LIVES");
					lives--;
					if (!isdead) {
						Engine::GetInstance().audio->PlayFx(deathfx);
					}
					isdead = true;
					isDeadDefinitive = true;
					anims.SetCurrent("death");

				}
			}


		}
		else if (IsProtected) {

			LOG("Player protected from ENEMY collision");

		}

		break;
	}


	case ColliderType::FIREBALL: {
		if (godMode)break;
		if (IsPlayerProtected)break;
		FireBall* fb = static_cast<FireBall*>(physB->listener);
		if (!fb) break;
		EntityType owner = fb->CheckOwner();
		if (owner == EntityType::FINALBOSS) {
			LOG("Player hit by FIREBALL — dying!");
			if (lives > 0) {
				LOG("Collision DEATH");
				if (!isdead) {
					Engine::GetInstance().audio->PlayFx(deathfx);
				}
				isdead = true;
				anims.SetCurrent("death");
				lives--;
			}
			else {
				LOG("Collision DEATH AND NO MORE LIVES");
				lives--;
				if (!isdead) {
					Engine::GetInstance().audio->PlayFx(deathfx);
				}
				isdead = true;
				isDeadDefinitive = true;
			}
		}


		break;
	}
	case ColliderType::PROTECTION: {
		IsProtected = true;


		break;
	}
	case ColliderType::EXTRALIVE: {
		lives++;



		break;
	}
	case ColliderType::FINALBOSS: {
		if (lives > 0) {
			LOG("Collision DEATH");
			if (!isdead) {
				Engine::GetInstance().audio->PlayFx(deathfx);
			}
			isdead = true;
			anims.SetCurrent("death");
			lives--;
		}
		else {
			LOG("Collision DEATH AND NO MORE LIVES");
			lives--;
			if (!isdead) {
				Engine::GetInstance().audio->PlayFx(deathfx);
			}
			isdead = true;
			isDeadDefinitive = true;
		}


	}
	default:

		break;
	}
	LOG("Ammount of lives: %d", lives);
}

void Player::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		LOG("End Collision PLATFORM");
		isCollidedFloor = false;
		break;
	case ColliderType::ITEM:
		LOG("End Collision ITEM");
		break;
	case ColliderType::DANGER:
		LOG("End Collision DANGER");
		break;
	case ColliderType::PARED:
		LOG("End Collision PARED");
		isCollidedWall = false;
		break;
	case ColliderType::SAVE:
		LOG("End Collision SAVE");
		break;
	case ColliderType::FIREBALL:



		break;
	default:
		break;
	}
}

// =====================
// UTILIDADES
// =====================

Vector2D Player::GetPosition() {
	int x, y;
	pbody->GetPosition(x, y);
	// Adjust for center
	return Vector2D((float)x - texW / 2, (float)y - texH / 2);
}

bool Player::CleanUp()
{
	LOG("Cleanup player");
	Engine::GetInstance().textures->UnLoad(texture);
	if (pbody != nullptr) {
		Engine::GetInstance().physics->DeletePhysBody(pbody);
		pbody = nullptr;
	}
	return true;
}

void Player::SetPosition(Vector2D pos)
{
	
	this->position = pos;

	if (pbody != nullptr) {
		int centerX = (int)pos.getX() + texW / 2;
		int centerY = (int)pos.getY() + texH / 2;
		pbody->SetPosition(centerX, centerY);
	}
}

void Player::Reset()
{
	
	b2Vec2 initialPos = respawnPosition;
	b2Rot rotation = b2MakeRot(0.0f);
	b2Body_SetTransform(pbody->body, initialPos, rotation);
	Engine::GetInstance().physics->SetLinearVelocity(pbody, { 0.0f, 0.0f });
	isdead = false;
	isJumping = false;
	isCollidedFloor = true;
	firstJump = true;

	float limitUp = Engine::GetInstance().render->camera.h / 4;
	Engine::GetInstance().render->camera.y = limitUp;
	IsProtected = false;
	anims.SetCurrent("idle");
	
}

// L08 TODO 6: Define OnCollision function for the player. 


bool Player::isDead()
{
	return isdead;
}

void Player::AddPoints(int points)
{
	Player::score += points;
	LOG("Score: %d", Player::score);
}

bool Player::isPlayerProtectedquestion() {
	return IsPlayerProtected;
}

void Player::ThrowFireBall(Side side) {

	/*std::shared_ptr<FireBall> fireball =std::dynamic_pointer_cast<FireBall>(Engine::GetInstance().entityManager->CreateEntity(EntityType::FIREBALL));
	fireball->spawnPos = Vector2D(position.getX(), position.getY());
	fireball->spawnSide = side;

	fireball->Start();
	fireballs.push_back(fireball);*/
}