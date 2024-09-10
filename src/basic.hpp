#ifndef BASIC_HPP_
#define BASIC_HPP_

#include "../core/include/object.hpp"

/* abstract class ObjectWatcherInterface
   wraps an ObjectManager, and pushes to a queue of references on spawns This creates associations between Manager 
   enqueues and the instances that invoke them. It is undefined behavior to enqueue a Script covariant without using
   a Watcher's wrapper enqueue methods, if that covariant is using a Watcher as an allocator.
*/
template<class T>
class WatcherInterface : public ObjectAllocatorInterface {
    friend T;

    std::queue<T*> _captors;
    ObjectManager *_manager;

    T *_popCaptor() {
        if (!_captors.empty()) {
            T *captor = _captors.front();
            _captors.pop();
            return captor;
        }
        return nullptr;
    }

protected:
    WatcherInterface(ObjectManager *manager) : _manager(manager) {}
    WatcherInterface(const WatcherInterface &other) = delete;
    WatcherInterface &operator=(const WatcherInterface &other) = delete;

public:
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

    Object *allocate() override {
        T *captive = allocateWatchedType();

        T *captor = _popCaptor();
        if (captor)
            captor->_capture(captive);
        
        return captive;
    }

    virtual T *allocateWatchedType() = 0;
};

// --------------------------------------------------------------------------------------------------------------------------

/* abstract class WatcherInterface
   wraps an ObjectManager, and pushes to a queue of references on spawns This creates associations between Manager 
   enqueues and the instances that invoke them. This should be fine so long as Watcher's wrapper methods are 
   always used for spawns that use the Watcher as an allocator; there will be a loss of parity if you invoke 
   a Watcher's allocator without using the Watcher's wrapped enqueue methods.
*/

template<class T>
class WatcherInterface : public ObjectAllocatorInterface {
    friend T;

    std::queue<T*> _captors;
    ObjectManager *_manager;

    T *_popCaptor() {
        if (!_captors.empty()) {
            T *captor = _captors.front();
            _captors.pop();
            return captor;
        }
        return nullptr;
    }

protected:
    WatcherInterface(ObjectManager *manager) : _manager(manager) {}
    WatcherInterface(const WatcherInterface &other) = delete;
    WatcherInterface &operator=(const WatcherInterface &other) = delete;

public:
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

    Object *allocate() override {
        T *captive = allocateWatchedType();

        T *captor = _popCaptor();
        if (captor)
            captor->_capture(captive);
        
        return captive;
    }

    virtual T *allocateWatchedType() = 0;
};

// --------------------------------------------------------------------------------------------------------------------------

template<class T>
class CaptorInterface {
    WatcherInterface<T> *_watcher;
protected:
    virtual void _capture(T *captive) = 0;

    CaptorInterface(WatcherInterface<T> *watcher) : _watcher(watcher) {}
    CaptorInterface *getWatcher() { return _watcher; }
};

// --------------------------------------------------------------------------------------------------------------------------

class Projectile1 : public Object {
    void _initObject() override {}
    void _baseObject() override {}
    void _killObject() override {}
    void _collisionObject(Box *box) override {}
public:
    Projectile1() : Object() {}
};

class Projectile2 : public Object {
    void _initObject() override {}
    void _baseObject() override {}
    void _killObject() override {}
    void _collisionObject(Box *box) override {}
public:
    Projectile2() : Object() {}
};

class Player : public Object, public CaptorInterface<Projectile1>, public CaptorInterface<Projectile2> {
    void _initObject() override {}
    void _baseObject() override {}
    void _killObject() override {}
    void _collisionObject(Box *box) override {}

    void _capture(Projectile1 *captive) override {}
    void _capture(Projectile2 *captive) override {}
public:
    Player(WatcherInterface<Projectile1> *watcher1, WatcherInterface<Projectile2> *watcher2) : 
        Object(), 
        CaptorInterface<Projectile1>(watcher1), 
        CaptorInterface<Projectile2>(watcher2)  
    {}
};

class Projectile1Watcher : public WatcherInterface<Projectile1> {
public:
    Projectile1 *allocateWatchedType() { return new Projectile1; }
};

class Projectile2Watcher : public WatcherInterface<Projectile2> {
public:
    Projectile2 *allocateWatchedType() { return new Projectile2; }
};

#endif