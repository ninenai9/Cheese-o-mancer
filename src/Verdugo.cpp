#include "Verdugo.h"
#include "Engine.h"
#include "Log.h"
#include "Textures.h"
#include <Scene.h>

Verdugo::Verdugo() : Enemy()
{
    name = "Verdugo";
}

Verdugo::~Verdugo()
{
}

bool Verdugo::Start()
{
    texW = 128*8;
    texH = 128*7;
	attackRange = 5;
    offsetAttackHitboxX = 40;
    offsetAttackHitboxY = -texH/2;
    std::unordered_map<int, std::string> aliasesAnim = { {0,"transformar"} };
    std::unordered_map<int, std::string> aliasesAnim1 = { {0,"ataque1"} };
    std::unordered_map<int, std::string> aliasesAnim2 = { {0,"ataque2"} };
    std::unordered_map<int, std::string> aliasesAnim3a = { {0,"ataque3start"}, {12,"ataque3run"} };
    std::unordered_map<int, std::string> aliasesAnim3b = { {0,"ataque3b"} };
    std::unordered_map<int, std::string> aliasesAnim3c = { {0,"ataque3c"} };
    std::unordered_map<int, std::string> aliasesAnim4a = { {0,"ataque4a"} };
    std::unordered_map<int, std::string> aliasesAnim4b = { {0,"ataque4b"} };
    std::unordered_map<int, std::string> aliasesAnimDeath = { {0,"muerte"} };
    anims.LoadFromTSX("assets/Textures/Spritesheets/Cheese Executoner/cexecutonerTest.tsx",aliasesAnim);
    animsAtaque1.LoadFromTSX("assets/Textures/Spritesheets/Cheese Executoner/ataque1.tsx", aliasesAnim1);
    animsAtaque2.LoadFromTSX("assets/Textures/Spritesheets/Cheese Executoner/ataque2.tsx", aliasesAnim2);
    animsAtaque3a.LoadFromTSX("assets/Textures/Spritesheets/Cheese Executoner/ataque3a.tsx", aliasesAnim3a);
    animsAtaque3b.LoadFromTSX("assets/Textures/Spritesheets/Cheese Executoner/ataque3b.tsx", aliasesAnim3b);
    animsAtaque3c.LoadFromTSX("assets/Textures/Spritesheets/Cheese Executoner/ataque3c.tsx", aliasesAnim3c);
    animsAtaque4a.LoadFromTSX("assets/Textures/Spritesheets/Cheese Executoner/ataque4a.tsx", aliasesAnim4a);
    animsAtaque4b.LoadFromTSX("assets/Textures/Spritesheets/Cheese Executoner/ataque4b.tsx", aliasesAnim4b);
    animsDeath.LoadFromTSX("assets/Textures/Spritesheets/Cheese Executoner/death.tsx", aliasesAnimDeath);
    texName = "assets/Textures/Spritesheets/Cheese Executoner/transformar.png";
    spriteSheetName = "";
    
    //Initialize Player parameters
    texture = Engine::GetInstance().textures->Load(texName);
    textureA1 = Engine::GetInstance().textures->Load("assets/Textures/Spritesheets/Cheese Executoner/ataque1.png");
    textureA2 = Engine::GetInstance().textures->Load("assets/Textures/Spritesheets/Cheese Executoner/ataque2.png");
    textureA3a = Engine::GetInstance().textures->Load("assets/Textures/Spritesheets/Cheese Executoner/ataque3a.png");
    textureA3b = Engine::GetInstance().textures->Load("assets/Textures/Spritesheets/Cheese Executoner/ataque3b.png");
    textureA3c = Engine::GetInstance().textures->Load("assets/Textures/Spritesheets/Cheese Executoner/ataque3c.png");
    textureA4a = Engine::GetInstance().textures->Load("assets/Textures/Spritesheets/Cheese Executoner/ataque4a.png");
    textureA4b = Engine::GetInstance().textures->Load("assets/Textures/Spritesheets/Cheese Executoner/ataque4b.png");
    textureDeath = Engine::GetInstance().textures->Load("assets/Textures/Spritesheets/Cheese Executoner/death.png");

    //Add physics to the enemy - initialize physics body
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
    Vector2D tilePos = Engine::GetInstance().map->WorldToMap((int)pos.getX(), (int)pos.getY());
    //Reset pathfinding
    pathfinding->ResetPath(tilePos);

    CreateAttackHitbox(GetPosition().getX(),GetPosition().getY(), 70,200);
    state = ATAQUE1;
    lastState = ATAQUE1;
    currentAnimSet = &animsAtaque1;
    currentAnimSet->SetCurrent("ataque1");
    texture = textureA1;
    LOG("Verdugo creado");

    return true;
}

