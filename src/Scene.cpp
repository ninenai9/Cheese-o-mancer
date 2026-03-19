#include "Engine.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Window.h"
#include "Scene.h"
#include "Log.h"
#include "Entity.h"
#include "EntityManager.h"
#include "Player.h"
#include "Map.h"
#include "Item.h"
#include "Enemy.h"
#include "EnemigoVolador.h"
#include "UIManager.h"
#include "UISlider.h"
#include <fstream>
#include "Physics.h"
#include "FINALBOSS.h"

Scene::Scene() : Module()
{
	name = "scene";
	currentScene = SceneID::INTRO_SCREEN;
}

// Destructor
Scene::~Scene()
{}

// Called before render is available
bool Scene::Awake()
{
	LOG("Loading Scene");
	LoadGame();
	//LoadScene(currentScene); 
	bool ret = true;

	return ret;
}

// Called before the first frame
bool Scene::Start()
{

	LoadScene(currentScene);
	return true;
}

// Called each loop iteration
bool Scene::PreUpdate()
{
	return true;
}

// Called each loop iteration
bool Scene::Update(float dt)
{
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_F8) == KEY_DOWN) {
		showUIDebug = !showUIDebug; 
	}
	switch (currentScene)
	{
	case SceneID::INTRO_SCREEN:
		UpdateIntro(dt); 
		break;
	case SceneID::MAIN_MENU:
		UpdateMainMenu(dt);
		if (introTexture != nullptr) {
			SDL_Renderer* renderer = Engine::GetInstance().render->renderer;
			SDL_RenderTexture(renderer, introTexture, NULL, NULL);
		}
		break;
	case SceneID::LEVEL1:
		UpdateLevel1(dt);
		break;
	case SceneID::LEVEL2:
		UpdateLevel2(dt);
		break;

	case SceneID::GAME_OVER:
		UpdateGameOver(dt);
		break;
	case SceneID::WIN_SCREEN:
		UpdateWinScreen(dt);
		break;
	case SceneID::FINAL_WIN:
		UpdateFinalWin(dt);
		break;
	}


	return true;
}

