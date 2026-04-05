#include "FINALBOSS.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "EntityManager.h"
#include "Map.h"
#include "FireBall.h"


FinalBoss::FinalBoss() : Entity(EntityType::FINALBOSS)
{
	name = "Enemy";
	pbody = nullptr;
	speed = 1.0f;
	isdead = false;
}

FinalBoss::~FinalBoss() {

}

bool FinalBoss::Awake() {
	return true;
}
void FinalBoss::ThrowFireBall(Side side) {

	std::shared_ptr<FireBall> fireball = std::dynamic_pointer_cast<FireBall>(Engine::GetInstance().entityManager->CreateEntity(EntityType::FIREBALL));
	fireball->spawnPos = Vector2D(position.getX(), position.getY());
	fireball->spawnSide = side;
	fireball->AssignOwner(EntityType::FINALBOSS);

	fireball->Start();
	fireballs.push_back(fireball);
}
void FinalBoss::Attack() {
	if (!phase) {
		if (facingRight) {

			ThrowFireBall(Side::RIGHT);
			LOG("Created Fireball");

		}
		else {

			ThrowFireBall(Side::LEFT);
			LOG("Created Fireball");
		}
	
	}else  {


		//Throws in every direction
		ThrowFireBall(Side::RIGHT);
		ThrowFireBall(Side::LEFT);
		ThrowFireBall(Side::UP);
		ThrowFireBall(Side::DOWN);
		ThrowFireBall(Side::DIAGONALUL);
		ThrowFireBall(Side::DIAGONALUR);
		ThrowFireBall(Side::DIAGONALDL);
		ThrowFireBall(Side::DIAGONALDR);
		LOG("Created multiple Fireballs");
		
	}
}
bool FinalBoss::Start() {

	// load
	enemyfx = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/PREV/enemigo_walk.wav");
	enemyDeathFx = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/PREV/enemy_death.wav");
	std::unordered_map<int, std::string> aliases = { {2, "move"}, {5, "move_second"}, {8,"idle"}, {11,"idle_second"}, {12,"attack"}, {15,"attack_second"}, };
	anims.LoadFromTSX("Assets/Textures/PREV/FinalBoss.tsx", aliases);
	anims.SetCurrent("idle");

	//Initialize Player parameters
	texture = Engine::GetInstance().textures->Load("Assets/Textures/PREV/FinalBoss.png");
	//Add physics to the enemy - initialize physics body
	texW = 32;
	texH = 50;

	if(pbody == nullptr) {
	position.setX(xInicial);
	position.setY(yInicial);

	pbody = Engine::GetInstance().physics->CreateCircle(
		(int)position.getX() + texW / 2,
		(int)position.getY() + texH / 2,
		texW / 2,
		bodyType::DYNAMIC
	);
	pbody->listener = this;
	pbody->ctype = ColliderType::FINALBOSS;


	
		
	}
	//ssign collider type

	// Initialize pathfinding
	pathfinding = std::make_shared<Pathfinding>();
	//Get the position of the enemy
	Vector2D pos = GetPosition();
	//Convert to tile coordinates
	Vector2D tilePos = Engine::GetInstance().map->WorldToMap((int)pos.getX(), (int)pos.getY()+1);
	//Reset pathfinding
	pathfinding->ResetPath(tilePos);
	

	return true;
}

bool FinalBoss::Update(float dt)

{
	bool isPaused = Engine::GetInstance().scene->isPaused;
	if (isdead) {
	
		if (pbody) {
			Engine::GetInstance().physics->DeletePhysBody(pbody);
			pbody = nullptr;
			LOG("Enemy physics body deleted upon fireball collision.");
		}
		return true;
	
	}
	if (!Engine::GetInstance().render->IsOnScreenWorldRect(position.getX(), position.getY(), (float)texW, (float)texH, 0))
		return true;

	if (!isPaused) {
	
	PerformPathfinding();
	GetPhysicsValues();
	Move();
	ApplyPhysics();
	
	
	}
	if(isPaused){
		
			if (pbody)
				Engine::GetInstance().physics->SetLinearVelocity(pbody, { 0.0f, 0.0f });

			Draw(dt);
			return true;
		
	}
	Draw(dt);
	/*pathfinding->DrawPath();*/
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F9) == KEY_DOWN)
		debug = !debug;
	if (debug) { pathfinding->DrawPath(); }
	if (!isPaused) {



	if (StartedAttackTime != 0.0f && !hasAttacked) {
		Attack();
		hasAttacked = true;
	}

	}
	
	return true;
}

