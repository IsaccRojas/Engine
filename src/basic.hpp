#ifndef BASIC_HPP_
#define BASIC_HPP_

#include "../core/include/object.hpp"

// extension of Object, includes virtual capture method _captureBasic() that is invoked by capture()
class Basic : public Object {
    glm::vec3 _scale;
    glm::vec3 _dimensions;

    void _initObject();
    void _baseObject();
    void _killObject();
    void _collisionObject(Box *box);
    virtual void _initBasic();
    virtual void _baseBasic();
    virtual void _killBasic();
    virtual void _collisionBasic(Box *box);

    //virtual void _captureBasic(Basic *captive);
public:
    Basic(glm::vec3 scale, glm::vec3 dimensions);

    //void capture(Basic *captive);
};

// wraps an ObjectManager, and pushes to a queue of T references on spawns 
template<class T>
class Interceptor {
    std::queue<T*> _captors;
    ObjectManager *_manager;
public:
    Interceptor(ObjectManager *manager) : _manager(manager) {}
    Interceptor(Interceptor &&other) { operator=(std::move(other)); }
    Interceptor(const Interceptor &other) = delete;

    Interceptor &operator=(Interceptor &&other) {
        if (this != &other) {
            _captors = other._captors;
            _manager = other._manager;
        }
        return *this;
    }
    Interceptor &operator=(const Interceptor &other) = delete;

    T *popCaptor() {
        if (!_captors.empty()) {
            T *captor = _captors.front();
            _captors.pop();
            return captor;
        }
        return nullptr;
    }

    void clearCaptors() {
        std::queue<T*> empty;
        _captors.swap(empty);
    }

    void spawnEntityEnqueue(T *captor, const char *name, int queue, glm::vec3 pos) {
        _captors.push(captor);
        _manager->spawnEntityEnqueue(name, queue, pos);
    }
    void spawnObjectEnqueue(T *captor, const char *name, int queue, glm::vec3 pos) {
        _captors.push(captor);
        _manager->spawnObjectEnqueue(name, queue, pos);
    }
};

/*

// pops an interceptor's captors and passes the newly allocated reference to their capture() method before
   returning them to a Manager

Interceptor<Basic> interceptor(&obj_manager);

...
   
auto Projectile_allocator = [&interceptor]() -> Object* {
    Projectile *projectile = new Projectile;
    
    Basic *captor = interceptor.popCaptor();
    if (captor)
        captor->captureBasic(projectile);

    return x;
};

// this creates associations between Manager enqueues and the instances that invoke them. this should be fine so
   long as interceptor's wrapper method is always used for spawns that have intercepted allocators; there will be 
   a loss of parity if you invoke an intercepted allocator without using the interceptor's wrapper method.
*/

#endif