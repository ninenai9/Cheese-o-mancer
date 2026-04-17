#pragma once

#include "Entity.h"
#include "Vector2D.h"

class CheeseBall : public Entity
{
public:
    CheeseBall();
    ~CheeseBall();

    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    bool CleanUp() override;

    void OnCollision(PhysBody* physA, PhysBody* physB) override;
    void OnCollisionEnd(PhysBody* physA, PhysBody* physB) override;

    void SetPosition(const Vector2D& pos);

private:
    PhysBody* body = nullptr;

    Vector2D position;

    float radius = 128.0f;

    SDL_Texture* texture = NULL;

    bool toDelete = false;
};