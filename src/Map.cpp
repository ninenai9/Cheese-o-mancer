
#include "Engine.h"
#include "Render.h"
#include "Textures.h"
#include "Map.h"
#include "Log.h"
#include "Physics.h"
#include "EntityManager.h"
#include "Enemy.h"          
#include "EnemigoVolador.h"
#include <math.h>
#include "Audio.h"
#include "Input.h"
#include "Checkpoint.h"
#include "coins.h"
#include "FinalBoss.h"
#include "Protection.h"
#include "ExtraLive.h"
#include "Scene.h"

Map::Map() : Module(), mapLoaded(false)
{
    name = "map";
}

// Destructor
Map::~Map()
{}

// Called before render is available
bool Map::Awake()
{
    name = "map";
    LOG("Loading Map Parser");

    return true;
}

bool Map::Start() {
    killedEnemies.clear();
    return true;

	/*int checkpointfx = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/checkpoint.wav");*/
}

bool Map::Update(float dt)
{
    bool ret = true;
    if (mapLoaded) {
        // L07 TODO 5: Prepare the loop to draw all tiles in a layer + DrawTexture()
        // iterate all tiles in a layer
        for (const auto& mapLayer : mapData.layers) {
            if (mapLayer->name == "Map") {
                continue;
            }
            //L09 TODO 7: Check if the property Draw exist get the value, if it's true draw the lawyer
            if (mapLayer->properties.GetProperty("Draw") != NULL && mapLayer->properties.GetProperty("Draw")->value == true) {
                // L19 TODO 3: Compute which tiles of the map are visible inside the camera view
                Vector2D camPosTile = GetCameraPositionInTiles();
                Vector2D limits = GetCameraLimitsInTiles(camPosTile);
                
                // L19 TODO 3: Update the loop to draw only the tiles in the camera view
                for (int i = camPosTile.getX(); i < limits.getX(); i++) {
                    for (int j = camPosTile.getY(); j < limits.getY(); j++) {
						// L07 TODO 9: Complete the draw function
                        //Get the gid from tile
                        int gid = mapLayer->Get(i, j);

                        //Check if the gid is different from 0 - some tiles are empty
                        if (gid != 0) {
                            //L09: TODO 3: Obtain the tile set using GetTilesetFromTileId
                            TileSet* tileSet = GetTilesetFromTileId(gid);
                            if (tileSet != nullptr) {
                                //Get the Rect from the tileSetTexture;
                                SDL_Rect tileRect = tileSet->GetRect(gid);
                                //Get the screen coordinates from the tile coordinates
                                Vector2D mapCoord = MapToWorld(i, j);
                                //Draw the texture
                                Engine::GetInstance().render->DrawTexture(tileSet->texture, (int)mapCoord.getX(), (int)mapCoord.getY(), &tileRect);
                            }
                        }
                    }
                }
            }
        }
    }

    return ret;
}

// L09: TODO 2: Implement function to the Tileset based on a tile id
TileSet* Map::GetTilesetFromTileId(int gid) const
{
    for (const auto& tileset : mapData.tilesets) {
        if (gid >= tileset->firstGid && gid < tileset->firstGid + tileset->tileCount) {
            return tileset;
        }
    }
    return nullptr;
}

// Called before quitting
bool Map::CleanUp()
{
    LOG("Unloading map");

    // L06: TODO 2: Make sure you clean up any memory allocated from tilesets/map
    for (const auto& tileset : mapData.tilesets) {
        delete tileset;
    }
    mapData.tilesets.clear();

    // L07 TODO 2: clean up all layer data
    for (const auto& layer : mapData.layers)
    {
        delete layer;
    }
    mapData.layers.clear();

	// Clean up object groups
    for (const auto& group : mapData.objectgroups) {
        if (group != nullptr) {
            for (const auto& object : group->objects) {
                delete object;
            }
            group->objects.clear();
            delete group;
        }
    }
    mapData.objectgroups.clear();
    checkpoints.clear();
    for (const auto& body : mapBodies) {
        Engine::GetInstance().physics->DeletePhysBody(body);
    }
    mapBodies.clear();
    killedEnemies.clear();
    return true;
}