// Called each loop iteration
bool Scene::PostUpdate()
{
	bool ret = true;

	switch (currentScene)
	{
	case SceneID::INTRO_SCREEN:
		if (introTexture != nullptr) {
			SDL_RenderTexture(Engine::GetInstance().render->renderer, introTexture, NULL, NULL);
		}
		if ((SDL_GetTicks() / 500) % 2 == 0) { 
			Engine::GetInstance().render->DrawText("PRESS SPACE TO PLAY", 500, 600, 0, 0, { 255, 255, 255, 255 });
		}
		break;
	case SceneID::MAIN_MENU:
		if (introTexture != nullptr) {
			SDL_RenderTexture(Engine::GetInstance().render->renderer, introTexture, NULL, NULL);
		}

		if (showCredits) {
			SDL_Rect bg = { 0, 0, 1280, 720 };
			Engine::GetInstance().render->DrawRectangle(bg, 0, 0, 0, 200, true, false);

			Engine::GetInstance().render->DrawText("CREDITS", 550, 150, 0, 0, { 255, 215, 0, 255 });
			Engine::GetInstance().render->DrawText("Irene & Queralt", 550, 250, 0, 0, { 255, 255, 255, 255 });
		}

		Engine::GetInstance().uiManager->PostUpdate();
		break;

	case SceneID::LEVEL1:
		PostUpdateLevel1();
		SaveLevel();
		break;
	case SceneID::LEVEL2:
		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_F6) == KEY_DOWN) {
			Engine::GetInstance().map->LoadEntities(player, enemies);
		}
		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_F5) == KEY_DOWN) {
			Engine::GetInstance().map->SaveEntities(player);
			savedLevel = 2;
			SaveLevel();
		}

		Engine::GetInstance().map->DrawLayer("Map");
		if (player != nullptr) {
			if (heartTexture != nullptr) {
				float w, h;
				SDL_GetTextureSize(heartTexture, &w, &h);
				int startX = 20;
				int startY = 20;
				int padding = 10;
				for (int i = 0; i < player->lives; i++) {
					float x = (float)(startX + i * (w + padding));
					float y = (float)startY;
					SDL_FRect destRect = { x, y, w, h };
					SDL_RenderTexture(Engine::GetInstance().render->renderer, heartTexture, NULL, &destRect);
				}
			}
			std::string scoreText = "Score: " + std::to_string(player->score);
			Engine::GetInstance().render->DrawText(scoreText.c_str(), 1100, 30, 0, 0, { 255, 255, 255, 255 });

			std::string timerText = "Time: " + std::to_string((int)levelTimer);
			Engine::GetInstance().render->DrawText(timerText.c_str(), 1100, 70, 0, 0, { 255, 255, 255, 255 });
		}
		if (isPaused) {
			SDL_Rect screenRect = { 0, 0, 1280, 720 }; 
			Engine::GetInstance().render->DrawRectangle(screenRect, 0, 0, 0, 150, true, false);

			Engine::GetInstance().render->DrawText("PAUSE", 580, 150, 0, 0, { 255, 255, 255, 255 });
		}
		Engine::GetInstance().uiManager->PostUpdate();
		if (showHelp && helpTexture != nullptr)
		{
			Engine::GetInstance().render->DrawTexture(helpTexture, 560, 300, NULL, 0.0f);
		}
		break;
	case SceneID::GAME_OVER:
		if (loseTexture != nullptr) {
			SDL_Rect sourceRect = loseAnimSet.GetCurrentFrame();

			SDL_FRect srcFRect = { (float)sourceRect.x, (float)sourceRect.y, (float)sourceRect.w, (float)sourceRect.h };

			SDL_RenderTexture(Engine::GetInstance().render->renderer, loseTexture, &srcFRect, NULL);
		}
		Engine::GetInstance().render->DrawText("YOU LOST", 440, 100, 400, 100, { 255, 0, 0, 255 });

		Engine::GetInstance().uiManager->PostUpdate();
		break;
	case SceneID::WIN_SCREEN:
		if (loseTexture != nullptr) {
			SDL_Rect sourceRect = loseAnimSet.GetCurrentFrame();
			if (sourceRect.w > 0) {
				SDL_FRect srcFRect = { (float)sourceRect.x, (float)sourceRect.y, (float)sourceRect.w, (float)sourceRect.h };
				SDL_RenderTexture(Engine::GetInstance().render->renderer, loseTexture, &srcFRect, NULL);
			}
		}

		Engine::GetInstance().render->DrawText("LEVEL COMPLETED!", 400, 100, 500, 100, { 0, 255, 0, 255 }); 
		{
			std::string scoreStr = "Score: " + std::to_string(Player::score);
			Engine::GetInstance().render->DrawText(scoreStr.c_str(), 540, 250, 0, 0, { 255, 255, 255, 255 });
		}

		{
			std::string timeStr = "Time: " + std::to_string((int)levelTimer) + "s";
			Engine::GetInstance().render->DrawText(timeStr.c_str(), 570, 300, 0, 0, { 255, 255, 255, 255 });
		}

		Engine::GetInstance().uiManager->PostUpdate();
		break;
	case SceneID::FINAL_WIN:
		if (loseTexture != nullptr) {
			SDL_Rect sourceRect = loseAnimSet.GetCurrentFrame();
			if (sourceRect.w > 0) {
				SDL_FRect srcFRect = { (float)sourceRect.x, (float)sourceRect.y, (float)sourceRect.w, (float)sourceRect.h };
				SDL_RenderTexture(Engine::GetInstance().render->renderer, loseTexture, &srcFRect, NULL);
			}
		}
		Engine::GetInstance().render->DrawText("CONGRATULATIONS!", 380, 100, 600, 100, { 255, 215, 0, 255 });

		{
			std::string scoreStr = "Final Score: " + std::to_string(Player::score);
			Engine::GetInstance().render->DrawText(scoreStr.c_str(), 500, 250, 0, 0, { 255, 255, 255, 255 });
		}

		{
			std::string timeStr = "Total Time: " + std::to_string((int)levelTimer) + "s";
			Engine::GetInstance().render->DrawText(timeStr.c_str(), 530, 300, 0, 0, { 255, 255, 255, 255 });
		}

		Engine::GetInstance().uiManager->PostUpdate();
		break;
	default:
		break;
	}
	if (showUIDebug) {
		for (const auto& ui : Engine::GetInstance().uiManager->UIElementsList) {

			if (!ui->visible) continue; 

			Uint8 r = 255, g = 0, b = 255;

			switch (ui->state) {
			case UIElementState::NORMAL:   r = 255; g = 0; b = 255;   break; // Magenta
			case UIElementState::FOCUSED:  r = 0; g = 255; b = 255;   break; // Cyan
			case UIElementState::PRESSED:  r = 255; g = 255; b = 0;   break; // Amarillo
			case UIElementState::SELECTED: r = 0; g = 255; b = 0;     break; // Verde
			case UIElementState::DISABLED: r = 255; g = 128; b = 0;   break; // Naranja
			}
			SDL_Rect box = ui->bounds;

			Engine::GetInstance().render->DrawRectangle(box, r, g, b, 255, false, false);

			SDL_Rect boxInner = { box.x + 1, box.y + 1, box.w - 2, box.h - 2 };
			Engine::GetInstance().render->DrawRectangle(boxInner, r, g, b, 255, false, false);

			SDL_Rect boxOuter = { box.x - 1, box.y - 1, box.w + 2, box.h + 2 };
			Engine::GetInstance().render->DrawRectangle(boxOuter, r, g, b, 255, false, false);
			SDL_Rect boxOuter2 = { box.x - 1, box.y - 1, box.w + 3, box.h + 3 };
			Engine::GetInstance().render->DrawRectangle(boxOuter2, r, g, b, 255, false, false);
		
		}
	}
	if (exitGame) return false;

	return ret;
}

