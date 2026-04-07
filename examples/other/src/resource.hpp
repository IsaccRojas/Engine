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
*/
template<typename T, typename U>
class ScriptResourceProvider : public ScriptProviderInterface<T> {
    U* _resource;
    T* _providerAllocate() override {
        T* t = new T;
        t->Resource<U>::setResource(_resource);
        return t;
    }
public:
    ScriptResourceProvider(U* resource) : _resource(resource) {}
};

// --------------------------------------------------------------------------------------------------------------------------

/* class EntityScriptResourcesProvider
   Templated implementation of EntityScriptProviderInterface that supports ResourcesMixin.
*/
template<typename T, typename U>
class EntityScriptResourceProvider : public EntityScriptProviderInterface<T> {
    U* _resource;
    T* _providerAllocate() override {
        T* t = new T;
        t->Resource<U>::setResource(_resource);
        return t;
    }
public:
    EntityScriptResourceProvider(U* resource) : _resource(resource) {}
};

#endif