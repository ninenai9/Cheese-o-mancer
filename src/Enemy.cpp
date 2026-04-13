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
}

Enemy::~Enemy() {

}

bool Enemy::Awake() {
	return true;
}

bool Enemy::Start() {

	// load
	std::unordered_map<int, std::string> aliases = { {0,"idle"} };
	/*anims.LoadFromTSX("Assets/Textures/enemy_Spritesheet.tsx", aliases);
	anims.SetCurrent("idle");*/

	//Initialize Player parameters
	texture = Engine::GetInstance().textures->Load("Assets/Textures/placeholder_Jester.png");

	//Add physics to the enemy - initialize physics body
	texW = 215;
	texH = 384;
	pbody = Engine::GetInstance().physics->CreateRectangle(position.getX(), position.getY(), texW, texH, bodyType::DYNAMIC);

	//Assign enemy class (using "this") to the listener of the pbody. This makes the Physics module to call the OnCollision method
	pbody->listener = this;

	//ssign collider type
	pbody->ctype = ColliderType::ENEMY;

	// Initialize pathfinding
	pathfinding = std::make_shared<Pathfinding>();
	//Get the position of the enemy
	Vector2D pos = GetPosition();
	//Convert to tile coordinates
	Vector2D tilePos = Engine::GetInstance().map->WorldToMap((int)pos.getX(), (int)pos.getY() + 1);
	//Reset pathfinding
	pathfinding->ResetPath(tilePos);

	return true;
}

bool Enemy::Update(float dt)
{
	repathTimer++;

	GetPhysicsValues();
	// Solo actuamos si el jugador está en el rango
	if (CalculateDistance()) {

		Vector2D currentPlayerTile = Engine::GetInstance().scene->GetPlayerPosition();

		if ((currentPlayerTile.getX() != lastPlayerTile.getX() ||
			currentPlayerTile.getY() != lastPlayerTile.getY()) && repathTimer >= repathDelay) {

			int ex, ey;
			pbody->GetPosition(ex, ey);
			Vector2D enemyTilePos = Engine::GetInstance().map->WorldToMap(ex, ey);

			pathfinding->ResetPath(enemyTilePos);
			
			repathTimer = 0;
			lastPlayerTile = currentPlayerTile;
		}

		pathfinding->PropagateAStar(SQUARED);
		// 2. Ejecutar el movimiento
		Move();

	}
	else {
		// Si el jugador sale del rango, el enemigo se detiene
		int ex, ey;
		pbody->GetPosition(ex, ey);
		Vector2D enemyTilePos = Engine::GetInstance().map->WorldToMap(ex, ey);
		pathfinding->ResetPath(enemyTilePos);
	}

	
	
	ApplyPhysics();
	Draw(dt);

	return true;
}

void Enemy::PerformPathfinding() {

	// Pathfinding testing inputs

	// Reset pathfinding with R key
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_R) == KEY_DOWN) {
		//Get the position of the enemy
		Vector2D pos = GetPosition();
		//Convert to tile coordinates
		Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap((int)pos.getX(), (int)pos.getY() + 1);
		//Reset pathfinding
		pathfinding->ResetPath(tilePos);
	}

	// Propagate BFS with J key
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_J) == KEY_DOWN) {
		pathfinding->PropagateBFS();
	}

	// Propagate BFS continuously with LShift + J
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_J) == KEY_REPEAT &&
		Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) {
		pathfinding->PropagateBFS();
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_K) == KEY_DOWN) {
		pathfinding->PropagateDijkstra();
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_K) == KEY_REPEAT &&
		Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) {
		pathfinding->PropagateDijkstra();
	}

	// L13: TODO 3:	Add the key inputs to propagate the A* algorithm with different heuristics (Manhattan, Euclidean, Squared)

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_B) == KEY_DOWN) {
		pathfinding->PropagateAStar(MANHATTAN);
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_B) == KEY_REPEAT &&
		Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) {
		pathfinding->PropagateAStar(MANHATTAN);
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_N) == KEY_DOWN) {
		pathfinding->PropagateAStar(EUCLIDEAN);
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_N) == KEY_REPEAT &&
		Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) {
		pathfinding->PropagateAStar(EUCLIDEAN);
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_M) == KEY_DOWN) {
		pathfinding->PropagateAStar(SQUARED);
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_M) == KEY_REPEAT &&
		Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) {
		pathfinding->PropagateAStar(SQUARED);
	}

}