void Scene::LoadGame()
{
	std::ifstream file("Assets/savegame.txt");
	if (!file.is_open()) {
		savedLevel = 1; // default value
		return;
	}

	file >> savedLevel;
	file.close();
}
bool Scene::OnUIMouseClickEvent(UIElement* uiElement)
{
	if (uiElement->id >= 20 && uiElement->id <= 30) { 
		HandlePauseUIEvents(uiElement);
		return true;
	}
	switch (currentScene)
	{
	case SceneID::INTRO_SCREEN:
		break;
	case SceneID::MAIN_MENU:
		HandleMainMenuUIEvents(uiElement);
		break;
	case SceneID::LEVEL1:
		break;
	case SceneID::LEVEL2:
		break;
	case SceneID::GAME_OVER: 
		HandleGameOverUIEvents(uiElement);
		break;
	case SceneID::WIN_SCREEN:
		HandleWinScreenUIEvents(uiElement);
		break;
	case SceneID::FINAL_WIN:
		HandleFinalWinUIEvents(uiElement);
		break;
	default:
		break;
	}

	return true;
}

// Called before quitting
bool Scene::CleanUp()
{
	LOG("Freeing scene");
	UnloadCurrentScene();
	return true;
}

Vector2D Scene::GetPlayerPosition()
{
	if (player) return player->GetPosition();
	else return Vector2D(0, 0);
}


// *********************************************
// Scene change functions
// *********************************************

void Scene::LoadScene(SceneID newScene)
{
	auto& engine = Engine::GetInstance();

	switch (newScene)
	{
	case SceneID::INTRO_SCREEN:
		LoadIntro();
		break;
	case SceneID::MAIN_MENU:
		LoadMainMenu();
		break;

	case SceneID::LEVEL1:
		LoadLevel1();
		break;

	case SceneID::LEVEL2:
		LoadLevel2();
		break;
	case SceneID::GAME_OVER:
		LoadGameOver();
		break;

	case SceneID::WIN_SCREEN:
		LoadWinScreen();
		break;
	case SceneID::FINAL_WIN:
		LoadFinalWin();
		break;
	}
}

void Scene::ChangeScene(SceneID newScene)
{
	UnloadCurrentScene();
	currentScene = newScene;
	LoadScene(currentScene);
}

void Scene::UnloadCurrentScene() {

	switch (currentScene)
	{
	case SceneID::INTRO_SCREEN:
		UnloadIntro();
		break;
	case SceneID::MAIN_MENU:
		UnloadMainMenu();
		break;

	case SceneID::LEVEL1:
		UnloadLevel1();
		break;

	case SceneID::LEVEL2:
		UnloadLevel2();
		break;
	case SceneID::GAME_OVER:
		UnloadGameOver();
		break;

	case SceneID::WIN_SCREEN:
		UnloadWinScreen();
		break;
	case SceneID::FINAL_WIN:
		UnloadFinalWin();
		break;
	}

}

// *********************************************
// MAIN MENU functions
// *********************************************

