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
   U - type of resource made available
   ...SubTs - variadic argument for subtypes to support ScriptReceivers of
*/
template<class T, typename U>
class ScriptResourceProvider : public ScriptProviderInterface<T> {
    U* _resource;
    T* _providerAllocate() override {
        T* t = new T;
        t->Resource<U>::setResource(_resource);
        return t;
    }
    void _providerOnDeallocation(ScriptInterface* script) override {}
public:
    ScriptResourceProvider(U* resource) : ScriptProviderInterface<T>(), _resource(resource) {}
};

// --------------------------------------------------------------------------------------------------------------------------

/* class EntityScriptResourcesProvider
   Templated implementation of EntityScriptProviderInterface that supports the Resource mixin with assignment on instantiation.
   T - type allocated; must be covariant of EntityScriptInterface
   U - type of resource made available
*/
template<class T, typename U>
class EntityScriptResourceProvider : public EntityScriptProviderInterface<T> {
    U* _resource;
    T* _providerAllocate() override {
        T* t = new T;
        t->Resource<U>::setResource(_resource);
        return t;
    }
    void _providerOnDeallocation(ScriptInterface* script) override {}
public:
    EntityScriptResourceProvider(U* resource) : EntityScriptProviderInterface<T>(), _resource(resource) {}
};

#endif