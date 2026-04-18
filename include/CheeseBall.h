#pragma once

#include "Entity.h"
#include "Vector2D.h"
#include "box2d/box2d.h"

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

    void SetVelocityy(b2Vec2 vel);
    PhysBody* pbody = nullptr;
    float radius = 128.0f;
private:
    

    Vector2D position;

    

    SDL_Texture* texture = NULL;

    bool toDelete = false;
};