void Scene::LoadMainMenu() {

	Engine::GetInstance().audio->PlayMusic("Assets/Audio/Music/soundtrack.wav");
	introTexture = Engine::GetInstance().textures->Load("Assets/Screens/IntroScreen.png");
	// Instantiate a UIButton in the Scene
	
	//Botón START
	SDL_Rect btPos = { 520, 280, 200, 50 };
	Engine::GetInstance().uiManager->CreateUIElement(UIElementType::BUTTON, 1, "START", btPos, this);
	
	//Botón CONTINUE
	SDL_Rect continuePos = { 520, 350, 200, 50 };
	Engine::GetInstance().uiManager->CreateUIElement(UIElementType::BUTTON, 5, "CONTINUE", continuePos, this);

	// Botón OPTIONS
	SDL_Rect optionsBtnRect = { 520, 420, 200, 50 };
	Engine::GetInstance().uiManager->CreateUIElement(UIElementType::BUTTON, 3, "OPTIONS", optionsBtnRect, this);

	// Slider VOLUMEN 
	SDL_Rect sliderRect = { 520,560, 200, 30 };
	auto slider = Engine::GetInstance().uiManager->CreateUIElement(UIElementType::SLIDER, 2, "VOLUME MUSIC", sliderRect, this);
	if (slider) slider->visible = false;  
	// Slider VOLUMEN EFECTOS
	SDL_Rect sliderRect2 = { 520,490, 200, 30 };
	auto slider2 = Engine::GetInstance().uiManager->CreateUIElement(UIElementType::SLIDER, 11, "VOLUME EFFECTS", sliderRect2, this);
	if (slider2) slider2->visible = false;

	//Checkbox de la fullscreen

	SDL_Rect Fullscreen = { 520,420, 200, 30 };
	auto fullscreen = Engine::GetInstance().uiManager->CreateUIElement(UIElementType::CHECKBOX, 12, "FULL SCREEN", Fullscreen, this);
	if (fullscreen) fullscreen->visible = false;

	// Botón BACK 
	SDL_Rect backBtnRect = { 520, 630, 200, 50 };
	auto backBtn = Engine::GetInstance().uiManager->CreateUIElement(UIElementType::BUTTON, 4, "BACK", backBtnRect, this);
	if (backBtn) backBtn->visible = false; 

	//Botón EXIT
	SDL_Rect exitPosRect = { 520, 560, 200, 50 };
	auto exitPos = Engine::GetInstance().uiManager->CreateUIElement(UIElementType::BUTTON, 6, "EXIT", exitPosRect, this);

	//Botón CREDITS
	SDL_Rect creditsPosRect = { 520, 490, 200, 50 };
	auto creditsPos = Engine::GetInstance().uiManager->CreateUIElement(UIElementType::BUTTON, 7, "CREDITS", creditsPosRect, this);

	//Botón BACK CREDITS
	SDL_Rect backCreditPosRect = { 520, 560, 200, 50 };
	auto backCreditsBtn = Engine::GetInstance().uiManager->CreateUIElement(UIElementType::BUTTON, 8, "BACK", backCreditPosRect, this);
	if (backCreditsBtn) backCreditsBtn->visible = false;
}

void Scene::UnloadMainMenu() {
	// Clean up UI elements related to the main menu
	Engine::GetInstance().textures->UnLoad(introTexture);
	introTexture = nullptr;
	Engine::GetInstance().uiManager->CleanUp();
}

void Scene::UpdateMainMenu(float dt) {}

void Scene::HandleMainMenuUIEvents(UIElement* uiElement)
{
	switch (uiElement->id)
	{
	case 1:
		LOG("Main Menu: MyButton clicked!");
		continueGame = false;
		ChangeScene(SceneID::LEVEL1);
		break;
	case 2: 
		if (uiElement->type == UIElementType::SLIDER) {
			if (uiElement->id==2) {
			
			UISlider* slider = static_cast<UISlider*>(uiElement);
			float vol = slider->GetValue();
			Engine::GetInstance().audio->SetMusicVol(vol);
		
			
			
			}
		}
		break;

	case 3: 
		for (auto& element : Engine::GetInstance().uiManager->UIElementsList) {
				element->visible = false;
			
			if (element->id == 2 || element->id == 4 || element->id==11 || element->id==12) {
				element->visible = true;
			}
		}
		break;

	case 4: 
		for (auto& element : Engine::GetInstance().uiManager->UIElementsList) {
			element->visible = false; 
			if (element->id == 1 || element->id == 3 || element->id == 5 || element->id == 6 || element->id == 7) {
				element->visible = true;
			}
		}
		break;
	case 5:
		continueGame = true; 
		if (savedLevel == 1) {
			ChangeScene(SceneID::LEVEL1);
		}
		else {
			ChangeScene(SceneID::LEVEL2);
		}
		break;
	case 6: 
		exitGame = true; 
		break;
	case 7: 
		showCredits = true;
		for (auto& element : Engine::GetInstance().uiManager->UIElementsList) {
			element->visible = (element->id == 8);
		}
		break;
	case 8:
		showCredits = false;
		for (auto& element : Engine::GetInstance().uiManager->UIElementsList) {
			if (element->id == 1 || element->id == 3 || element->id == 5 || element->id == 6 || element->id == 7) {
				element->visible = true;
			}
			if (element->id == 8) {
				element->visible = false;
			}
		}
		break;
	case 11:
		if (uiElement->type == UIElementType::SLIDER) {
			if (uiElement->id == 11) {
				UISlider* slider = static_cast<UISlider*>(uiElement);
				float vol = slider->GetValue();
				Engine::GetInstance().audio->SetSFXVol(vol);
			}
		}
		break;
	case 12:
		if(uiElement->state== UIElementState::SELECTED) {
			Engine::GetInstance().window->SetFullscreen(true);
		}
		else {
			Engine::GetInstance().window->SetFullscreen(false);
		}

		
		break;
	}
}

