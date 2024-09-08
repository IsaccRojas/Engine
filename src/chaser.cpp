#include "chaser.hpp"

void Chaser::_initBasic() {
    getAnimState().setCycleState(0);
}

void Chaser::_baseBasic() {
    if (*_killflag) {
        enqueueKill();
        return;
    }
    
    if (getBox()->getCollided())
        chaserCollision();
    chaserMotion();
    _t++;
}

void Chaser::_killBasic() {
    getManager()->spawnEntityEnqueue(_killeffect.c_str(), 1, getBox()->pos);
}

void Chaser::_collisionBasic(Box *box) {}

Object *Chaser::_getTarget() {
    // return first ID found
    std::vector<unsigned> ids = getManager()->getAllByGroup(1);
    if (ids.size() > 0)
        return getManager()->getObject(ids[0]);
    else
        return nullptr;
}

Chaser::Chaser(glm::vec3 scale, glm::vec3 dimensions, float health, std::string killeffect, bool *killflag) : 
    Basic(scale, dimensions), 
    _accel(0.075f), 
    _deccel(0.05f), 
    _spd_max(0.15f), 
    _t(rand() % 256), 
    _prevdir(0.0f),
    _health(health),
    _killeffect(killeffect),
    _killflag(killflag)
{}

void Chaser::chaserMotion() {
    // get a target
    Object *target = _getTarget();

    glm::vec3 &vel = getBox()->vel;
    glm::vec3 vel_i = vel;
    float spd_i = glm::length(vel);
    float dec_factor;

    // get decceleration based on current speed and apply
    if (spd_i != 0.0f)
        dec_factor = _deccel / glm::length(vel);
    else
        dec_factor = 0.0f;
    vel -= vel * dec_factor;

    // determine if deccelerated completely ("passed" 0)
    if ((vel_i.x > 0 && vel.x < 0) || (vel_i.x < 0 && vel.x > 0))
        vel.x = 0.0f;
    if ((vel_i.y > 0 && vel.y < 0) || (vel_i.y < 0 && vel.y > 0))
        vel.y = 0.0f;

    // accelerate based on position of target
    glm::vec3 dir;
    if (target) {
        dir = glm::normalize(target->getBox()->pos - getBox()->pos);
        if (dir.x == 0.0f && dir.y == 0.0f)
            dir = _prevdir;
        else
            _prevdir = dir;

        // orient direction vector based on time
        float sint = sinf(_t / 30.0f) * 22.5f;
        dir = glm::rotate(dir, glm::radians(sint), glm::vec3(0.0f, 0.0f, 1.0f));
    } else
        dir = _prevdir;
    
    vel += glm::vec3(dir.x * _accel, dir.y * _accel, 0.0f);

    // reduce velocity to max if speed exceeds max
    if (glm::length(vel) > _spd_max)
        vel = glm::normalize(vel) * _spd_max;
}

void Chaser::chaserCollision() {
        _health--;

    // spawn 2-3 particles
    int count = (rand() % 2) + 2;
    for (int i = 0; i < count; i++) {
        //Entity *effect = getManager()->getEntity(getManager()->spawnEntity("BallParticle"));
        //effect->getQuad()->pos.v = getBox()->pos;
    }

    if (_health <= 0)
        enqueueKill();
}