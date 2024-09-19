#ifndef MANAGEDQUEUE_HPP_
#define MANAGEDQUEUE_HPP_

#include <queue>

/* class ManagedQueue
   Wraps an STL queue references of T, and assumes ownership of all inserted
   addresses.
*/
template<typename T>
class ManagedQueue {
    // storage
    std::queue<T*> _Ts;

public:
    ManagedQueue() {}
    ManagedQueue(ManagedQueue &&other) { operator=(std::move(other)); }
    ManagedQueue(const ManagedQueue &other) = delete;
    ~ManagedQueue() { clear(); }
    
    ManagedQueue &operator=(ManagedQueue &&other) {
        _Ts = other._Ts;
        std::queue<T*> empty;
        other._Ts.swap(empty);
    }
    ManagedQueue &operator=(const ManagedQueue &other) = delete;

    /* Deletes and removes all stored references. */
    void clear() {
        while (!_Ts.empty()) {
            T *t = _Ts.front();
            delete t;
            _Ts.pop();
        }
    }

    /* Takes ownership of and inserts a reference into this queue. */
    void push(T *t) { _Ts.push(t); }

    /* Deletes and erases the oldest reference stored. Note that the reference 
       becomes invalid after calling this.
    */
    void pop() {
        T *t = _Ts.front();
        delete t;
        _Ts.pop();
    }

    /* Returns the oldest reference stored. */
    T *front() {
        return _Ts.front();
    }

    /* Moves other ManagedQueue contents into this. */
    void move(ManagedQueue &other) {
        _Ts = other._Ts;
        std::queue<T*> empty;
        other._Ts.swap(empty);
    }

    unsigned size() { return _Ts.size(); }

    bool empty() { return _Ts.empty(); }
};

#endif