// *********************************************
// Level 1 functions
// *********************************************

void Scene::LoadLevel1() {

	Engine::GetInstance().audio->PlayMusic("Assets/Audio/Music/soundtrack.wav");

	// Load Map
	Engine::GetInstance().map->Load("Assets/Maps/", "queralt.tmx");

	// Load Entities from map
	Engine::GetInstance().map->LoadEntities(player, enemies);

	heartTexture = Engine::GetInstance().textures->Load("Assets/Textures/heart4.png");
	isPaused = false;   
	CreatePauseUI();

	if (player != nullptr) {
		if (continueGame == false) {
			Vector2D startPos = Engine::GetInstance().map->GetStartPoint("Checkpoints", "Player");

			if (startPos.getX() != 0 || startPos.getY() != 0) {
				player->SetPosition(startPos);
				player->respawnPosition = { PIXEL_TO_METERS(startPos.getX()), PIXEL_TO_METERS(startPos.getY()) };
			}
			Player::score = 0;
			levelTimer = 0.0f;
			
		}
		
	}

	// Crear Items Manuales (Del antiguo Awake/Start)
	std::shared_ptr<Item> item = std::dynamic_pointer_cast<Item>(Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM));
	item->position = Vector2D(200, 672);
	item->Start();

	// Textura de ayuda
	helpTexture = Engine::GetInstance().textures->Load("Assets/textures/HELP.png");
}

void Scene::UpdateLevel1(float dt) {
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN) {
		SetPause(!isPaused);
	}
	if (isPaused) {
		return; 
	}

	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_F2) == KEY_DOWN) {
		ChangeScene(SceneID::LEVEL2);
	}

	if (player && !player->isDead()) {
		levelTimer += dt / 1000.0f;
	}
	// Lógica de cambio de nivel (Debug)
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_F2) == KEY_DOWN) {
		ChangeScene(SceneID::LEVEL2);
	}

	// Lógica de Ayuda (H)
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_H) == KEY_DOWN)
	{
		showHelp = !showHelp;
	}

	if (player && !player->isDead()) {
		levelTimer += dt / 1000.0f;
	}

	if (player && player->lives <= 0) {
		ChangeScene(SceneID::GAME_OVER);
		return;
	}

	// Lógica de Checkpoints (Teclas 1-9)
	for (int i = 0; i < 9; ++i) {
		if (Engine::GetInstance().input->GetKey((SDL_Scancode)(SDL_SCANCODE_1 + i)) == KEY_DOWN) {
			if (i < Engine::GetInstance().map->checkpoints.size()) {
				Vector2D checkpointPos = Engine::GetInstance().map->checkpoints[i]->position;
				checkpointPos.setY(checkpointPos.getY() - 16);
				if (player) player->SetPosition(checkpointPos);
			}
		}
	}

	for (const auto& checkpoint : Engine::GetInstance().map->checkpoints) {
		if (checkpoint->name == "end" && checkpoint->isActivated) {
			ChangeScene(SceneID::WIN_SCREEN);
			return;
		}
	}

	// Lógica de gestión de enemigos muertos
	if (player && player->isDead()) {
		for (auto it = enemies.begin(); it != enemies.end(); ) {
			if ((*it)->toDelete) { // si se tiene que borrar la destruye
				it = enemies.erase(it);
			}
			else {
				(*it)->Reset();
				++it;
			}
		}
	}
	
}

void Scene::UnloadLevel1() {

	// Clean up UI elements
	auto& uiManager = Engine::GetInstance().uiManager;
	uiManager->CleanUp();

	// Reset player reference
	player.reset();

	// Clear enemies list
	enemies.clear();
	heartTexture = nullptr;
	// Clean up map and entities
	Engine::GetInstance().map->CleanUp();
	Engine::GetInstance().entityManager->CleanUp();

}

