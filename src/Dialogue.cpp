#include "Dialogue.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "EntityManager.h"
//El primer dialogo es el ultimo del vector!!
Dialogue::Dialogue()
{
	
	
}

Dialogue::~Dialogue() {
	
}

bool Dialogue::Awake() {
	return true;
}

bool Dialogue::Start() {

	
	
	
	


	return true;
}
void Dialogue::Draw(float dt) {
	if (hasStarted && lenght > 0) {
		SDL_Texture* currentTexture = dialogue.back();
		Engine::GetInstance().render->DrawTexture(currentTexture, xInicial, yInicial);
	}
}

bool Dialogue::Update(float dt)
{
	if (!hasStarted) return true;
	Draw(dt);
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN) {
	
		NextDialogue();
	
	}
	
	return true;
}
void Dialogue::BeginDialogue() {
	dialogueHelper = dialogue;

	hasStarted = true;



}
void Dialogue:: NextDialogue() {
	if (lenght > 0) {
		dialogue.pop_back();
		lenght--;
	}
	else {
		hasStarted = false;
		
	}
	HasEnded(hasStarted);
}
bool Dialogue:: HasEnded(bool name){

	hasEnded = !name;
	return hasEnded;


}
void Dialogue::AddDialogue(SDL_Texture* texture) {
	dialogue.push_back(texture);
	lenght++;
}

bool Dialogue::CleanUp()
{
	LOG("Unloading Coin");

		dialogue.clear();

	
	return true;
}

