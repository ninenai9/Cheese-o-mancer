#pragma once
#include "Enemy.h"
#include "Physics.h"
class Rat : public Enemy
{
public:
    Rat();
    ~Rat();

    bool Start() override;
    void Attack() override;
    bool Update(float dt) override;
    void UpdateAttack();
    void OnCollision(PhysBody* physA, PhysBody* physB) override;
    void OnCollisionEnd(PhysBody* physA, PhysBody* physB) override;
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
};