void Verdugo::Attack()
{
	isAttacking = true;
	attackTimer = attackDuration;

	LOG("Verdugo empieza ataque");
}

bool Verdugo::Update(float dt)
{
    repathTimer++;

    GetPhysicsValues();

    distanceToPlayer = CalculateDistance();

    //if (isAttacking) {
    //    velocity.x = 0; 
    //    UpdateAttack();
    //}
    //else {
    //    attackTimer--; // cooldown

    //    if (distanceToPlayer < detectionRange) {
    //        if (distanceToPlayer < attackRange && attackTimer <= 0.0f) {
    //            Attack();
    //        }
    //        else if (distanceToPlayer < attackRange) {
    //            //No hacer nada
    //        }
    //        else {
    //            PerformPathfinding();
    //            Move();
    //        }
    //    }
    //    else {
    //        Vector2D enemyPos = GetPosition();
    //        Vector2D enemyTilePos = Engine::GetInstance().map->WorldToMap(enemyPos.getX(), enemyPos.getY());
    //        ResetPathfinding(enemyTilePos);
    //    }
    //}
    if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_Z) == KEY_DOWN)
    {
        DebugChangeState();
    }
    if (!attackInProgress)
    {
        currentAttack = ChooseRandomAttack();

        float range = attackRange1;

        if (currentAttack == ATTACK_2) range = attackRange2;
        if (currentAttack == ATTACK_3) range = attackRange3;

        if (MoveToAttackRange(range))
        {
            ExecuteAttack();
        }
    }
    else
    {
        UpdateAttackLogic();
        
    }
    /*PerformPathfinding();
    Move();*/
    ChangeCurrentAnimation();
    ApplyPhysics();
    Draw(dt);

    return true;
}

void Verdugo::Draw(float dt)
{
    currentAnimSet->Update(dt);
    const SDL_Rect& animFrame = currentAnimSet->GetCurrentFrame();

    //// Update render position using your PhysBody helper
    int x, y;
    pbody->GetPosition(x, y);
    position.setX((float)x);
    position.setY((float)y);

    // Draw pathfinding debug
    pathfinding->DrawPath();

    SDL_FlipMode flip = facingLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    int drawX = x - animFrame.w / 2;
    int drawY = y - animFrame.h / 2 - offset;

    Engine::GetInstance().render->DrawTexture(
        texture,
        drawX,
        drawY,
        &animFrame,
        1.0f,
        0.0,
        INT_MAX,
        INT_MAX,
        flip
    );
    //Draw the player using the texture and the current animation frame
    /*Engine::GetInstance().render->DrawTexture(texture, x - texW / 2, y - texH / 2, &sect, 1, 0, 0, 0, SDL_FLIP_NONE);*/

    //SDL_FlipMode flip = facingLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    /*Engine::GetInstance().render->DrawTexture(texture, x - texW / 2, y - texH / 2, &animFrame, 1.0f, 0.0, INT_MAX, INT_MAX, SDL_FLIP_NONE);*/
}

void Verdugo::UpdateAttack()
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

void Verdugo::OnCollision(PhysBody* physA, PhysBody* physB)
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

void Verdugo::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
    if (physA == attackHitbox && physB->ctype == ColliderType::PLAYER) {
        playerInHitbox = false;
    }
}