// Load new map
bool Map::Load(std::string path, std::string fileName)//
{
    bool ret = false;

    // Assigns the name of the map file and the path
    mapFileName = fileName;
    mapPath = path;
    std::string mapPathName = mapPath + mapFileName;

   
    pugi::xml_parse_result result = mapFileXML.load_file(mapPathName.c_str());

    if(result == NULL)
	{
		LOG("Could not load map xml file %s. pugi error: %s", mapPathName.c_str(), result.description());
		ret = false;
    }
    else {

        // L06: TODO 3: Implement LoadMap to load the map properties
        // retrieve the paremeters of the <map> node and store the into the mapData struct
        mapData.width = mapFileXML.child("map").attribute("width").as_int();
        mapData.height = mapFileXML.child("map").attribute("height").as_int();
        mapData.tileWidth = mapFileXML.child("map").attribute("tilewidth").as_int();
        mapData.tileHeight = mapFileXML.child("map").attribute("tileheight").as_int();

        // L06: TODO 4: Implement the LoadTileSet function to load the tileset properties
       
        //Iterate the Tileset
        for(pugi::xml_node tilesetNode = mapFileXML.child("map").child("tileset"); tilesetNode!=NULL; tilesetNode = tilesetNode.next_sibling("tileset"))
		{
            //Load Tileset attributes
			TileSet* tileSet = new TileSet();
            tileSet->firstGid = tilesetNode.attribute("firstgid").as_int();
            tileSet->name = tilesetNode.attribute("name").as_string();
            tileSet->tileWidth = tilesetNode.attribute("tilewidth").as_int();
            tileSet->tileHeight = tilesetNode.attribute("tileheight").as_int();
            tileSet->spacing = tilesetNode.attribute("spacing").as_int();
            tileSet->margin = tilesetNode.attribute("margin").as_int();
            tileSet->tileCount = tilesetNode.attribute("tilecount").as_int();
            tileSet->columns = tilesetNode.attribute("columns").as_int();

			//Load the tileset image
			std::string imgName = tilesetNode.child("image").attribute("source").as_string();
            tileSet->texture = Engine::GetInstance().textures->Load((mapPath+imgName).c_str());

			mapData.tilesets.push_back(tileSet);
		}

        // L07: TODO 3: Iterate all layers in the TMX and load each of them
        for (pugi::xml_node layerNode = mapFileXML.child("map").child("layer"); layerNode != NULL; layerNode = layerNode.next_sibling("layer")) {

            // L07: TODO 4: Implement the load of a single layer 
            //Load the attributes and saved in a new MapLayer
            MapLayer* mapLayer = new MapLayer();
            mapLayer->id = layerNode.attribute("id").as_int();
            mapLayer->name = layerNode.attribute("name").as_string();
            mapLayer->width = layerNode.attribute("width").as_int();
            mapLayer->height = layerNode.attribute("height").as_int();

            //L09: TODO 6 Call Load Layer Properties
            LoadProperties(layerNode, mapLayer->properties);

            //Iterate over all the tiles and assign the values in the data array
            for (pugi::xml_node tileNode = layerNode.child("data").child("tile"); tileNode != NULL; tileNode = tileNode.next_sibling("tile")) {
                mapLayer->tiles.push_back(tileNode.attribute("gid").as_int());
            }

            //add the layer to the map
            mapData.layers.push_back(mapLayer);
        }

        for (pugi::xml_node objectGroupNode = mapFileXML.child("map").child("objectgroup"); objectGroupNode != NULL; objectGroupNode = objectGroupNode.next_sibling("objectgroup")) {

            ObjectGroup* objectGroup = new ObjectGroup();
            objectGroup->id = objectGroupNode.attribute("id").as_int();
            objectGroup->name = objectGroupNode.attribute("name").as_string();

            // Itera sobre todos los objetos y asigna los valores en el array de datos
            for (pugi::xml_node objectNode = objectGroupNode.child("object"); objectNode != NULL; objectNode = objectNode.next_sibling("object")) {
                ObjectGroup::Object* object = new ObjectGroup::Object();
                object->id = objectNode.attribute("id").as_int();
                object->name = objectNode.attribute("name").as_string();
                object->x = objectNode.attribute("x").as_int();
                object->y = objectNode.attribute("y").as_int();
                object->width = objectNode.attribute("width").as_int();
                object->height = objectNode.attribute("height").as_int();

                objectGroup->objects.push_back(object);
            }

            mapData.objectgroups.push_back(objectGroup);
        }
        checkpoints.clear();
            

        // L08 TODO 3: Create colliders
        // L08 TODO 7: Assign collider type
        // Later you can create a function here to load and create the colliders from the map

        //Iterate the layer and create colliders
        for (const auto& objectGroup : mapData.objectgroups) {
            ColliderType type = ColliderType::DANGER;
            bool isSensor = false;

            if (objectGroup->name == "suelo") {
                type = ColliderType::PLATFORM;
            }
            else if (objectGroup->name == "paredes") {
                type = ColliderType::PARED;
            }
            else if (objectGroup->name == "spikes") {
                type = ColliderType::DANGER;
            }
            else if (objectGroup->name == "Checkpoints") {
                for (const auto& object : objectGroup->objects) {
                    if (object->name == "Player") {
                        continue;
                    }
                    std::shared_ptr<Checkpoint> checkpoint = std::dynamic_pointer_cast<Checkpoint>(
                        Engine::GetInstance().entityManager->CreateEntity(EntityType::CHECKPOINT)
                    );
                    checkpoint->position = Vector2D((float)object->x, (float)object->y);
                    checkpoint->name = object->name;
                    checkpoint->Start();
                    checkpoints.push_back(checkpoint);
                }
                continue;
            }
            else if (objectGroup->name == "Coins") {
                for (const auto& object : objectGroup->objects) {
                    if (object->name == "Coin" || object->name == "coin") {
                        std::shared_ptr<Coins> coin = std::dynamic_pointer_cast<Coins>(
                            Engine::GetInstance().entityManager->CreateEntity(EntityType::COIN)
                        );
                        coin->position = Vector2D((float)object->x, (float)object->y);
                        coin->xInicial = (int)object->x;
                        coin->yInicial = (int)object->y;
                        coin->Start();
                    }
                }
                continue;
              
            }
            else if (objectGroup->name == "Items") {
                for (const auto& object : objectGroup->objects) {
                    if (object->name == "Protection") {
                        std::shared_ptr<Protection> protection = std::dynamic_pointer_cast<Protection>(
                            Engine::GetInstance().entityManager->CreateEntity(EntityType::PROTECTION)
                        );
                        protection->position = Vector2D((float)object->x, (float)object->y);
                        protection->xInicial = (int)object->x;
                        protection->yInicial = (int)object->y;
                        protection->Start();
                    }
                    if (object->name == "Live") {
                        std::shared_ptr<ExtraLive> protection = std::dynamic_pointer_cast<ExtraLive>(
                            Engine::GetInstance().entityManager->CreateEntity(EntityType::EXTRALIVE)
                        );
                        protection->position = Vector2D((float)object->x, (float)object->y);
                        protection->xInicial = (int)object->x;
                        protection->yInicial = (int)object->y;
                        protection->Start();
                    }
                }
                continue;
            }
            else {
                continue;
            }
            for (const auto& object : objectGroup->objects) {

                int x = object->x;
                int y = object->y;
                int w = object->width;
                int h = object->height;

                if (type == ColliderType::SAVE && (w == 0 || h == 0)) {
                    w = mapData.tileWidth;
                    h = mapData.tileHeight;
                }

                int centerX = x + w / 2;
                int centerY = y + h / 2;

                PhysBody* body = nullptr;

                if (isSensor) {
                    body = Engine::GetInstance().physics.get()->CreateRectangleSensor(centerX, centerY, w, h, STATIC);
                }
                else {
                    body = Engine::GetInstance().physics.get()->CreateRectangle(centerX, centerY, w, h, STATIC);
                }

                if (body != nullptr) {
                    body->ctype = type;
                    body->objectName = object->name;
                    mapBodies.push_back(body);
                }
            }
        }

       

        ret = true;

        // L06: TODO 5: LOG all the data loaded iterate all tilesetsand LOG everything
        if (ret == true)
        {
            LOG("Successfully parsed map XML file :%s", fileName.c_str());
            LOG("width : %d height : %d", mapData.width, mapData.height);
            LOG("tile_width : %d tile_height : %d", mapData.tileWidth, mapData.tileHeight);
            LOG("Tilesets----");

            //iterate the tilesets
            for (const auto& tileset : mapData.tilesets) {
                LOG("name : %s firstgid : %d", tileset->name.c_str(), tileset->firstGid);
                LOG("tile width : %d tile height : %d", tileset->tileWidth, tileset->tileHeight);
                LOG("spacing : %d margin : %d", tileset->spacing, tileset->margin);
            }
            			
            LOG("Layers----");

            for (const auto& layer : mapData.layers) {
                LOG("id : %d name : %s", layer->id, layer->name.c_str());
				LOG("Layer width : %d Layer height : %d", layer->width, layer->height);
            }   
        }
        else {
            LOG("Error while parsing map file: %s", mapPathName.c_str());
        }

      

    }

    mapLoaded = ret;
    return ret;
}

