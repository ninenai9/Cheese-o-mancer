#include "CheeseBall.h"
#include "Engine.h"
#include "Physics.h"
#include "Render.h"
#include "Textures.h"
#include "Audio.h"
#include "Log.h"

CheeseBall::CheeseBall()
    : Entity(EntityType::CHEESEBALL)
{
    name = "CheeseBall";
}

CheeseBall::~CheeseBall()
{
}

bool CheeseBall::Awake()
{
    return true;
}

bool CheeseBall::Start()
{
    texture = Engine::GetInstance().textures->Load("assets/textures/cheeseball.png");

    body = Engine::GetInstance().physics->CreateCircle(position.getX(), position.getY(), radius, bodyType::DYNAMIC);

    body->listener = this;
    body->ctype = ColliderType::CHEESEBALL;

    LOG("CheeseBall created");

    return true;
}

bool CheeseBall::Update(float dt)
{
    if (toDelete) return true;

    int x, y;
    body->GetPosition(x, y);

    position.setX((float)x);
    position.setY((float)y);

    Engine::GetInstance().render->DrawTexture(
        texture,
        x - radius,
        y - radius,
        nullptr
    );

    return true;
}

void CheeseBall::OnCollision(PhysBody* physA, PhysBody* physB)
{
    //if (physB->ctype == ColliderType::PLAYER)
    //{
    //    LOG("Player touched CheeseBall");
    //    // aquí decides: sumar puntos, heal, boost, etc.
    //}
}

void CheeseBall::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
}

bool CheeseBall::CleanUp()
{
    LOG("Cleaning CheeseBall");

    if (body)
    {
        Engine::GetInstance().physics->DeletePhysBody(body);
        body = nullptr;
    }

    Engine::GetInstance().textures->UnLoad(texture);

    return true;
}

void CheeseBall::SetPosition(const Vector2D& pos)
{
    position = pos;

    if (body)
    {
        body->SetPosition(pos.getX(), pos.getY());
    }
}