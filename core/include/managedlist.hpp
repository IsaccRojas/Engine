#ifndef MANAGEDLIST_HPP_
#define MANAGEDLIST_HPP_

#include <list>

/* class ManagedList
   Wraps a non-const STL list references of T, and assumes ownership of all inserted
   addresses.
*/
template<typename T>
class ManagedList {
    // storage
    std::list<T*> _Ts;

public:
    ManagedList() {}
    ManagedList(ManagedList &&other) { operator=(std::move(other)); }
    ManagedList(const ManagedList &other) = delete;
    ~ManagedList() { clear(); }
    
    ManagedList &operator=(ManagedList &&other) {
        clear();
        _Ts = other._Ts;
        other._Ts.clear();
        return *this;
    }
    ManagedList &operator=(const ManagedList &other) = delete;

    /* Deletes and removes all stored references. */
    void clear() {
        for (auto &t : _Ts)
            delete t;
    }

    /* Takes ownership of and inserts a reference into this list. Returns iterator
       referring to the element's location.
    */
    typename std::list<T*>::iterator push_back(T *t) {
        _Ts.push_back(t);
        auto iter = _Ts.end();
        iter--;
        return iter;
    }

    /* Deletes and erases the reference at the provided iterator from the list. Note that the reference 
       becomes invalid after calling this.
    */
    void erase(typename std::list<T*>::iterator elem) {
        T *t = *elem;
        delete t;
        _Ts.erase(elem);
    }

    /* Returns internal list begin() iterator. */
    typename std::list<T*>::iterator begin() {
        return _Ts.begin();
    }

    /* Returns internal list end() iterator. */
    typename std::list<T*>::iterator end() {
        return _Ts.end();
    }

    unsigned size() { return _Ts.size(); }
};

#endif