void FinalBoss::PerformPathfinding()
{
	
		Vector2D playerPos = Engine::GetInstance().scene->GetPlayerPosition();
		
		Vector2D enemyPos = GetPosition();
		

		Vector2D playerTilePos = Engine::GetInstance().map->WorldToMap(playerPos.getX(), playerPos.getY());
		Vector2D enemyTilePos = Engine::GetInstance().map->WorldToMap(enemyPos.getX()+5, enemyPos.getY()+5);

		pathfinding->ResetPath(enemyTilePos);


		float dx = playerPos.getX() - enemyPos.getX();
		float dy = playerPos.getY() - enemyPos.getY();
		float dist = sqrt(dx * dx + dy * dy);

		if (dist > 200.0f)
		{
			
			
			pathfinding->pathTiles.clear();   
			isFollowing = false;
			return;
			
			
			
		}
		if (dist < 300) {

			PlayerClose = true;
		}
		else { PlayerClose = false; }

	
		while (pathfinding->pathTiles.empty()) {
		
		pathfinding->PropagateAStar(SQUARED);
		
		isFollowing = true;
		if (pathfinding->frontierAStar.empty()) break;
		}
	
	
		
	
}


void FinalBoss::GetPhysicsValues() {
	velocity = Engine::GetInstance().physics->GetLinearVelocity(pbody);

	
	velocity.y = Engine::GetInstance().physics->GetLinearVelocity(pbody).y;
}

void FinalBoss::Move(){

	if(StartedAttackTime != 0.0f) {
		int currentTime = (int)SDL_GetTicks();
		if(currentTime - (int)StartedAttackTime < 500) {
			if (!phase) {
			
			anims.SetCurrent("attack");
			}
			else {
				anims.SetCurrent("attack_second");
			}
			velocity.x = 0;
			return;
		}
		else {
			StartedAttackTime = 0.0f;
			closeEnough = false;
			hasAttacked = false;
		}
		return;
	}
	else {
	
		Vector2D playerPos = Engine::GetInstance().scene->GetPlayerPosition();
		float dxp = playerPos.getX() - position.getX();
		float dyp = playerPos.getY() - position.getY();
		float playerDist = sqrt(dxp * dxp + dyp * dyp);

		// START ATTACK IF CLOSE
		if (playerDist < 100.0f && StartedAttackTime == 0.0f) {
			StartedAttackTime = SDL_GetTicks();
			hasAttacked = false;
			return;
		}

		if (pathfinding->pathTiles.empty()) {
			if (!phase) {
				anims.SetCurrent("idle");
			}
			else {
				anims.SetCurrent("idle_second");
			}
			return;
		}

		Vector2D nextTile = pathfinding->pathTiles.front();
		Vector2D nextWorld = Engine::GetInstance().map->MapToWorld(nextTile.getX(), nextTile.getY());

		if (nextWorld.getX() > position.getX()) {
			velocity.x = speed;
			facingRight = true;
		}
		else if (nextWorld.getX() < position.getX()) {
			velocity.x = -speed;
			facingRight = false;
		}
		else {
			velocity.x = 0;
		}




		float threshold = 4.0f;
		float dx = nextWorld.getX() - position.getX();
		float dy = nextWorld.getY() - position.getY();
		float dist = sqrt(dx * dx + dy * dy);

		if (dist < threshold)
		{
			pathfinding->pathTiles.pop_front(); //cuando llega ahí lo elimino
		}
	
		

		if (!phase) {
		
		anims.SetCurrent("move");
		
		}else {
			anims.SetCurrent("move_second");
		}
		if (isFollowing) {
			int currentTime = (int)SDL_GetTicks();
			if (currentTime - lastStepTime > 350) {
				Engine::GetInstance().audio->PlayFx(enemyfx);
				lastStepTime = currentTime;
			}
		}
		else {
			lastStepTime = 0;
		}

	}
	
}
void FinalBoss::Jump() {
	
			Engine::GetInstance().physics->ApplyLinearImpulseToCenter(pbody, 0.0f, -1.3f, true);
	
}
void FinalBoss::ApplyPhysics() {

	Engine::GetInstance().physics->SetLinearVelocity(pbody, velocity);
}

