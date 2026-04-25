#ifndef ENTITYUTIL_HPP_
#define ENTITYUTIL_HPP_

#include "entity.hpp"



/* class EntityScriptProviderInterface<T, ...SubTs>
   Templated implementation of the EntityScriptAllocatorInterface, that can provide subtype references
   of allocated ScriptInterface types to attached ScriptReceiverInterfaces. Supported subtypes must be specified
   per the variadic template argument. It is undefined behavior for an EntityScriptProviderInterface instance 
   to go out of scope before its passed ScriptReceiverInterfaces.
   T - type allocated; must be covariant of EntityScriptInterface
   ...SubTs - types to be supported to be passed to ScriptReceiverInterfaces; must be covariant of T and thus covariant of EntityScriptInterface
*/
template<class T, class ...SubTs>
class EntityScriptProviderInterface : public EntityScriptAllocatorInterface {
   std::tuple<std::unordered_set<ScriptReceiverInterface<SubTs>*>...> scriptreceivers;
   
   void _passToReceivers(T* t) {
      // expand scriptreceivers tuple
      std::apply(
         [&](auto&& ...tuple) {
               // define lambda to iterate over each list in tuple
               auto iterate_on = [&](auto& scriptreceiverlist) {
                  for (auto& scriptreceiver : scriptreceiverlist)
                     scriptreceiver->receiveInstance(t);
               };

               // fold expression
               (iterate_on(tuple), ...);
         },
         scriptreceivers
      );
   };

   void _loseOnReceivers(ScriptInterface* script) {
      // expand scriptreceivers tuple
      std::apply(
         [&](auto&& ...tuple) {
               // define lambda to iterate over each list in tuple
               auto iterate_on = [&](auto& scriptreceiverlist) {
                  for (auto& scriptreceiver : scriptreceiverlist)
                     scriptreceiver->loseInstance(script);
               };

               // fold expression
               (iterate_on(tuple), ...);
         },
         scriptreceivers
      );
   };

   EntityScriptInterface* _allocate() override {
      T* t = _providerAllocate();
      _passToReceivers(t);
      return t;
   }

   void _onDeallocation(ScriptInterface* script) override {
      _providerOnDeallocation(script);
      _loseOnReceivers(script);
   }

protected:
   virtual T* _providerAllocate() = 0;
   virtual void _providerOnDeallocation(ScriptInterface* script) = 0;

public:
   EntityScriptProviderInterface() {}

   template<class U>
   void attach(ScriptReceiverInterface<U>* scriptreceiver) {
      std::get<std::unordered_set<ScriptReceiverInterface<U>*>>(scriptreceivers).insert(scriptreceiver);
   }

   template<class U>
   void detach(ScriptReceiverInterface<U>* scriptreceiver) {
      std::get<std::unordered_set<ScriptReceiverInterface<U>*>>(scriptreceivers).erase(scriptreceiver);
   }
};

/* class GenericEntityScriptProvider<T>
   Generic implementation of EntityScriptProviderInterface<T>. Allocates instances of T with default constructor.
*/
template<class T>
class GenericEntityScriptProvider : public EntityScriptProviderInterface<T, T> {
   T* _providerAllocate() override { return new T; }
   void _providerOnDeallocation(ScriptInterface* script) override {}
public:
   GenericEntityScriptProvider() {}
};

#endif