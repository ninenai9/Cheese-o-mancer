#include "EntityManager.h"
#include "Player.h"
#include "NPC.h"
#include "Handman.h"
#include "Engine.h"
#include "Textures.h"
#include "Scene.h"
#include "Log.h"
#include "Coins.h"
#include "Item.h"
#include "Enemy.h"
#include "EnemigoVolador.h"
#include "Checkpoint.h"
#include "Protection.h"
#include "ExtraLive.h"
#include "FinalBoss.h"
#include "Verdugo.h"
#include "Rat.h"

EntityManager::EntityManager() : Module()
{
	name = "entitymanager";
}

// Destructor
EntityManager::~EntityManager()
{}

// Called before render is available
bool EntityManager::Awake()
{
	LOG("Loading Entity Manager");
	bool ret = true;

	//Iterates over the entities and calls the Awake
	for(const auto entity : entities)
	{
		if (entity->active == false) continue;
		ret = entity->Awake();
	}

	return ret;

}

bool EntityManager::Start() {

	bool ret = true; 

	//Iterates over the entities and calls Start
	for(const auto entity : entities)
	{
		if (entity->active == false) continue;
		ret = entity->Start();
	}

	return ret;
}

// Called before quitting
bool EntityManager::CleanUp()
{
	bool ret = true;

	for(const auto entity : entities)
	{
		if (entity->active == false) continue;
		ret = entity->CleanUp();
	}

	entities.clear();

	return ret;
}

std::shared_ptr<Entity> EntityManager::CreateEntity(EntityType type)
{
	std::shared_ptr<Entity> entity;

	//L04: TODO 3a: Instantiate entity according to the type and add the new entity to the list of Entities
	switch (type)
	{
	case EntityType::PLAYER:
		entity = std::make_shared<Player>();
		break;
	case EntityType::FIREBALL:
		entity = std::make_shared<FireBall>();
		break;
	case EntityType::ITEM:
		entity = std::make_shared<Item>();
		break;
	case EntityType::COIN:
		entity = std::make_shared<Coins>();
		break;
	case EntityType::ENEMY:
		entity = std::make_shared<Enemy>();
		break;
	case EntityType::ENEMYFLYING:
		entity = std::make_shared<EnemigoVolador>();
		break;
	case EntityType::CHECKPOINT:
		entity = std::make_shared<Checkpoint>();
		break;
	case EntityType::PROTECTION:
		entity = std::make_shared<Protection>();
		break;
	case EntityType::EXTRALIVE:
		entity = std::make_shared<ExtraLive>();
		break;
	case EntityType::FINALBOSS:
		entity =  std::make_shared<FinalBoss>();
		break;
	case EntityType::VERDUGO:
		entity = std::make_shared<Verdugo>();
		break;
	case EntityType::RAT:
		entity = std::make_shared<Rat>();
		break;
	case EntityType::HANDMAN:
		entity = std::make_shared<HANDMAN>();
		break;
	case EntityType::NPC:
		entity = std::make_shared<NPC>();
		break;
	default:
		break;
	}

	entities.push_back(entity);

	return entity;
}

void EntityManager::DestroyEntity(std::shared_ptr<Entity> entity)
{
	entity->CleanUp();
	entities.remove(entity);
}

void EntityManager::AddEntity(std::shared_ptr<Entity> entity)
{
	if ( entity != nullptr) entities.push_back(entity);
}

bool EntityManager::Update(float dt)
{
	bool ret = true;
	for(const auto entity : entities)
	{
		if (entity->active == false) continue;
		ret = entity->Update(dt);
		
	}
	for (auto it = entities.begin(); it != entities.end(); )
	{
		if ((*it)->toDelete)
		{
			(*it)->CleanUp();
			it = entities.erase(it);
		}
		else
		{
			++it;
		}
	}
	return ret;
}