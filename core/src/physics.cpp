#include "../include/physics.hpp"

Collider::Collider(Collider &&other) { operator=(std::move(other)); }
Collider::Collider() :
    _collisionspace(nullptr),
    _collision_enabled(false)
{}
Collider::~Collider() {}

Collider& Collider::operator=(Collider &&other) {
    if (this != &other) {
        _collisionspace = other._collisionspace;
        _this_iter = other._this_iter;
        _filterstate = other._filterstate;
        _collision_enabled = other._collision_enabled;
        other._collisionspace = nullptr;
        other._collision_enabled = false;
    }
    return *this;
}

// --------------------------------------------------------------------------------------------------------------------------

ColliderView::ColliderView(Collider *collider) : _collider(collider) {}

// --------------------------------------------------------------------------------------------------------------------------

CollisionSpace::CollisionSpace(unordered_map_string_Filter_t *filters) { init(filters); }
CollisionSpace::CollisionSpace() : _filters(nullptr), _initialized(false) {}
CollisionSpace::CollisionSpace(CollisionSpace &&other) { operator=(std::move(other)); }
CollisionSpace::~CollisionSpace() { /* automatic destruction is fine */ }

CollisionSpace &CollisionSpace::operator=(CollisionSpace &&other) {
    if (this != &other) {
        _colliders = std::move(other._colliders);

        // safe as structures owning memory are already moved
        other.uninit();
    }
    return *this;
}

void CollisionSpace::init(unordered_map_string_Filter_t *filters) {
    if (_initialized)
        throw InitializedException();
    
    _filters = filters;
    _initialized = true;
}

void CollisionSpace::uninit() {
    if (!_initialized)
        return;

    _colliders.clear();
    _filters = nullptr;
    _initialized = false;
}

ColliderView CollisionSpace::spawnCollider(Transform transform, glm::vec3 vel, const char *filter_name) {}

void CollisionSpace::erase(ColliderView colliderview) {}

void CollisionSpace::resetCollidedCount() {}

void CollisionSpace::detectCollisionAABB() {}

void CollisionSpace::step() {}

unsigned CollisionSpace::getCount() {}

bool CollisionSpace::initialized() {}

// --------------------------------------------------------------------------------------------------------------------------

bool computeCollisionAABB(Transform transf1, Transform transf2) {
    glm::vec3 &pos1 = transf1.pos;
    glm::vec3 &dim1 = transf1.scale;
    glm::vec3 &pos2 = transf2.pos;
    glm::vec3 &dim2 = transf2.scale;

    // get current collision
    float coll_x_space = glm::abs(pos1.x - pos2.x) - ((dim1.x + dim2.x) / 2.0f);
    float coll_y_space = glm::abs(pos1.y - pos2.y) - ((dim1.y + dim2.y) / 2.0f);
    float coll_z_space = glm::abs(pos1.z - pos2.z) - ((dim1.z + dim2.z) / 2.0f);

    if (coll_x_space < 0.0f && coll_y_space < 0.0f && coll_y_space < 0.0f)
        return true;
    return false;
}