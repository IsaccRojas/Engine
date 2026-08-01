#include <iostream>
#include <tuple>
#include <list> 
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>

template <class T>
class RefContainer;

/* abstract class RefReceiverInterface<T>
   Specifies an interface for receiving pointers of type T.
   T - type received as a reference
*/
template<class T>
class RefReceiverInterface {
public:
    virtual void receiveInstance(T* t) = 0;
    virtual void receiveInstanceAddr(void* vt) = 0;
};

template<class BaseT, class ConvertT>
class RefReceiverConverter : public RefReceiverInterface<ConvertT> {
	RefReceiverInterface<BaseT>* _refreceiver;
public:
	RefReceiverConverter(RefReceiverInterface<BaseT>* refreceiver) : _refreceiver(refreceiver) {}
	void receiveInstance(ConvertT* t) override {
		_refreceiver->receiveInstance(t);
	}
    void receiveInstanceAddr(void* vt) override {
        _refreceiver->receiveInstanceAddr(vt);
    }
};

// --------------------------------------------------------------------------------------------------------------------------

/* class RefContainer<T>
   Templated container of references of T that implements RefReceiverInterface. Defines receiveInstance(T*) has an insertion,
   and receiveInstanceAddr(void*) as an erasure. Does not take ownership.
   T - type stored as a reference
*/
template<class T>
class RefContainer : public RefReceiverInterface<T> {
    std::unordered_map<void*, T*> _Ts;
    T* _last_inst;

public:
    RefContainer() : RefReceiverInterface<T>(), _last_inst(nullptr) {}

    /* Stores instance passed. */
    void receiveInstance(T* t) override {
        _Ts[t] = t;
        _last_inst = t;
    }

    /* Removes instance passed. */
    void receiveInstanceAddr(void* vt) override {
        _Ts.erase(vt);
        if (vt == (void*)(_last_inst))
            _last_inst = nullptr;
    }

    /* Returns instance of type T corresponding to address vt. */
    T* getInstance(void* vt) {
        if (_Ts.empty())
            throw std::runtime_error("Attempt to get instance from empty Container");
        if (_Ts.find(vt) == _Ts.end())
            throw std::runtime_error("Attempt to get instance from Container with address it does not contain");
        return _Ts[vt];
    }

    /* Returns the last instance inserted. Returns nullptr if last insertion was removed before calling this. */
    T* getLastInstance() {
        if (_Ts.empty())
            throw std::runtime_error("Attempt to get last instance from empty Container");
        return _last_inst;
    }

    /* Returns internal unordered map begin() iterator. Key is a void pointer cast of value. */
    typename std::unordered_map<void*, T*>::iterator begin() {
        return _Ts.begin();
    }

    /* Returns internal unordered map end() iterator. Key is a void pointer cast of value. */
    typename std::unordered_map<void*, T*>::iterator end() {
        return _Ts.end();
    }

    /* Erases all entries. */
    void clear() {
        _Ts.clear();
    }
};

// --------------------------------------------------------------------------------------------------------------------------

/* class RefProvider<T, ...BaseTs>
   Templated implementation of RefReceiverInterface, that can provide covariant type references of T to attached
   RefReceiverInterface references. Supported types for must be specified per the variadic template argument.
   T - type received as a reference
   ...BaseTs - types to be supported to be passed to attached RefReceiverInterfaces; must be covariant of T
*/
template<class T>
class RefProvider : RefReceiverInterface<T> {
    std::unordered_set<RefReceiverInterface<T>*> _refreceivers;

    void _passToReceivers(T* t) {
        for (auto& refreceiver : _refreceivers)
            refreceiver->receiveInstance(t);
    };

    void _passToReceiversAddr(void* vt) {
        for (auto& refreceiver : _refreceivers)
            refreceiver->receiveInstanceAddr(vt);
    };

public:
    RefProvider() : RefReceiverInterface<T>() {}

    /* Passes instance to all attached RefReceivers. */
    void receiveInstance(T* t) override {
        _passToReceivers(t);
    }

    /* Passes instance address to all attached RefReceivers. */
    void receiveInstanceAddr(void* vt) override {
        _passToReceiversAddr(vt);
    }

    /* Attaches RefReceiver, which will receive instances passed to invocations of receiveInstance(T*) on this provider. */
    void attach(RefReceiverInterface<T>* refreceiver) {
        _refreceivers.insert(refreceiver);
    }

    /* Detaches RefReceiver, which will no longer receive instances from this provider. */
    void detach(RefReceiverInterface<T>* refreceiver) {
        _refreceivers.erase(refreceiver);
    }
};

class A {
public:
	virtual ~A() {}
	virtual void print() { std::cout << "printing from A" << std::endl; }
};

class AA : public A {
public:
	~AA() {}
	void print() override { std::cout << "printing from AA" << std::endl; }
};

class AB : public A {
public:
	~AB() {}
	void print() override { std::cout << "printing from AB" << std::endl; }
};

int main()
{
    RefContainer<A> rc_a;
    RefContainer<AB> rc_ab;
    RefProvider<AB> rp_ab;

    rp_ab.attach(new RefReceiverConverter<A, AB>(&rc_a));
    rp_ab.attach(&rc_ab);

    AB* ab = new AB;
    
    rp_ab.receiveInstance(ab);
    
    std::cout << "invoking from RefContainer<A>" << std::endl;
    rc_a.getLastInstance()->print();

    std::cout << "invoking from RefContainer<AB>" << std::endl;
    rc_ab.getLastInstance()->print();

    std::cout << "done" << std::endl;
	return 0;
}