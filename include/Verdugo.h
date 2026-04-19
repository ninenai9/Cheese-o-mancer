#pragma once
#include "Enemy.h"
#include "Physics.h"

enum VerdugoState {
    ATAQUE1,
    ATAQUE2,
    ATAQUE3A,
    ATAQUE3B,
    ATAQUE3C,
    ATAQUE4A,
    ATAQUE4B,
    MUERTO
};

enum AttackType {
    ATTACK_1,
    ATTACK_2,
    ATTACK_3,
    ATTACK_4
};

class Verdugo : public Enemy
{
public:
    Verdugo();
    ~Verdugo();

    bool Start() override;
    void Attack() override;
    bool Update(float dt) override;
    void Draw(float dt) override;
    void UpdateAttack();
    void OnCollision(PhysBody* physA, PhysBody* physB) override;
    void OnCollisionEnd(PhysBody* physA, PhysBody* physB) override;
    void ChangeCurrentAnimation();
    void DebugChangeState();
    AttackType ChooseRandomAttack();

    void ExecuteAttack();

    void UpdateAttackLogic();

    bool MoveToAttackRange(float targetRange);
   
protected:
    int attackTimer = 0;
    int attackCooldown = 50;
    float attackDuration = 20.0f;

    float hitboxStart = 5.0f;
    float hitboxEnd = 15.0f;
    bool hasHit = false;
    bool hitboxActive = false;

    bool isAttacking = false;

    bool playerInHitbox = true;
    AnimationSet animsAtaque1;
    AnimationSet animsAtaque2;
    AnimationSet animsAtaque3a;
    AnimationSet animsAtaque3b;
    AnimationSet animsAtaque3c;
    AnimationSet animsAtaque4a;
    AnimationSet animsAtaque4b;
    AnimationSet animsDeath;

    AnimationSet* currentAnimSet = nullptr;

    VerdugoState state;
    VerdugoState lastState;

    SDL_Texture* textureA1 = NULL;
    SDL_Texture* textureA2 = NULL;
    SDL_Texture* textureA3a = NULL;
    SDL_Texture* textureA3b = NULL;
    SDL_Texture* textureA3c = NULL;
    SDL_Texture* textureA4a = NULL;
    SDL_Texture* textureA4b = NULL;
    SDL_Texture* textureDeath = NULL;
    SDL_Texture* textureT = NULL;
    
    float offset = 0.0f;

    AttackType currentAttack = ATTACK_1;
    bool attackInProgress = false;
    bool attackFinished = false;

    float attackRange1 = 80.0f;
    float attackRange2 = 200.0f;
    float attackRange3 = 120.0f;

    bool facingLeft = false;
    bool bolazo = false;
};