void  Scene::PostUpdateLevel1() {

	// Cargar/Guardar estado (F5/F6)
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_F6) == KEY_DOWN) {
		Engine::GetInstance().map->LoadEntities(player, enemies);
	}
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_F5) == KEY_DOWN) {
		Engine::GetInstance().map->SaveEntities(player);
		savedLevel = 1;
	}


	// Dibujar Mapa
	Engine::GetInstance().map->DrawLayer("Map");

	// Dibujar Ayuda
	if (showHelp && helpTexture != nullptr)
	{
		Engine::GetInstance().render->DrawTexture(helpTexture, 560, 300, NULL, 0.0f);
	}

	if (player != nullptr) {
		if (heartTexture != nullptr) {
			float w, h;
			SDL_GetTextureSize(heartTexture, &w, &h);

			int startX = 20;
			int startY = 20;
			int padding = 10; 

			for (int i = 0; i < player->lives; i++) {
				float x = (float)(startX + i * (w + padding));
				float y = (float)startY;
				SDL_FRect destRect = { x, y, w, h };
				SDL_RenderTexture(Engine::GetInstance().render->renderer, heartTexture, NULL, &destRect);
			}
		}
		else {
			for (int i = 0; i < player->lives; i++) {
				SDL_Rect r = { 20 + i * 40, 20, 30, 30 };
				Engine::GetInstance().render->DrawRectangle(r, 255, 0, 0, 255, true, false);
			}
		}

		std::string scoreText = "Score: " + std::to_string(player->score);
		Engine::GetInstance().render->DrawText(scoreText.c_str(), 1100, 30, 0, 0, { 255, 255, 255, 255 });
	}

	if (player != nullptr) {
		std::string scoreText = "Score: " + std::to_string(player->score);
		Engine::GetInstance().render->DrawText(scoreText.c_str(), 1100, 30, 0, 0, { 255, 255, 255, 255 });

		std::string timerText = "Time: " + std::to_string((int)levelTimer);
		Engine::GetInstance().render->DrawText(timerText.c_str(), 1100, 70, 0, 0, { 255, 255, 255, 255 });
	}
	if (isPaused) {
		SDL_Rect screenRect = { 0, 0, 1280, 720 };
		Engine::GetInstance().render->DrawRectangle(screenRect, 0, 0, 0, 150, true, false);
		Engine::GetInstance().render->DrawText("PAUSE", 580, 150, 0, 0, { 255, 255, 255, 255 });
	}

	Engine::GetInstance().uiManager->PostUpdate();
}

// *********************************************
// Level 2 functions
// *********************************************

void Scene::LoadLevel2() {

	Engine::GetInstance().audio->PlayMusic("Assets/Audio/Music/level2music.wav");

	isPaused = false;   
	CreatePauseUI();
	
	heartTexture = Engine::GetInstance().textures->Load("Assets/Textures/heart4.png");
	//Call the function to load the map. 
	Engine::GetInstance().map->Load("Assets/Maps/", "level2.tmx");

	//Call the function to load entities from the map
	Engine::GetInstance().map->LoadEntities(player, enemies);
	if (continueGame == false) {

		levelTimer = 0.0f;
		Player::score = 0;
		if (player) {
			player->lives = 3;
		}

		Vector2D startPos = Engine::GetInstance().map->GetStartPoint("Checkpoints", "Player");

		if (startPos.getX() != 0 || startPos.getY() != 0) {
			player->SetPosition(startPos);
			player->respawnPosition = { PIXEL_TO_METERS(startPos.getX()), PIXEL_TO_METERS(startPos.getY()) };
		}
	}
}

void Scene::UpdateLevel2(float dt) {
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN) {
		SetPause(!isPaused);
	}
	if (isPaused) {
		return;
	}

	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_F2) == KEY_DOWN) {
		ChangeScene(SceneID::LEVEL2);
	}
	auto& allEntities = Engine::GetInstance().entityManager->entities;
	for (const auto& entity : allEntities) {
		if (entity->type == EntityType::FINALBOSS) {

			auto boss = std::static_pointer_cast<FinalBoss>(entity);
			if(boss->PlayerClose && !boss->isMusic) {
				Engine::GetInstance().audio->PlayMusic("assets/audio/music/boss_music.wav");
				boss->isMusic = true;
			}
			if (boss->isdead) {
				Player::AddPoints(1000);
				boss->toDelete = true;

				ChangeScene(SceneID::FINAL_WIN);
				return;
			}
		}
	}

	if (player && !player->isDead()) {
		levelTimer += dt / 1000.0f;
	}
	if (player && player->lives <= 0) {
		ChangeScene(SceneID::GAME_OVER);
		return;
	}
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN) {
		ChangeScene(SceneID::LEVEL1);
	}
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_H) == KEY_DOWN)
	{
		showHelp = !showHelp;
	}
	if (player) {
		levelTimer += dt / 1000.0f;
	}
	for (int i = 0; i < 9; ++i) {
		if (Engine::GetInstance().input->GetKey((SDL_Scancode)(SDL_SCANCODE_1 + i)) == KEY_DOWN) {
			if (i < Engine::GetInstance().map->checkpoints.size()) {
				Vector2D checkpointPos = Engine::GetInstance().map->checkpoints[i]->position;
				checkpointPos.setY(checkpointPos.getY() - 16);
				if (player) player->SetPosition(checkpointPos);
			}
		}
	}
	//if the finalboss PlayerClose= true, bossmusic sounds (same as dead music)
	
}