void Enemy::GetPhysicsValues() {
	// Read current velocity
	velocity = Engine::GetInstance().physics->GetLinearVelocity(pbody);
	
}

void Enemy::Move() {

	// Si no hay camino o ya llegamos, detenemos el movimiento horizontal
	if (pathfinding->pathTiles.empty()) {

		return;
	}

	// Obtenemos el siguiente paso del path (el último elemento añadido a la lista)
	Vector2D nextTile = pathfinding->pathTiles.front();
	Vector2D nextPosWorld = Engine::GetInstance().map->MapToWorld((int)nextTile.getX(), (int)nextTile.getY());

	// Posición actual del enemigo
	int ex, ey;
	pbody->GetPosition(ex, ey);

	float speed = 5.0f; // Ajusta según la escala de tu mundo Box2D

	// Lógica de movimiento horizontal hacia el siguiente tile
	float threshold = 5.0f;

	if (ex < nextPosWorld.getX() - threshold)
		velocity.x = speed;
	else if (ex > nextPosWorld.getX() + threshold)
		velocity.x = -speed;
	else
		velocity.x = 0;
	// Move 
}

void Enemy::ApplyPhysics() {

	// Apply velocity via helper
	Engine::GetInstance().physics->SetLinearVelocity(pbody, velocity);
}

void Enemy::Draw(float dt) {

	//anims.Update(dt);
	//const SDL_Rect& animFrame = anims.GetCurrentFrame();

	//// Update render position using your PhysBody helper
	int x, y;
	pbody->GetPosition(x, y);
	position.setX((float)x);
	position.setY((float)y);

	// Draw pathfinding debug
	pathfinding->DrawPath();

	//Draw the player using the texture and the current animation frame
	Engine::GetInstance().render->DrawTexture(texture, x - texW / 2, y - texH / 2, 0, 1, 0, 0, 0, SDL_FLIP_NONE);
}

bool Enemy::CleanUp()
{
	LOG("Cleanup enemy");
	Engine::GetInstance().textures->UnLoad(texture);
	return true;
}

void Enemy::SetPosition(Vector2D pos) {
	pbody->SetPosition((int)(pos.getX()), (int)(pos.getY()));
}

Vector2D Enemy::GetPosition() {
	int x, y;
	pbody->GetPosition(x, y);
	// Adjust for center
	return Vector2D((float)x - texW / 2, (float)y - texH / 2);
}

//Define OnCollision function for the enemy. 
void Enemy::OnCollision(PhysBody* physA, PhysBody* physB) {

}

void Enemy::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{

}

bool Enemy::CalculateDistance() {
	// 1. Posición del Enemigo (Píxeles)
    int ex, ey;
    pbody->GetPosition(ex, ey); 
	Vector2D enemyWorld = Engine::GetInstance().map->WorldToMap(ex, ey);
    // 2. Posición del Jugador (Tiles -> Convertir a Píxeles)
    // ¡OJO! Asegúrate de que GetPlayerPosition() devuelve coordenadas de TILE (ej. 15, 10)
    Vector2D pTile = Engine::GetInstance().scene->GetPlayerPosition();
    Vector2D pWorld = Engine::GetInstance().map->WorldToMap((int)pTile.getX(), (int)pTile.getY());

    // 3. LOG DE CONTROL: Mira esto en la consola para ver cuál de los dos falla
    // printf("DEBUG: Enemigo en (%d, %d) | Jugador en (%f, %f)\n", ex, ey, pWorld.getX(), pWorld.getY());

    // 4. Calcular diferencia
    float dx = (float)enemyWorld.getX() - pWorld.getX();
    float dy = (float)enemyWorld.getY() - pWorld.getY();

    // 5. Pitágoras
    float distance = sqrtf((dx * dx) + (dy * dy));

    // Si el número sigue siendo 900000, es que ex/ey o pWorld.getX() valen algo absurdo
	printf("Player raw: (%f, %f)\n", pTile.getX(), pTile.getY());
	printf("Enemy raw: (%d, %d)\n", ex, ey);
	printf("Enemy world: (%f, %f)\n", enemyWorld.getX(), enemyWorld.getY());
	printf("Player world: (%f, %f)\n", pWorld.getX(), pWorld.getY());

    float detectionRange = 400.0f;
    return (distance < detectionRange);
}