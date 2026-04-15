#pragma once

#include "Module.h"
#include "Player.h"
#include "Enemy.h"
#include "UIButton.h"
#include "Animation.h"

struct SDL_Texture;

enum class SceneID
{
	INTRO_SCREEN,
	MAIN_MENU,
	LEVEL1,
	LEVEL2,
	GAME_OVER,
	WIN_SCREEN,
	FINAL_WIN
};

class Scene : public Module
{
public:
	int savedLevel = 1;
	bool isPaused = false;
	bool storeOn = false;
	bool showUIDebug = false;
	Scene();

	// Destructor
	virtual ~Scene();

	// Called before render is available
	bool Awake();

	// Called before the first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called before all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	bool IsGamePaused() const
	{
		return showHelp;
	}
	// Return the player position
	Vector2D GetPlayerPosition();

	// Get tilePosDebug value
	std::string GetTilePosDebug() {
		return tilePosDebug;
	}

	bool OnUIMouseClickEvent(UIElement* uiElement);
	float levelTimer = 0.0f;
	void ChangeScene(SceneID newScene);
	void UnloadCurrentScene();
	void LoadScene(SceneID newScene);

	void SaveLevel();
	void LoadGame();

private:
	//Introscreen functions
	void LoadIntro();
	void UpdateIntro(float dt);
	void UnloadIntro();

	void ChangeToLvl1();
	void ChangeToLvl2();

	// L17 TODO 3: Define specific function for main menu scene: Load, Unload, Handle UI events
	void LoadMainMenu();
	void UnloadMainMenu();
	void UpdateMainMenu(float dt);
	void HandleMainMenuUIEvents(UIElement* uiElement);

	// L17 TODO 4: Define specific functions for level1 scene: Load, Unload, Update, PostUpdate
	void LoadLevel1();
	void UnloadLevel1();
	void UpdateLevel1(float dt);
	void PostUpdateLevel1();

	// L17 TODO 5: Define specific functions for level2 scene: Load, Unload, Update
	void LoadLevel2();
	void UpdateLevel2(float dt);
	void UnloadLevel2();

	// Funciones para la pantalla de Game Over
	void LoadGameOver(); 
	void UpdateGameOver(float dt); 
	void UnloadGameOver();
	void HandleGameOverUIEvents(UIElement* uiElement);

	// Funciones para Win Screen
	void LoadWinScreen();           
	void UpdateWinScreen(float dt);  
	void UnloadWinScreen();          
	void HandleWinScreenUIEvents(UIElement* uiElement);

	// Funciones para el Menú de Pausa
	void CreatePauseUI();            
	void UpdatePauseMenu();           
	void HandlePauseUIEvents(UIElement* uiElement); 
	void SetPause(bool pause);        

	// Funciones para Win Screen final
	void LoadFinalWin();
	void UpdateFinalWin(float dt);
	void UnloadFinalWin();
	void HandleFinalWinUIEvents(UIElement* uiElement);


	//Funciones Store
	void CreateStoreLevel1();

	void SetStore(bool store);

	void HandleStoreUIEvents(UIElement* uiElement);
private:

	//L03: TODO 3b: Declare a Player attribute
	std::shared_ptr<Player> player;
	std::string tilePosDebug = "[0,0]";
	SDL_Texture* helpTexture = nullptr;
	SDL_Texture* map1Texture = nullptr;
	bool showHelp = false;
	bool showMap = false;
	SDL_Texture* introTexture = nullptr;
	SDL_Texture* heartTexture = nullptr;
	std::vector<std::shared_ptr<Enemy>> enemies;

	std::shared_ptr<UIButton> uiBt;
	float volume = 1.0;
	bool continueGame = false;
	bool exitGame = false;
	bool showCredits = false;
	SceneID currentScene = SceneID::MAIN_MENU;
	SDL_Texture* loseTexture = nullptr;
	AnimationSet loseAnimSet;
};