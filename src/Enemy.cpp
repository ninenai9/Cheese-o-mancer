#include "Enemy.h"
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


Enemy::Enemy() : Entity(EntityType::ENEMY)
{
	name = "Enemy";
	pbody = nullptr;
}

Enemy::~Enemy() {

}

bool Enemy::Awake() {
	return true;
}

bool Enemy::Start() {

	// load
	enemyfx = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/PREV/enemigo_walk.wav");
	enemyDeathFx = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/PREV/enemy_death.wav");
	std::unordered_map<int, std::string> aliases = { {0,"idle"}, {3, "move"}};
	anims.LoadFromTSX("Assets/Textures/PREV/Enemy-Recovered.tsx", aliases);
	anims.SetCurrent("idle");

	//Initialize Player parameters
	texture = Engine::GetInstance().textures->Load("Assets/Textures/PREV/Enemy-Recovered.png");
	//Add physics to the enemy - initialize physics body
	texW = 32;
	texH = 32;

	if(pbody == nullptr) {
	position.setX(xInicial);
	position.setY(yInicial);

	pbody = Engine::GetInstance().physics->CreateCircle(
		(int)position.getX() + texW / 2,
		(int)position.getY() - texH / 2,
		texW / 2,
		bodyType::DYNAMIC
	);
	pbody->listener = this;
	pbody->ctype = ColliderType::ENEMY;


	
		
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

bool Enemy::Update(float dt)
{
	if (!Engine::GetInstance().render->IsOnScreenWorldRect(position.getX(), position.getY(), (float)texW, (float)texH, 0))
		return true;
	if(toDelete) {
		return true;
	}
	bool isPaused = Engine::GetInstance().scene->isPaused;
	if(Player::isPlayerProtectedquestion()){
		
	}
	else {
	PerformPathfinding();
	
	}
	if(!isPaused) {
	GetPhysicsValues();
	Move();
	ApplyPhysics();
	
	}
	Draw(dt);
	/*pathfinding->DrawPath();*/
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F9) == KEY_DOWN)
		debug = !debug;
	if (debug) { pathfinding->DrawPath(); }
	
	return true;
}

void Enemy::PerformPathfinding()
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
		if(dy<-64.0f){
			pathfinding->pathTiles.clear();
			isFollowing = false;
			return;
		}

		int i = 100;//maximo de 100 iteraciones por frame
		int aux = 0;
		while (pathfinding->pathTiles.empty() && aux<i) {
		
		pathfinding->PropagateAStar(SQUARED);
	aux++;
		
		isFollowing = true;
		 if (pathfinding->frontierAStar.empty()) break;
		
		}
	
	
		
	
}


void Enemy::GetPhysicsValues() {
	velocity = Engine::GetInstance().physics->GetLinearVelocity(pbody);

	
	velocity.y = Engine::GetInstance().physics->GetLinearVelocity(pbody).y;
}

void Enemy::Move(){

	
	if (pathfinding->pathTiles.empty()) {
		anims.SetCurrent("idle");
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
	

	anims.SetCurrent("move");
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
void Enemy::Jump() {
	
			Engine::GetInstance().physics->ApplyLinearImpulseToCenter(pbody, 0.0f, -1.3f, true);
	
}
void Enemy::ApplyPhysics() {

	Engine::GetInstance().physics->SetLinearVelocity(pbody, velocity);
}

void Enemy::Draw(float dt) {

	anims.Update(dt);
	const SDL_Rect& animFrame = anims.GetCurrentFrame();

	int x, y;
	pbody->GetPosition(x, y);
	position.setX((float)x);
	position.setY((float)y);
	SDL_FlipMode flip = facingRight ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
	Engine::GetInstance().render->DrawTexture(texture, x - texW / 2, y - texH / 2, &animFrame,1.0f,0.0,INT_MAX,INT_MAX,flip);
}

bool Enemy::CleanUp()
{
	LOG("Cleanup enemy");
	Engine::GetInstance().textures->UnLoad(texture);
	if(pbody != nullptr) {
		Engine::GetInstance().physics->DeletePhysBody(pbody);
		pbody = nullptr;
	}
	return true;
}
void Enemy::Reset() {

	pbody->SetPosition(xInicial, yInicial);
	pathfinding->pathTiles.clear();
	isFollowing = false;
	isCollidedFloor = false;
	isCollidedWall = false;
	Engine::GetInstance().physics->SetLinearVelocity(pbody, { 0, 0 });
	pathfinding->ResetPath(Engine::GetInstance().map->WorldToMap(xInicial, yInicial));

}
void Enemy::SetPosition(Vector2D pos) {
	position = pos;
}

Vector2D Enemy::GetPosition() {
	int x, y;
	pbody->GetPosition(x, y);
	// Adjust for center
	return Vector2D((float)x-texW/2,(float)y-texH/2);
}

//Define OnCollision function for the enemy. 
void Enemy::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		LOG("Collision PLATFORM");
		isCollidedFloor = true;
		break;

	case ColliderType::PARED:
		LOG("Collision PARED enemigo");
		isCollidedWall = true;
		if (isFollowing ) {
			Jump();
			LOG("JUMP ENEMY");
		}
		break;

	case ColliderType::FIREBALL:
		LOG("Enemy hit by FIREBALL — dying!");
		Engine::GetInstance().audio->PlayFx(enemyDeathFx);   
		if (!toDelete) {
			Engine::GetInstance().map->killedEnemies.push_back(mapID);
		}
		Player::AddPoints(100);

		toDelete = true;
		if (pbody) {
			Engine::GetInstance().physics->DeletePhysBody(pbody);
			pbody = nullptr;
			LOG("Enemy physics body deleted upon fireball collision.");
		}
		break;
	case ColliderType::DANGER:
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
	case ColliderType::PLAYER:
		LOG("Collision PLAYER");
		Reset();
		break;

	default:
		break;
	}
}

void Enemy::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
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