void Scene::UnloadLevel2() {

	auto& uiManager = Engine::GetInstance().uiManager;
	uiManager->CleanUp();

	player.reset();

	Engine::GetInstance().map->CleanUp();
	Engine::GetInstance().entityManager->CleanUp();

}

// *********************************************
// GAME OVER functions
// *********************************************

void Scene::LoadGameOver() {
	LOG("Loading Game Over Screen");
	Engine::GetInstance().audio->PlayMusic("Assets/Audio/Music/losemusic.wav");
	loseTexture = Engine::GetInstance().textures->Load("Assets/Screens/lose-win-screen.png");

	std::unordered_map<int, std::string> aliases;
	aliases[4] = "play";

	loseAnimSet.LoadFromTSX("Assets/Maps/lose.tsx", aliases);

	loseAnimSet.SetCurrent("play");
	 Engine::GetInstance().audio->PlayMusic("Assets/Audio/Music/losemusic.wav");

	SDL_Rect btnPos = { 1000, 650, 250, 50 };
	Engine::GetInstance().uiManager->CreateUIElement(UIElementType::BUTTON, 9, "BACK TO TITLE ->", btnPos, this);

}

void Scene::UpdateGameOver(float dt) {
	loseAnimSet.Update(dt);
}

void Scene::UnloadGameOver() {
	LOG("Unloading Game Over Screen");
	Engine::GetInstance().textures->UnLoad(loseTexture);
	loseTexture = nullptr;
	Engine::GetInstance().uiManager->CleanUp();
}

void Scene::HandleGameOverUIEvents(UIElement* uiElement)
{
	switch (uiElement->id)
	{
	case 9: 
		ChangeScene(SceneID::MAIN_MENU);
		break;
	default:
		break;
	}
}

// *********************************************
// WIN SCREEN functions
// *********************************************

void Scene::LoadWinScreen() {
	LOG("Loading Win Screen");
	Engine::GetInstance().audio->PlayMusic("Assets/Audio/Music/winmusic.wav");
	loseTexture = Engine::GetInstance().textures->Load("Assets/Screens/lose-win-screen.png");

	std::unordered_map<int, std::string> aliases;
	aliases[0] = "win"; 
	loseAnimSet.LoadFromTSX("Assets/Maps/lose.tsx", aliases);
	loseAnimSet.SetCurrent("win");

	SDL_Rect btnPos = { 1000, 650, 250, 50 };
	Engine::GetInstance().uiManager->CreateUIElement(UIElementType::BUTTON, 10, "NEXT LEVEL -->", btnPos, this);
}

void Scene::UpdateWinScreen(float dt) {
	loseAnimSet.Update(dt);
}

void Scene::UnloadWinScreen() {
	Engine::GetInstance().textures->UnLoad(loseTexture);
	loseTexture = nullptr;
	Engine::GetInstance().uiManager->CleanUp();
}

void Scene::HandleWinScreenUIEvents(UIElement* uiElement) {
	if (uiElement->id == 10) { 
		ChangeScene(SceneID::LEVEL2);
	}
}

void Scene::CreatePauseUI() {
	auto btnPauseHUD = Engine::GetInstance().uiManager->CreateUIElement(UIElementType::BUTTON, 30, "||", { 1200, 20, 50, 50 }, this);
	btnPauseHUD->visible = true;
	int x = 540;
	int y = 400;

	// RESUME
	auto btnResume = Engine::GetInstance().uiManager->CreateUIElement(UIElementType::BUTTON, 20, "RESUME", { x, y, 200, 50 }, this);
	btnResume->visible = false;

	// OPTIONS
	auto btnOptions = Engine::GetInstance().uiManager->CreateUIElement(UIElementType::BUTTON, 21, "OPTIONS", { x, y + 70, 200, 50 }, this);
	btnOptions->visible = false;

	//BACK TO TITLE
	auto btnTitle = Engine::GetInstance().uiManager->CreateUIElement(UIElementType::BUTTON, 22, "TITLE SCREEN", { x, y + 210, 200, 50 }, this);
	btnTitle->visible = false;

	// EXIT
	auto btnExit = Engine::GetInstance().uiManager->CreateUIElement(UIElementType::BUTTON, 23, "EXIT GAME", { x, y + 140, 200, 50 }, this);
	btnExit->visible = false;

	//SLIDER MUSICA
	auto sliderMusic = Engine::GetInstance().uiManager->CreateUIElement(UIElementType::SLIDER, 26, "MUSIC", { x, y, 200, 30 }, this);
	sliderMusic->visible = false;

	//SLIDER FX
	auto sliderFX = Engine::GetInstance().uiManager->CreateUIElement(UIElementType::SLIDER, 27, "FX", { x, y + 50, 200, 30 }, this);
	sliderFX->visible = false;

	//BACK FROM OPTIONS
	auto btnBackOpt = Engine::GetInstance().uiManager->CreateUIElement(UIElementType::BUTTON, 25, "BACK", { x, y + 140, 200, 50 }, this);
	btnBackOpt->visible = false;
}