// L07: TODO 8: Create a method that translates x,y coordinates from map positions to world positions
Vector2D Map::MapToWorld(int i, int j) const
{
    Vector2D ret;
    ret.setX((float)(i * mapData.tileWidth));
    ret.setY((float)(j * mapData.tileHeight));

    return ret;
}

// L09: TODO 6: Load a group of properties from a node and fill a list with it
bool Map::LoadProperties(pugi::xml_node& node, Properties& properties)
{
    bool ret = false;

    for (pugi::xml_node propertieNode = node.child("properties").child("property"); propertieNode; propertieNode = propertieNode.next_sibling("property"))
    {
        Properties::Property* p = new Properties::Property();
        p->name = propertieNode.attribute("name").as_string();
        p->value = propertieNode.attribute("value").as_bool(); // (!!) I'm assuming that all values are bool !!

        properties.propertyList.push_back(p);
    }

    return ret;
}

Vector2D Map::GetMapSizeInPixels()
{
    Vector2D sizeInPixels;
    sizeInPixels.setX((float)(mapData.width * mapData.tileWidth));
    sizeInPixels.setY((float)(mapData.height * mapData.tileHeight));
    return sizeInPixels;
}


// L07: TODO 8: Create a method that translates x,y coordinates from map positions to world positions



// L09: TODO 6: Load a group of properties from a node and fill a list with it
Vector2D Map::WorldToMap(int x, int y) {

    Vector2D ret(0, 0);
    ret.setX((float)(x / mapData.tileWidth));
    ret.setY((float)(y / mapData.tileHeight));

    return ret;
}

