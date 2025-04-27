#ifndef __EXTRALIFEPOWERUP_H__
#define __EXTRALIFEPOWERUP_H__

#include "GameObject.h"

class ExtraLifePowerUp : public GameObject
{
public:
    ExtraLifePowerUp();                 
    virtual ~ExtraLifePowerUp();        

    virtual bool CollisionTest(shared_ptr<GameObject> o);   
    virtual void OnCollision(const GameObjectList& objects); 
};

#endif