void Scene::SetPause(bool pause) {
	isPaused = pause;
	for (auto& element : Engine::GetInstance().uiManager->UIElementsList) {
		if (element->id >= 20 && element->id <= 23) {
			element->visible = isPaused;
		}
		if (element->id == 30) {
			element->visible = !isPaused;
		}
		if (element->id >= 25 && element->id <= 29) {
			element->visible = false;
		}
	}
}


void Scene::HandlePauseUIEvents(UIElement* uiElement) {
	switch (uiElement->id) {
	case 20: 
		SetPause(false);
		break;
	case 21:
		for (auto& el : Engine::GetInstance().uiManager->UIElementsList) {
			if (el->id >= 20 && el->id <= 23) el->visible = false;
			if (el->id == 25 || el->id == 26 || el->id == 27) el->visible = true;
		}
		break;
	case 22: 
		SetPause(false); 
		ChangeScene(SceneID::MAIN_MENU);
		break;
	case 23: 
		exitGame = true;
		break;
	case 26: 
		if (uiElement->type == UIElementType::SLIDER) {
			UISlider* slider = static_cast<UISlider*>(uiElement);
			float vol = slider->GetValue();
			Engine::GetInstance().audio->SetMusicVol(vol);
			Engine::GetInstance().audio->SetSFXVol(vol);
		}
		break;
	case 25: 
		for (auto& el : Engine::GetInstance().uiManager->UIElementsList) {
			if (el->id >= 20 && el->id <= 23) el->visible = true;
			if (el->id >= 25 && el->id <= 28) el->visible = false;
		}
		break;
	case 27:
		if (uiElement->type == UIElementType::SLIDER) {
			float vol = ((UISlider*)uiElement)->GetValue();
			Engine::GetInstance().audio->SetSFXVol(vol); 
		}
		break;
	case 30: 
		SetPause(true); 
		break;
	}
}

void Scene::SaveLevel()
{
	std::ofstream file("Assets/savegame.txt");
	if (!file.is_open()) return;

	file << savedLevel << std::endl;

	file.close();
}

// *********************************************
// FINAL WIN functions 
// *********************************************

void Scene::LoadFinalWin() {
	LOG("Loading Final Win Screen");

	loseTexture = Engine::GetInstance().textures->Load("Assets/Screens/lose-win-screen.png");
	std::unordered_map<int, std::string> aliases;
	aliases[0] = "win";
	loseAnimSet.LoadFromTSX("Assets/Maps/lose.tsx", aliases);
	loseAnimSet.SetCurrent("win");

	Engine::GetInstance().audio->PlayMusic("Assets/Audio/Music/win_sound.wav"); //poner audio
	SDL_Rect btnPos = { 950, 650, 250, 50 };
	Engine::GetInstance().uiManager->CreateUIElement(UIElementType::BUTTON, 13, "GO TO TITLE ->", btnPos, this);
}

void Scene::UpdateFinalWin(float dt) {
	loseAnimSet.Update(dt);
}

void Scene::UnloadFinalWin() {
	Engine::GetInstance().textures->UnLoad(loseTexture);
	loseTexture = nullptr;
	Engine::GetInstance().uiManager->CleanUp();
}

void Scene::HandleFinalWinUIEvents(UIElement* uiElement) {
	if (uiElement->id == 13) {
		ChangeScene(SceneID::MAIN_MENU);
	}
}

// *********************************************
// INTRO SCREEN functions
// *********************************************

void Scene::LoadIntro() {
	LOG("Loading Intro Screen");
	introTexture = Engine::GetInstance().textures->Load("Assets/Screens/logo.png");
}

void Scene::UpdateIntro(float dt) {
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN) {
		ChangeScene(SceneID::MAIN_MENU);
	}
}

void Scene::UnloadIntro() {
	LOG("Unloading Intro Screen");
	Engine::GetInstance().textures->UnLoad(introTexture);
	introTexture = nullptr;
}