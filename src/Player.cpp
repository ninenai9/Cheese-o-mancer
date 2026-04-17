
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
#include "CheeseBall.h"

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

	state = RUNNING;
	lastState = RUNNING;
	// load
	movefx = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/PREV/player_walk.wav");
	jumpfx = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/PREV/salto.wav");
	checkpointfx = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/PREV/checkpoint.wav");
	deathfx = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/PREV/player_death.wav");
	std::unordered_map<int, std::string> aliases2x3 = { {0,"run"},{10,"jump"},{20,"hoponcheese"}};
	std::unordered_map<int, std::string> aliases3x3 = { {0,"run"},{10,"jump"},{20,"hoponcheese"},{18,"death"}, {24, "walk_protected"}, {30, "jump_protected"}, {39, "idle_protected"} };
	std::unordered_map<int, std::string> aliases3x4 = { {0,"attack1"},{6,"attack2"},{10,"attack3"}};
	std::unordered_map<int, std::string> aliases4x4 = { {0,"ballroll"}};
	std::unordered_map<int, std::string> aliases5x5 = { {0,"ballattack"}};
	float limitUp = Engine::GetInstance().render->camera.h / 4;
	Engine::GetInstance().render->camera.y = limitUp;

	anims2x3.LoadFromTSX("assets/Textures/Spritesheets/Jester/2x3/j_sp.tsx", aliases2x3);
	anims3x3.LoadFromTSX("assets/Textures/Spritesheets/Jester/3x3/j_sp_3x3.tsx", aliases3x3);
	anims3x4.LoadFromTSX("assets/Textures/Spritesheets/Jester/3x4/j_sp_3x4.tsx", aliases3x4);
	anims4x4.LoadFromTSX("assets/Textures/Spritesheets/Jester/4x4/j_sp_ballroll5x5.tsx", aliases4x4);
	anims5x5.LoadFromTSX("assets/Textures/Spritesheets/Jester/5x5/j_sp_5x5.tsx", aliases5x5);

	texture2x3 = Engine::GetInstance().textures->Load("assets/Textures/Spritesheets/Jester/2x3/j_2x3.png");
	//texture3x3 = Engine::GetInstance().textures->Load("assets/Textures/Spritesheets/Jester/3x3/j_3x3.png");
	texture3x4 = Engine::GetInstance().textures->Load("assets/Textures/Spritesheets/Jester/3x4/j_3x4.png");
	texture4x4 = Engine::GetInstance().textures->Load("assets/Textures/Spritesheets/Jester/4x4/j_ballroll.png");
	texture5x5 = Engine::GetInstance().textures->Load("assets/Textures/Spritesheets/Jester/5x5/j_5x5.png");
	//L03: TODO 2: Initialize Player parameters
	texture = texture2x3;
	currentAnimSet = &anims2x3;
	currentAnimSet->SetCurrent("run");
	// L08 TODO 5: Add physics to the player - initialize physics body
	texW = 215;
	texH = 384;
	pbody = Engine::GetInstance().physics->CreateRectangle(position.getX(),position.getY(),texW,texH, bodyType::DYNAMIC);
	
	// L08 TODO 6: Assign player class (using "this") to the listener of the pbody. This makes the Physics module to call the OnCollision method
	pbody->listener = this;

	// L08 TODO 7: Assign collider type
	pbody->ctype = ColliderType::PLAYER;

	attackHitbox = Engine::GetInstance().physics->CreateRectangleSensor(
		position.getX(),
		position.getY(),
		80,   // ancho
		120,  // alto
		bodyType::KINEMATIC
	);

	attackHitbox->listener = this;
	attackHitbox->ctype = ColliderType::PLAYERATTACK;
	hitboxActive = false;

	//initialize audio effect
	pickCoinFxId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/PREV/coin-collision-sound-342335.wav");

	respawnPosition = { PIXEL_TO_METERS(position.getX()), PIXEL_TO_METERS(position.getY()) };
	
	return true;
}

