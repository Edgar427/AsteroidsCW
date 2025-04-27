#include "ExtraLifePowerUp.h"
#include "Spaceship.h"
#include "BoundingShape.h"

ExtraLifePowerUp::ExtraLifePowerUp()
    : GameObject("ExtraLifePowerUp")
{
}

ExtraLifePowerUp::~ExtraLifePowerUp()
{
}

bool ExtraLifePowerUp::CollisionTest(shared_ptr<GameObject> o)
{
    if (o->GetType() != GameObjectType("Spaceship")) return false;
    return mBoundingShape->CollisionTest(o->GetBoundingShape());
}

void ExtraLifePowerUp::OnCollision(const GameObjectList& objects)
{
    for (auto& obj : objects)
    {
        if (obj->GetType() == GameObjectType("Spaceship"))
        {
            auto spaceship = dynamic_pointer_cast<Spaceship>(obj);
            if (spaceship)
            {
                mWorld->FlagForRemoval(GetThisPtr());  
              
            }
        }
    }
}
