#include "Rat.h"
#include "Engine.h"
#include "Log.h"

Rat::Rat() : Enemy()
{
    name = "Rat";
}

Rat::~Rat()
{
}

bool Rat::Start()
{
    texW = 300;
    texH = 100;
	attackRange = 5;
    offsetAttackHitboxX = 40;
    offsetAttackHitboxY = -texH/2;
    texName = "Assets/Textures/placeholder_Jester.png";
    Enemy::Start();
    CreateAttackHitbox(GetPosition().getX(),GetPosition().getY(), 70,200);
    
    LOG("Verdugo creado");

    return true;
}

void Rat::Attack()
{
	isAttacking = true;
	attackTimer = attackDuration;

	LOG("Verdugo empieza ataque");
}

bool Rat::Update(float dt)
{
    repathTimer++;

    GetPhysicsValues();

    distanceToPlayer = CalculateDistance();

    if (isAttacking) {
        velocity.x = 0; 
        UpdateAttack();
    }
    else {
        attackTimer--; // cooldown

        if (distanceToPlayer < detectionRange) {
            if (distanceToPlayer < attackRange && attackTimer <= 0.0f) {
                Attack();
            }
            else if (distanceToPlayer < attackRange) {
                //No hacer nada
            }
            else {
                PerformPathfinding();
                Move();
            }
        }
        else {
            Vector2D enemyPos = GetPosition();
            Vector2D enemyTilePos = Engine::GetInstance().map->WorldToMap(enemyPos.getX(), enemyPos.getY());
            ResetPathfinding(enemyTilePos);
        }
    }

    ApplyPhysics();
    Draw(dt);

    return true;
}

void Rat::UpdateAttack()
{
    if (!isAttacking) return;

    attackTimer--;

    // Ventana de ataque
    if (attackTimer <= (attackDuration - hitboxStart) &&
        attackTimer >= (attackDuration - hitboxEnd))
    {
        if (!hitboxActive) {
            hitboxActive = true;
            hasHit = false; 
            LOG("Hitbox ACTIVADA");
        }
    }
    else {
        if (hitboxActive) {
            hitboxActive = false;
            LOG("Hitbox DESACTIVADA");
        }
    }

    
    if (hitboxActive && playerInHitbox && !hasHit) {
        LOG("AAUAUCHHH");

      

        hasHit = true; 
    }

    // Fin del ataque
    if (attackTimer <= 0) {
        isAttacking = false;
        hitboxActive = false;
        attackTimer = attackCooldown;

        LOG("Ataque terminado");
    }
}

void Rat::OnCollision(PhysBody* physA, PhysBody* physB)
{
    // Solo actuar si la hitbox está activa
    if (!hitboxActive) return;

    switch (physB->ctype)
    {
    case ColliderType::PLAYER:
    {
        playerInHitbox = true;
        break;
    }

    default:
        break;
    }
}

void Rat::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
    if (physA == attackHitbox && physB->ctype == ColliderType::PLAYER) {
        playerInHitbox = false;
    }
}

