#ifndef RESOURCE_HPP_
#define RESOURCE_HPP_

#include "../../../core/include/entity.hpp"

/* class Resource<T>
   Mix-in class for accessing GlobalResources reference.
*/
template<typename T>
class Resource {
    T *_resource;

public:
    Resource() : _resource(nullptr) {}
    void setResource(T* r) { _resource = r; }
    T* resource() { return _resource; }
};

// --------------------------------------------------------------------------------------------------------------------------

/* class ResourceScriptAllocator
   Templated implementation of ProvidingScriptAllocatorInterface that supports the Resource mixin with assignment on instantiation.
   T - type allocated; must be covariant of ScriptInterface
   U - type of resource made available
*/
template<class T, typename U>
class ResourceScriptAllocator : public ProvidingScriptAllocatorInterface<T> {
    U* _resource;
    T* _providingAllocate() override {
        T* t = new T;
        t->Resource<U>::setResource(_resource);
        return t;
    }
    void _providingOnDeallocation(ScriptInterface* script) override {}
public:
    ResourceScriptAllocator(U* resource) : ProvidingScriptAllocatorInterface<T>(), _resource(resource) {}
};

// --------------------------------------------------------------------------------------------------------------------------

/* class ResourceEntityScriptAllocator
   Templated implementation of ProvidingEntityScriptAllocatorInterface that supports the Resource mixin with assignment on instantiation.
   T - type allocated; must be covariant of EntityScriptInterface
   U - type of resource made available
*/
template<class T, typename U>
class ResourceEntityScriptAllocator : public ProvidingEntityScriptAllocatorInterface<T> {
    U* _resource;
    T* _providingAllocate() override {
        T* t = new T;
        t->Resource<U>::setResource(_resource);
        return t;
    }
    void _providingOnDeallocation(ScriptInterface* script) override {}
public:
    ResourceEntityScriptAllocator(U* resource) : ProvidingEntityScriptAllocatorInterface<T>(), _resource(resource) {}
};

#endif