bool Player::Update(float dt)
{
	bool isPaused = Engine::GetInstance().scene->isPaused;
	const SDL_Rect& animFrame = currentAnimSet->GetCurrentFrame();
	if (!isPaused) {
		if (animFrame.x == 160 && animFrame.y == 96 && isdead) {
			Reset();
		}
		GetPhysicsValues();
		Move();
		Attack();
		HandleAttack();
		Jump();
		SpawnCheeseBall();
		ChangeCurrentAnimation();
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
				state = RUNNING;
			}
			else {
				state = RUNNING;
				
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
				state = RUNNING;
				
			}
			else {
				state = RUNNING;
				
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
					/*ChangeCurrentAnimationSet(anims3x3,texture3x3);
					currentAnimSet.SetCurrent("idle");*/
				}
				else {
					/*ChangeCurrentAnimationSet(anims3x3, texture3x3);
					currentAnimSet.SetCurrent("idle");*/
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
					state = JUMPING;
					
				}
				else {
					state = JUMPING;
					
				}
				isWalking = true;
			}
			else if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) {
				velocity.y = speed;
				isWalking = true;
			}
		}
		if (!isWalking && !isJumping) {
			/*if (IsProtected) {
				ChangeCurrentAnimationSet(anims3x3, texture3x3);
				currentAnimSet.SetCurrent("idle");
			}
			else {
				ChangeCurrentAnimationSet(anims3x3, texture3x3);
				currentAnimSet.SetCurrent("idle");
			}	*/
		}
		if (isJumping || !isCollidedFloor ) {
			if(IsProtected){
				state = JUMPING;
				
			}
			else {
				state = JUMPING;
				 }
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
			state = JUMPING;
			
		}
		else {
			state = JUMPING;
			
		}
		isJumping = true;
 		Engine::GetInstance().physics->ApplyLinearImpulseToCenter(pbody, 0.0f, -jumpForce, true);
		LOG("Player jumped action");
		secondJump = false;
		firstJump = false;
	}
	/*else if(spacePressed && canSecondJump){
		Engine::GetInstance().audio->PlayFx(jumpfx);
		if(IsProtected){
			currentAnimSet.SetCurrent("jump");
		}
		else {
			currentAnimSet.SetCurrent("jump");
		}
		Engine::GetInstance().physics->ApplyLinearImpulseToCenter(pbody, 0.0f, -jumpForce + 0.1f, true);
		secondJump = true;
	}*/
	
}

void Player::ApplyPhysics() {
	// Preserve vertical speed while jumping
	if (isJumping == true) {
		velocity.y = Engine::GetInstance().physics->GetYVelocity(pbody);

	}

	// Apply velocity via helper
	Engine::GetInstance().physics->SetLinearVelocity(pbody, velocity);
	UpdateAttackHitbox();
}

// =====================
// RENDER Y CAMARA
// =====================