// L10: TODO 7: Create a method to get the map size in pixels


Vector2D Map::GetMapSizeInTiles()
{
    return Vector2D((float)mapData.width, (float)mapData.height);
}

// Method to get the navigation layer from the map
MapLayer* Map::GetNavigationLayer() {
    for (const auto& layer : mapData.layers) {
        if (layer->properties.GetProperty("Navigation") != NULL &&
            layer->properties.GetProperty("Navigation")->value) {
            return layer;
        }
    }

    return nullptr;
}


    void Map::LoadEntities(std::shared_ptr<Player>& player, std::vector<std::shared_ptr<Enemy>>& enemies) {
        std::list<std::shared_ptr<Entity>> toDestroy;
        for (auto& entity : Engine::GetInstance().entityManager->entities) {
            if (entity->type == EntityType::ENEMY || entity->type == EntityType::ENEMYFLYING) {
                toDestroy.push_back(entity);
            }
        }
        for (auto& entity : toDestroy) {
            Engine::GetInstance().entityManager->DestroyEntity(entity);
        }

        enemies.clear();
        //Iterate the object groups
        for (pugi::xml_node objectGroupNode = mapFileXML.child("map").child("objectgroup"); objectGroupNode != NULL; objectGroupNode = objectGroupNode.next_sibling("objectgroup")) {
            //Check if the object group is "Entities"
            if (objectGroupNode.attribute("name").as_string() == std::string("Entities") || objectGroupNode.attribute("name").as_string() == std::string("FinalBoss")) {

                //Iterate the objects
                for (pugi::xml_node objectNode = objectGroupNode.child("object"); objectNode != NULL; objectNode = objectNode.next_sibling("object")) {
                    
                    int id = objectNode.attribute("id").as_int();

                    bool isDead = false;
                    for (int killedId : killedEnemies) {
                        if (killedId == id) {
                            isDead = true;
                            break;
                        }
                    }
                    // Si está muerto, saltamos al siguiente ciclo y no lo creamos
                    if (isDead) continue;
                    //Get the entity type and position
                    std::string entityType = objectNode.attribute("type").as_string();
                    float x = objectNode.attribute("x").as_float();
                    float y = objectNode.attribute("y").as_float();

                    // Create entity based on type
                    if (entityType == "Player") {
                        // Create Player entity
                        if (player == nullptr) {
                            player = std::dynamic_pointer_cast<Player>(Engine::GetInstance().entityManager->CreateEntity(EntityType::PLAYER));
                            player->position = Vector2D(x, y);
                            player->Start();
                        }
                        //If the player already exists, just set its position
                        
                            player->SetPosition(Vector2D(x, y));
                            if (objectNode.attribute("score")) {
                                Player::score = objectNode.attribute("score").as_int();
                                LOG("Score cargado desde XML: %d", Player::score);
                            }

                            if (objectNode.attribute("timer")) {
                                Engine::GetInstance().scene->levelTimer = objectNode.attribute("timer").as_float();
                                LOG("Timer cargado desde XML: %f", Engine::GetInstance().scene->levelTimer);
                            }
                        
                    }
                    else if (entityType == "Enemy") {
                        std::shared_ptr<Enemy> enemy = std::dynamic_pointer_cast<Enemy>(Engine::GetInstance().entityManager->CreateEntity(EntityType::ENEMY));

                        enemy->position = Vector2D(x, y);
                        enemy->xInicial = (int)x;
                        enemy->yInicial = (int)y;
                        enemy->Start();
                        enemy->mapID = id;

                    }
                    else if (entityType == "EnemigoVolador") {
                        std::shared_ptr<EnemigoVolador> flyer = std::dynamic_pointer_cast<EnemigoVolador>(Engine::GetInstance().entityManager->CreateEntity(EntityType::ENEMYFLYING));

                        flyer->position = Vector2D(x, y);
                        flyer->xInicial = (int)x;
                        flyer->yInicial = (int)y;
                        flyer->Start();
                        flyer->mapID = id;
                       
                    }
                    else if (entityType == "FinalBoss") {
                        std::shared_ptr<FinalBoss> boss = std::dynamic_pointer_cast<FinalBoss>(Engine::GetInstance().entityManager->CreateEntity(EntityType::FINALBOSS));
                        boss->position = Vector2D(x, y);
                        boss->xInicial = (int)x;
                        boss->yInicial = (int)y;
                        boss->Start();
                        boss->mapID = id;
                    }
                }
            }
        }
    }

    //L15 TODO 4: Define a method to save entities to the map XML
    void Map::SaveEntities(std::shared_ptr<Player> player) {

        //Iterate the object groups
        for (pugi::xml_node objectGroupNode = mapFileXML.child("map").child("objectgroup"); objectGroupNode != NULL; objectGroupNode = objectGroupNode.next_sibling("objectgroup")) {

            //Check if the object group is "Entities"
            if (objectGroupNode.attribute("name").as_string() == std::string("Entities")) {

                //Iterate the objects
                for (pugi::xml_node objectNode = objectGroupNode.child("object"); objectNode != NULL; objectNode = objectNode.next_sibling("object")) {
                    std::string entityType = objectNode.attribute("type").as_string();
                    // Modify entity based on type
                    if (entityType == "Player") {
                        // Modify the Player entity values
                        Vector2D playerPos = player->GetPosition();
                        objectNode.attribute("x").set_value(playerPos.getX());
                        objectNode.attribute("y").set_value(playerPos.getY());

                        pugi::xml_attribute scoreAttr = objectNode.attribute("score");
                        if (!scoreAttr) scoreAttr = objectNode.append_attribute("score");
                        scoreAttr.set_value(Player::score);

                        pugi::xml_attribute timerAttr = objectNode.attribute("timer");
                        if (!timerAttr) timerAttr = objectNode.append_attribute("timer");
                        timerAttr.set_value(Engine::GetInstance().scene->levelTimer);
                    }
                }
            }
        }

        //Important: save the modifications to the XML 
        std::string mapPathName = mapPath + mapFileName;
        mapFileXML.save_file(mapPathName.c_str());

    }
    Vector2D Map::GetStartPoint(std::string layerName, std::string objectName)
    {
        for (const auto& group : mapData.objectgroups) {
            if (group->name == layerName) {
                for (const auto& object : group->objects) {
                    if (object->name == objectName) {
                        return Vector2D((float)object->x, (float)object->y);
                    }
                }
            }
        }
        return Vector2D(0, 0);
    }
    void Map::DrawLayer(const char* layerName) {
        if (!mapLoaded) return;

        for (const auto& mapLayer : mapData.layers) {
            if (mapLayer->name != layerName) continue;

            for (int i = 0; i < mapData.width; i++) {
                for (int j = 0; j < mapData.height; j++) {
                    int gid = mapLayer->Get(i, j);
                    if (gid != 0) {
                        TileSet* tileSet = GetTilesetFromTileId(gid);
                        if (tileSet != nullptr) {
                            SDL_Rect tileRect = tileSet->GetRect(gid);
                            Vector2D mapCoord = MapToWorld(i, j);
                            Engine::GetInstance().render->DrawTexture(tileSet->texture, (int)mapCoord.getX(), (int)mapCoord.getY(), &tileRect);
                        }
                    }
                }
            }
        }
    }
    // L19 TODO 1: Calculate Camera position in Tiles
    Vector2D Map::GetCameraPositionInTiles() {

        // Gets the camera position in world space. Moving the camera right means drawing the world shifted left. 
        // Multiplying by -1 converts render offset actual world - space camera position
        Vector2D camPos = Vector2D(Engine::GetInstance().render->camera.x * -1, Engine::GetInstance().render->camera.y * -1);
        if (camPos.getX() < 0) camPos.setX(0);
        if (camPos.getY() < 0) camPos.setY(0);

        // Converts the camera position to map tile coordinates
        Vector2D camPosTile = WorldToMap(camPos.getX(), camPos.getY());

        return camPosTile;
    }

    // L19 TODO 2: Calculate Camera limits in Tiles
    Vector2D Map::GetCameraLimitsInTiles(Vector2D camPosTile) {

        // Gets the camera size in world space and converts it to map tile coordinates
        Vector2D camSize = Vector2D(Engine::GetInstance().render->camera.w, Engine::GetInstance().render->camera.h);
        Vector2D camSizeTile = WorldToMap(camSize.getX(), camSize.getY());

        // Computes the tile range to draw
        Vector2D limits = Vector2D(camPosTile.getX() + camSizeTile.getX(), camPosTile.getY() + camSizeTile.getY());
        if (limits.getX() > mapData.width) limits.setX(mapData.width);
        if (limits.getY() > mapData.height) limits.setY(mapData.height);

        return limits;
    }
