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

/* class ScriptResourcesProvider
   Templated implementation of EntityScriptProviderInterface that supports the Resource mixin with assignment on instantiation.
   T - type allocated; must be covariant of ScriptInterface
   U - type stored; must be covariant of T and thus covariant of ScriptInterface
   V - type of resource made available
*/
template<class T, class U, typename V>
class ScriptResourceProvider : public ScriptProviderInterface<T, U> {
    V* _resource;
    T* _providerAllocate() override {
        T* t = new T;
        t->Resource<V>::setResource(_resource);
        return t;
    }
    void _providerOnDeallocation(ScriptInterface* script) override {}
public:
    ScriptResourceProvider(ScriptContainer<U>* scriptcontainer, V* resource) : ScriptProviderInterface<T, U>(scriptcontainer), _resource(resource) {}
};

// --------------------------------------------------------------------------------------------------------------------------

/* class EntityScriptResourcesProvider
   Templated implementation of EntityScriptProviderInterface that supports the Resource mixin with assignment on instantiation.
   T - type allocated; must be covariant of EntityScriptInterface
   U - type stored; must be covariant of T and thus covariant of EntityScriptInterface
   V - type of resource made available
*/
template<class T, class U, typename V>
class EntityScriptResourceProvider : public EntityScriptProviderInterface<T, U> {
    V* _resource;
    T* _providerAllocate() override {
        T* t = new T;
        t->Resource<V>::setResource(_resource);
        return t;
    }
    void _providerOnDeallocation(ScriptInterface* script) override {}
public:
    EntityScriptResourceProvider(ScriptContainer<U>* scriptcontainer, V* resource) : EntityScriptProviderInterface<T, U>(scriptcontainer), _resource(resource) {}
};

#endif