#include "orb.hpp"

void Orb::_initBasic() {
    //removeBox();
}

void Orb::_baseBasic() {
    // get source position and mouse position
    glm::vec3 srcpos = _source->getBox()->pos;
    glm::vec2 mousepos = _input->mousepos();
    glm::vec3 mousepos3d = glm::vec3(mousepos.x, mousepos.y, 0.0f);

    // get normalized and scaled direction
    glm::vec3 dirvec = mousepos3d - srcpos;
    dirvec /= glm::length(dirvec);

    // set new position
    getBox()->pos = srcpos + (dirvec * _radius);

    // spawn projectile if not on cooldown
    if (_cooldown <= 0.0f) {
        if (_input->get_m1()) {
            // spawn projectile, set its position, and set cooldown
            int id = _pm->spawnObject("OrbShot");
            if (id >= 0) {
                Object *shot = _pm->getObject(id);
                shot->getBox()->pos = getBox()->pos;
                shot->getBox()->vel = dirvec;
            }

            _cooldown = _max_cooldown;

            getAnimState().setAnimState(1);
            _anim_cooldown = _anim_max_cooldown;
        }
    } else
        _cooldown -= 1.0f;

    if (_anim_cooldown > 0)
        _anim_cooldown -= 1.0f;
    else if (_anim_cooldown == 0) {
        getAnimState().setAnimState(0);
        _anim_cooldown -= 1.0f;
    }

    
}

void Orb::_killBasic() {}

void Orb::_collisionBasic(Box *box) {}

Orb::Orb() : Basic(glm::vec3(6.0f, 6.0f, 0.0f)), _input(nullptr), _source(nullptr), _pm(nullptr), _orb_ready(false), _radius(12.0f), _cooldown(0.0f), _max_cooldown(30.0f),
_anim_cooldown(0.0f), _anim_max_cooldown(5.0f) {}

void Orb::orbSetup(Input *input, Basic *source, ObjectManager *projectilemanager) {
    _input = input;
    _source = source;
    _pm = projectilemanager;
    _orb_ready = true;
};