void Player::Draw(float dt) {

	
	
	currentAnimSet->Update(dt);
	const SDL_Rect& animFrame = currentAnimSet->GetCurrentFrame();

	int x, y;
	pbody->GetPosition(x, y);

	position.setX((float)x);
	position.setY((float)y);

	SDL_FlipMode flip = facingLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

	int drawX = x - animFrame.w / 2;
	int drawY = y - animFrame.h / 2; 

	Engine::GetInstance().render->DrawTexture(
		texture,
		drawX,
		drawY,
		&animFrame,
		1.0f,
		0.0,
		INT_MAX,
		INT_MAX,
		flip
	);
	
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

void Player::OnCollision(PhysBody* physA, PhysBody* physB)
{
	ColliderType other = physB->ctype;

	// =========================
	// 1. ATAQUE DEL JUGADOR
	// =========================
	if (physA->ctype == ColliderType::PLAYERATTACK)
	{
		if (other == ColliderType::ENEMY)
		{
			if (!hitboxActive || hasHit) return;

			Enemy* e = static_cast<Enemy*>(physB->listener);
			if (e)
			{
				
				LOG("Enemy hit by player attack");
			}

			hasHit = true;
		}

		return; // importante: no seguir procesando como player normal
	}

	// =========================
	// 2. COLISIONES DEL PLAYER (CUERPO PRINCIPAL)
	// =========================
	switch (other)
	{
	case ColliderType::PLATFORM:
	{
		isJumping = false;
		firstJump = true;
		isCollidedFloor = true;
		break;
	}

	case ColliderType::PARED:
	{
		isCollidedWall = true;
		break;
	}

	case ColliderType::COIN:
	{
		Engine::GetInstance().audio->PlayFx(pickCoinFxId);
		physB->listener->Destroy();
		AddPoints(10);
		break;
	}

	case ColliderType::SAVE:
	{
		if (physB->objectName != "Player")
			Engine::GetInstance().audio->PlayFx(checkpointfx);

		if (pbody && !B2_IS_NULL(pbody->body))
			respawnPosition = b2Body_GetPosition(pbody->body);

		break;
	}

	case ColliderType::DANGER:
	case ColliderType::ENEMY:
	case ColliderType::FIREBALL:
	case ColliderType::FINALBOSS:
	{
		if (godMode) break;
		if (IsProtected) break;

		if (!isdead)
			Engine::GetInstance().audio->PlayFx(deathfx);

		isdead = true;
		lives--;

		if (lives <= 0)
		{
			isDeadDefinitive = true;
		}

		currentAnimSet->SetCurrent("jump");
		break;
	}

	case ColliderType::PROTECTION:
	{
		IsProtected = true;
		break;
	}

	case ColliderType::EXTRALIVE:
	{
		lives++;
		break;
	}

	default:
		break;
	}

	//LOG("Lives: %d", lives);
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
	currentAnimSet->SetCurrent("jump");
	
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

void Player::ChangeCurrentAnimation() {

	if (state == ATTACKING) return;
	if (state == lastState) return;

	lastState = state;

	switch (state)
	{
	case JUMPING:
		currentAnimSet = &anims2x3;
		texture = texture2x3;
		currentAnimSet->SetCurrent("jump");
		break;

	case RUNNING:
		currentAnimSet = &anims2x3;
		texture = texture2x3;
		currentAnimSet->SetCurrent("run");
		break;

	case ONCHEESE:
		currentAnimSet = &anims2x3;
		texture = texture2x3;
		currentAnimSet->SetCurrent("hoponcheese");
		break;

	default:
		break;
	}
}

void Player::Attack() {
	static bool wasPressedLastFrame = false;

	bool isPressed = Engine::GetInstance().input->GetMouseButtonDown(1);

	// Detectar flanco (click real)
	if (isPressed && !wasPressedLastFrame) {
		attackRequested = true;
		LOG("Attack requested (single click)");
	}

	wasPressedLastFrame = isPressed;
}

void Player::HandleAttack() {

	const float comboResetTimeMs = 100;
	static Uint32 lastAttackTime = 0;

	Uint32 now = SDL_GetTicks();

	// INPUT DETECTADO
	if (attackRequested)
	{
		attackRequested = false;

		if (!isAttacking)
		{
			attackCombo = 1;
			StartAttack(attackCombo);
			lastAttackTime = now;
		}
		else
		{
			// Guardamos el siguiente ataque en buffer
			bufferedAttack = true;
		}
	}

	// SI ESTAMOS ATACANDO
	if (isAttacking)
	{
		if (currentAnimSet->HasFinished())
		{
			lastAttackTime = now;
			hitboxActive = false;
			if (bufferedAttack)
			{
				bufferedAttack = false;

				attackCombo++;
				if (attackCombo > 3) attackCombo = 1;

				StartAttack(attackCombo);
				
			}
			else
			{
				isAttacking = false;
				state = DEFAULT;
			}
		}
	}

	// RESET DEL COMBO SI PASA MUCHO TIEMPO
	if (!isAttacking && attackCombo > 0)
	{
		if (now - lastAttackTime > comboResetTimeMs)
		{
			attackCombo = 0;
			state = DEFAULT;
			LOG("Combo reset (timeout)");
		}
	}
}

void Player::StartAttack(int combo)
{
	isAttacking = true;
	state = ATTACKING;

	currentAnimSet = &anims3x4;
	texture = texture3x4;

	switch (combo)
	{
	case 1:
		currentAnimSet->SetCurrent("attack1");
		currentAnimSet->Resets();
		break;
	case 2:
		currentAnimSet->SetCurrent("attack2");
		currentAnimSet->Resets();
		break;
	case 3:
		currentAnimSet->SetCurrent("attack3");
		currentAnimSet->Resets();
		break;
	}


	hasHit = false;
	hitboxActive = true;
	

	LOG("Player attack start combo %d", combo);
	LOG("Start attack combo %d", combo);
}

void Player::UpdateAttackHitbox()
{
	if (!attackHitbox) return;

	int x, y;
	pbody->GetPosition(x, y);

	int offsetX = facingLeft ? -offsetAttackHitboxX : offsetAttackHitboxX;

	attackHitbox->SetPosition(
		x + offsetX,
		y + offsetAttackHitboxY
	);
}

void Player::SpawnCheeseBall()
{
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_C) == KEY_DOWN) {
		auto entity =
			Engine::GetInstance().entityManager->CreateEntity(EntityType::CHEESEBALL);

		auto cb = std::static_pointer_cast<CheeseBall>(entity);

		Vector2D spawnPos = position;

		cb->SetPosition(spawnPos);
		cb->Start();
	}
}