void FinalBoss::Draw(float dt) {

	anims.Update(dt);
	const SDL_Rect& animFrame = anims.GetCurrentFrame();

	int x, y;
	pbody->GetPosition(x, y);
	position.setX((float)x);
	position.setY((float)y);
	SDL_FlipMode flip = facingRight ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
	Engine::GetInstance().render->DrawTexture(texture, x - texW / 2, y - texH / 2, &animFrame,1.0f,0.0,INT_MAX,INT_MAX,flip);
}

bool FinalBoss::CleanUp()
{
	LOG("Cleanup enemy");
	Engine::GetInstance().textures->UnLoad(texture);
	if(pbody != nullptr) {
		Engine::GetInstance().physics->DeletePhysBody(pbody);
		pbody = nullptr;
	}
	return true;
}
void FinalBoss::Reset() {

	pbody->SetPosition(xInicial, yInicial);
	pathfinding->pathTiles.clear();
	isFollowing = false;
	isCollidedFloor = false;
	isCollidedWall = false;
	Engine::GetInstance().physics->SetLinearVelocity(pbody, { 0, 0 });
	pathfinding->ResetPath(Engine::GetInstance().map->WorldToMap(xInicial, yInicial));

}
void FinalBoss::SetPosition(Vector2D pos) {
	position = pos;
}

Vector2D FinalBoss::GetPosition() {
	int x, y;
	pbody->GetPosition(x, y);
	// Adjust for center
	return Vector2D((float)x-texW/2,(float)y-texH/2);
}

//Define OnCollision function for the enemy. 
void FinalBoss::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM: {
		LOG("Collision PLATFORM");
		isCollidedFloor = true;
		break;
	}
	case ColliderType::PARED: {
		LOG("Collision PARED enemigo");
		isCollidedWall = true;
		if (isFollowing) {
			Jump();
			LOG("JUMP ENEMY");
		}
		break;
	}
	case ColliderType::FIREBALL: {
		FireBall* fb = dynamic_cast<FireBall*>(physB->listener);
		if (fb == nullptr) {
			LOG("Fireball listener is NULL!");
			break;
		}
		if (!fb) break;
		EntityType owner = fb->CheckOwner();
		LOG("Enemy hit by FIREBALL — dying!");
		
		if (lives < 20 && !phase) {
			SecondPhase();
			lives--;

			LOG("Phase change");

		}
		else if (lives < 0) {
			Engine::GetInstance().audio->PlayFx(enemyDeathFx);
			if (!toDelete) {
				Engine::GetInstance().map->killedEnemies.push_back(mapID);
			}
			Player::AddPoints(500);
			/*toDelete = true;*/
			
			isdead = true;
		}
		else {

			lives--;
			LOG("Final boss lives: %i", lives);
		}


		break;
	}
	case ColliderType::DANGER: {
		LOG("Enemy hit by SPIKES — dying!");
		Engine::GetInstance().audio->PlayFx(enemyDeathFx);
		if (!toDelete) {
			Engine::GetInstance().map->killedEnemies.push_back(mapID);
		}

		toDelete = true;
		if (pbody) {
			Engine::GetInstance().physics->DeletePhysBody(pbody);
			pbody = nullptr;
			LOG("Enemy physics body deleted upon fireball collision.");
		}
		break;
	}
	case ColliderType::PLAYER: {
		LOG("Collision PLAYER");

		break;
	}
	default:
		break;
	}
}

void FinalBoss::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		LOG("End Collision PLATFORM");
		isCollidedFloor = false;
		break;

	case ColliderType::PARED:
		LOG("End Collision PARED");
		isCollidedWall = false;
		break;
	default:
		break;
	}
}

void FinalBoss :: SecondPhase() {
	speed += 2.0f;
	phase = true;
}