#include "util.hpp"
#include <unordered_set>

// Allocator/Invoker layer ----------------------------------------------------------------------------------------------------------

class Base {};

class Invoker;
class AllocatorInterface {
    friend Invoker;
protected:
    /* - tag - user value that can be used to interpret this allocation invocation externally */
    virtual Base *_allocate(int tag) = 0;
};

class Invoker {
public:
    Base *call(AllocatorInterface *allocator, int tag) {
        return allocator->_allocate(tag);
    }
};

// Servicer/Receiver layer ----------------------------------------------------------------------------------------------------------

template<typename T>
class Servicer;

template<class T>
class Receiver {
    friend Servicer<T>;
    int _channel;
protected:
    virtual void _receive(T *t) = 0;
    Receiver() : _channel(-1) {}
public:
    void setChannel(int channel) { _channel = channel; }
    int getChannel() { return _channel; }
};

template<class T>
class Servicer : public AllocatorInterface {
    std::unordered_set<Receiver<T>*> _receivers;
    
    Base * _allocate(int tag) override {
        // interpret tag as channel

        T *t = _allocateInstance();

        // if channel is non-negative, deliver instance
        if (tag >= 0) {

            // if channel matches, deliver
            for (const auto& receiver: _receivers)
                if (receiver->_channel == tag)
                    receiver->_receive(t);
        }
        return t;
    }
protected:
    virtual T *_allocateInstance() { return new T; }
public:
    void subscribe(Receiver<T> *receiver) {
        _receivers.insert(receiver);
    }
};