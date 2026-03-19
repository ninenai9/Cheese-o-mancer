#include "EnemigoVolador.h"
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

EnemigoVolador::EnemigoVolador() : Entity(EntityType::ENEMYFLYING){
    name = "EnemigoVolador";
    pbody = nullptr;
}

EnemigoVolador::~EnemigoVolador() {

}

bool EnemigoVolador::Awake() {
    return true;
}

bool EnemigoVolador::Start() {

    // load
    std::unordered_map<int, std::string> aliases = { {0,"idle"} };
    anims.LoadFromTSX("Assets/Textures/FuegoEnemigo.tsx", aliases);
    anims.SetCurrent("idle");
    enemyDeathFx = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/enemy_death.wav");
    //Initialize Enemy parameters
    texture = Engine::GetInstance().textures->Load("Assets/Textures/FuegoEnemigo.png");

    //Add physics - initialize physics body
    texW = 32;
    texH = 32;
    if (pbody == nullptr) {


    pbody = Engine::GetInstance().physics->CreateCircle(
        (int)position.getX() + texW / 2,
        (int)position.getY() + texH / 2,
        texW / 2,
        bodyType::DYNAMIC
    );
    b2Body_SetGravityScale(pbody->body, 0.0f);
	

    //Assign this class as the listener
    pbody->listener = this;

    //Assign collider type
    pbody->ctype = ColliderType::ENEMY;


    }
    // Initialize pathfinding
    pathfinding = std::make_shared<Pathfinding>();

    Vector2D pos = GetPosition();
    Vector2D tilePos = Engine::GetInstance().map->WorldToMap(
        (int)pos.getX(),
        (int)pos.getY() + 1
    );

    pathfinding->ResetPath(tilePos);

    return true;
}

bool EnemigoVolador::Update(float dt)
{
    if (toDelete)
        return true;
    bool isPaused = Engine::GetInstance().scene->isPaused;
    if (Player::isPlayerProtectedquestion()) {

    }
    else {
        PerformPathfinding();

    }
    if (!isPaused) {
    GetPhysicsValues();
    Move();
    ApplyPhysics();
    
    
    }
    Draw(dt);
    if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F9) == KEY_DOWN)
        debug = !debug;
    if (debug) { pathfinding->DrawPath(); }

    return true;
}

void EnemigoVolador::PerformPathfinding()
{
    Vector2D playerPos = Engine::GetInstance().scene->GetPlayerPosition();

    Vector2D enemyPos = this->position;


    Vector2D playerTilePos = Engine::GetInstance().map->WorldToMap(playerPos.getX() , playerPos.getY() );
    Vector2D enemyTilePos = Engine::GetInstance().map->WorldToMap(enemyPos.getX() , enemyPos.getY() );

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


    int i = 100;//maximo de 100 iteraciones por frame
    int aux = 0;
    while (pathfinding->pathTiles.empty() && aux < i) {

        pathfinding->PropagateAStar(SQUARED);
        aux++;

        isFollowing = true;
        if (pathfinding->frontierAStar.empty()) break;

    }
}

void EnemigoVolador::GetPhysicsValues() {
    velocity = Engine::GetInstance().physics->GetLinearVelocity(pbody);
    velocity.y = Engine::GetInstance().physics->GetLinearVelocity(pbody).y;
}

void EnemigoVolador::Move() {

    if (pathfinding->pathTiles.empty()) {
        velocity.x = 0;
        velocity.y = 0;
        anims.SetCurrent("idle");
        return;
    }

    Vector2D nextTile = pathfinding->pathTiles.front();
    Vector2D nextWorld = Engine::GetInstance().map->MapToWorld(nextTile.getX(), nextTile.getY());

    Vector2D playerPos = Engine::GetInstance().scene->GetPlayerPosition();

    
  //  if (nextWorld.getX() > position.getX()) velocity.x = speed;
  //  else if (nextWorld.getX() < position.getX()) velocity.x = -speed;
  //  else velocity.x = 0;

  //  if (isCollidedWall) {
  //  //si se colisiona contra una pared, lo que hace es que sigue el pathfinding, en vez de quedarse atascado
  //      if (nextWorld.getY() > position.getY()) velocity.y = speed;
		//else if (nextWorld.getY() < position.getY()) velocity.y = -speed;
  //  
  //  }
  //  else {
  //  
  //  float dy = playerPos.getY() - position.getY();
  //  velocity.y = dy * 0.1f; 
  //  
  //  }
    Vector2D dir = nextWorld - position;

    float length = sqrt(dir.getX() * dir.getX() + dir.getY() * dir.getY());
    if (length != 0) {
        dir.setX(dir.getX() / length);
        dir.setY(dir.getY() / length);
    }

    velocity.x = dir.getX() * speed;
    velocity.y = dir.getY() * speed;

    float threshold = 4.0f;
    float dx = nextWorld.getX() - position.getX();
    if (abs(dx) < threshold) {
        pathfinding->pathTiles.pop_front();
    }

    anims.SetCurrent("idle");
}




void EnemigoVolador::ApplyPhysics() {
    Engine::GetInstance().physics->SetLinearVelocity(pbody, velocity);
}

void EnemigoVolador::Draw(float dt) {

    anims.Update(dt);
    const SDL_Rect& animFrame = anims.GetCurrentFrame();

    int x, y;
    pbody->GetPosition(x, y);
    position.setX((float)x);
    position.setY((float)y);


    Engine::GetInstance().render->DrawTexture(texture, x - texW / 2, y - texH / 2, &animFrame);
}

bool EnemigoVolador::CleanUp()
{
    LOG("Cleanup EnemigoVolador");
    Engine::GetInstance().textures->UnLoad(texture);
    if (pbody != nullptr) {
        Engine::GetInstance().physics->DeletePhysBody(pbody);
        pbody = nullptr;
    }
    return true;
}

void EnemigoVolador::Reset() {
   
    pbody->SetPosition(xInicial, yInicial);
    pathfinding->pathTiles.clear();
    isFollowing = false;
    isCollidedFloor = false;
    isCollidedWall = false;
    Engine::GetInstance().physics->SetLinearVelocity(pbody, { 0, 0 });
    pathfinding->ResetPath(Engine::GetInstance().map->WorldToMap(xInicial, yInicial));
}

void EnemigoVolador::SetPosition(Vector2D pos) {
 
        this->position = pos;
        if (pbody)
            pbody->SetPosition((int)pos.getX() + texW / 2, (int)pos.getY() + texH / 2);
    
}

Vector2D EnemigoVolador::GetPosition() {
    int x, y;
    pbody->GetPosition(x, y);
    return Vector2D((float)x - texW / 2, (float)y - texH / 2);
}

//Define OnCollision function for the enemy. 
void EnemigoVolador::OnCollision(PhysBody* physA, PhysBody* physB) {
    switch (physB->ctype)
    {
    case ColliderType::PLATFORM:
        LOG("Collision PLATFORM");
        isCollidedFloor = true;
        break;

    case ColliderType::PARED:
        LOG("Collision PARED enemigo");
        isCollidedWall = true;
       
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

void EnemigoVolador::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
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
