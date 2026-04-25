#ifndef SCRIPTUTIL_HPP_
#define SCRIPTUTIL_HPP_

#include "script.hpp"

/* class ScriptContainer<T>
   Templated container of Scripts. Can be passed to ScriptProviderInterface implementations to store
   allocated instances. It is undefined behavior to access an instance of this class after it is removed
   from any ScriptProviderInterface instance it was passed to.
   T - type stored; must be covariant of ScriptInterface, allocated by ScriptProviderInterface
*/
template<class T>
class ScriptContainer {
   std::unordered_map<ScriptInterface*, T*> _Ts;
   T* _last_inst;

public:
   ScriptContainer() : _last_inst(nullptr) {}

   void insertInstance(T* t) {
      _Ts[t] = t;
      _last_inst = t;
   }

   void removeInstance(ScriptInterface* script) {
      _Ts.erase(script);
      if (script == (ScriptInterface*)(_last_inst))
         _last_inst = nullptr;
   }

   T* getInstance(ScriptInterface* script) {
      if (_Ts.empty())
         throw std::runtime_error("Attempt to get instance from empty ScriptContainer");
      if (_Ts.find(script) == _Ts.end())
         throw std::runtime_error("Attempt to get instance from ScriptContainer with address it does not contain");
      return _Ts[script];
   }

   /* Returns the last instance inserted. Returns nullptr if last insertion was removed before calling this. */
   T* getLastInstance() {
      if (_Ts.empty())
         throw std::runtime_error("Attempt to get last instance from empty ScriptContainer");
      return _last_inst;
   }

   void clear() {
      _Ts.clear();
   }
};

/* class ScriptProviderInterface<T, U>
   Templated implementation of the ScriptAllocatorInterface, that can provide subtype references
   of allocated ScriptInterface types to a ScriptContainer. It is undefined behavior for a ScriptProviderInterface
   instance to go out of scope before its passed ScriptContainer.
   T - type allocated; must be covariant of ScriptInterface
   U - type to be stored by ScriptContainer; must be covariant of T and thus covariant of ScriptInterface
*/
template<class T>
class ScriptProviderInterface : public ScriptAllocatorInterface {
   ScriptContainer<T>* _scriptcontainer;

   ScriptInterface* _allocate() override {
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
   ScriptProviderInterface(ScriptContainer<T>* scriptcontainer) : _scriptcontainer(scriptcontainer) {}
};

/* class GenericScriptProvider<T>
   Generic implementation of ScriptProvider<T>. Allocates instances of T with default constructor.
*/
template<class T>
class GenericScriptProvider : public ScriptProviderInterface<T> {
   T* _providerAllocate() override { return new T; }
   void _providerOnDeallocation(ScriptInterface* script) override {}
public:
   GenericScriptProvider(ScriptContainer<T>* scriptcontainer) : ScriptProviderInterface<T>(scriptcontainer) {}
};

#endif