void Verdugo::ChangeCurrentAnimation() {

    if (state == lastState) return;

    lastState = state;

    switch (state)
    {
    case ATAQUE1:
        currentAnimSet = &animsAtaque1;
        texture = textureA1;
        currentAnimSet->SetCurrent("ataque1");
        offset = 0.0f;
        break;

    case ATAQUE2:
        currentAnimSet = &animsAtaque2;
        texture = textureA2;
        currentAnimSet->SetCurrent("ataque2");
        offset = 0.0f;
        break;

    case ATAQUE3A:
        currentAnimSet = &animsAtaque3a;
        texture = textureA3a;
        currentAnimSet->SetCurrent("ataque3start"); // importante: alias correcto
        offset = texH/2;
        break;

    case ATAQUE3B:
        currentAnimSet = &animsAtaque3b;
        texture = textureA3b;
        currentAnimSet->SetCurrent("ataque3b");
        offset = texH/2;
        break;

    case ATAQUE3C:
        currentAnimSet = &animsAtaque3c;
        texture = textureA3c;
        currentAnimSet->SetCurrent("ataque3c");
        offset = texH/2;
        break;

    case ATAQUE4A:
        currentAnimSet = &animsAtaque4a;
        texture = textureA4a;
        currentAnimSet->SetCurrent("ataque4a");
        offset = texH/2;
        break;

    case ATAQUE4B:
        currentAnimSet = &animsAtaque4b;
        texture = textureA4b;
        currentAnimSet->SetCurrent("ataque4b");
        offset = texH/2;
        break;

    case MUERTO:
        currentAnimSet = &animsDeath;
        texture = textureDeath;
        currentAnimSet->SetCurrent("muerte");
        offset = 0.0f;
        break;

    default:
        break;
    }

}

void Verdugo::DebugChangeState()
{
    int next = (int)state + 1;

    if (next > MUERTO)
        next = ATAQUE1;

    state = (VerdugoState)next;
    currentAnimSet->Resets();
    LOG("Cambio de estado a: %d", state);
}

AttackType Verdugo::ChooseRandomAttack()
{
    int r = rand() % 3;

    AttackType atk;

    switch (r)
    {
    case 0: atk = ATTACK_1; break;
    case 1: atk = ATTACK_2; break;
    case 2: atk = ATTACK_3; break;
    default: atk = ATTACK_1; break;
    }

    LOG("Elegido ataque: %d", atk);

    return atk;
}
void Verdugo::ExecuteAttack()
{
    switch (currentAttack)
    {
    case ATTACK_1:
        LOG("INICIO ATAQUE 1 (garrotazo)");
        state = ATAQUE1;
        attackInProgress = true;
        break;

    case ATTACK_2:
        LOG("INICIO ATAQUE 2 (salto)");
        state = ATAQUE2;

        // impulso parabólico simple
        velocity.y = -40.0f;
        velocity.x = facingLeft ? 25.0f : -25.0f;

        attackInProgress = true;
        break;

    case ATTACK_3:
        LOG("INICIO ATAQUE 3 (persecución)");
        state = ATAQUE3A;
        attackInProgress = true;
        break;

    default:
        break;
    }
}

void Verdugo::UpdateAttackLogic()
{
    switch (currentAttack)
    {
    case ATTACK_1:
        // termina cuando animación acaba
        if (currentAnimSet->HasFinished())
        {
            currentAnimSet->Resets();
            attackInProgress = false;
        }
        break;

    case ATTACK_2:
        if (currentAnimSet->HasFinished())
        {
            currentAnimSet->Resets();
            attackInProgress = false;
            velocity.x = 0;
        }
        break;

    case ATTACK_3:
        // fase A: acercarse corriendo
        if (state == ATAQUE3A)
        {
            if (playerInHitbox)
            {
                state = ATAQUE3B;
                currentAnimSet->Resets();
            }

            
            if (bolazo)
            {
                bolazo = false;
                state = ATAQUE3C;
                currentAnimSet->Resets();
            }
        }
        else
        {
            if (currentAnimSet->HasFinished())
            {
                currentAnimSet->Resets();
                attackInProgress = false;
            }
        }
        break;

    default:
        break;
    }
}
bool Verdugo::MoveToAttackRange(float targetRange)
{
    Vector2D playerPos = Engine::GetInstance().scene->GetPlayerPosition();
    Vector2D myPos = GetPosition();

    float dx = playerPos.getX() - myPos.getX();
    float dy = playerPos.getY() - myPos.getY();

    float distSq = dx * dx + dy * dy;
    float rangeSq = targetRange * targetRange;

    facingLeft = playerPos.getX() < myPos.getX();

    if (distSq > rangeSq)
    {
        velocity.x = facingLeft ? -speed : speed;
        return false;
    }

    velocity.x = 0;
    return true;
}