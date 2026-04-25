#include "entity.hpp"

/* class EntityScriptProviderInterface<T, U>
   Templated implementation of the EntityScriptAllocatorInterface, that can provide subtype references
   of allocated EntityScriptInterface types to a ScriptContainer. It is undefined behavior for an 
   EntityScriptProviderInterface instance to go out of scope before its passed ScriptContainer.
   T - type allocated; must be covariant of EntityScriptInterface
   U - type to be stored by ScriptContainer; must be covariant of T and thus covariant of EntityScriptInterface
*/
template<class T, class U>
class EntityScriptProviderInterface : public EntityScriptAllocatorInterface {
   ScriptContainer<U>* _scriptcontainer;

   EntityScriptInterface* _allocate() override {
      T* t = _providerAllocate();
      _scriptcontainer->insertInstance(t);
      return t;
   }

   void _onDeallocation(ScriptInterface* script) override {
      _providerOnDeallocation(script);
      _scriptcontainer->removeInstance(script);
   }

protected:
   virtual T* _providerAllocate() = 0;

   virtual void _providerOnDeallocation(ScriptInterface* script) = 0;

public:
   EntityScriptProviderInterface(ScriptContainer<U>* scriptcontainer) : _scriptcontainer(scriptcontainer) {}
};

/* class GenericEntityScriptProvider<T>
   Generic implementation of EntityScriptProviderInterface<T, U>. Allocates instances of T with default constructor.
*/
template<class T, class U>
class GenericEntityScriptProvider : public EntityScriptProviderInterface<T, U> {
   T* _providerAllocate() override { return new T; }
   void _providerOnDeallocation(ScriptInterface* script) override {}
public:
   GenericEntityScriptProvider(ScriptContainer<U>* scriptcontainer) : EntityScriptProviderInterface<T, U>(scriptcontainer) {}
};