#ifndef SCRIPTUTIL_HPP_
#define SCRIPTUTIL_HPP_

#include "script.hpp"

/* abstract class ScriptReceiverInterface<T>
   Specifies an interface for receiving and "losing" subtypes of Script, for use with Providers.
   T - type received; must be covariant of ScriptInterface, allocated by ScriptProviderInterface
*/
template<class T>
class ScriptReceiverInterface {
public:
    virtual void receiveInstance(T* t) = 0;
    virtual void loseInstance(ScriptInterface* script) = 0;
};

/* class ScriptContainer<T>
   Templated container of Scripts that implements ScriptReceiverInterface. Can be passed to 
   ScriptProviderInterface implementations to store allocated instances. It is undefined behavior to 
   access an instance of this class after it is removed from any ScriptProviderInterface instance it 
   was passed to.
   T - type stored; must be covariant of ScriptInterface, allocated by ScriptProviderInterface
*/
template<class T>
class ScriptContainer : public ScriptReceiverInterface<T> {
    std::unordered_map<ScriptInterface*, T*> _Ts;
    T* _last_inst;

public:
    ScriptContainer() : _last_inst(nullptr) {}

    void receiveInstance(T* t) override {
        _Ts[t] = t;
        _last_inst = t;
    }

    void loseInstance(ScriptInterface* script) override {
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

/* class ScriptProviderInterface<T, ...SubTs>
   Templated implementation of the ScriptAllocatorInterface, that can provide subtype references
   of allocated ScriptInterface types to attached ScriptReceiverInterfaces. Supported subtypes must be specified
   per the variadic template argument. It is undefined behavior for a ScriptProviderInterface instance 
   to go out of scope before its passed ScriptReceiverInterfaces.
   T - type allocated; must be covariant of ScriptInterface
   ...SubTs - types to be supported to be passed to ScriptReceiverInterfaces; must be covariant of T and thus covariant of ScriptInterface
*/
template<class T, class ...SubTs>
class ScriptProviderInterface : public ScriptAllocatorInterface {
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

    ScriptInterface* _allocate() override {
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
    ScriptProviderInterface() {}

    template<class U>
    void attach(ScriptReceiverInterface<U>* scriptreceiver) {
        std::get<std::unordered_set<ScriptReceiverInterface<U>*>>(scriptreceivers).insert(scriptreceiver);
    }

    template<class U>
    void detach(ScriptReceiverInterface<U>* scriptreceiver) {
        std::get<std::unordered_set<ScriptReceiverInterface<U>*>>(scriptreceivers).erase(scriptreceiver);
    }
};

/* class GenericScriptProvider<T>
   Generic implementation of ScriptProvider<T>. Allocates instances of T with default constructor.
*/
template<class T>
class GenericScriptProvider : public ScriptProviderInterface<T, T> {
    T* _providerAllocate() override { return new T; }
    void _providerOnDeallocation(ScriptInterface* script) override {}
public:
    GenericScriptProvider() {}
};

#endif