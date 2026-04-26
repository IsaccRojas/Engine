#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <queue>
#include "C:\dev\include\glm\glm.hpp"

#define PI_UTIL 3.14159265358979323846264338327950288

struct Transform {
    glm::vec3 pos = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    // default copy assignment/construction are fine

    /* Adds position vector of other Transform to this position, and muliplies scale vector of this Transform to other's scale. */
    Transform& apply(Transform& other);
    static Transform apply(const Transform& first, const Transform& second);
};

std::string readfile(const char* filename);

/* class InactiveIntException
   This exception is thrown when an inactive integer provided by an IntGenerator is accessed.
*/
class InactiveIntException : public std::logic_error {
public:
    InactiveIntException();
};

// --------------------------------------------------------------------------------------------------------------------------

class Image {
    unsigned char* _data;
    int _w;
    int _h;
    int _components;
    int _size;
public:
    Image(const char* filename);
    Image(const Image& other);
    Image();
    Image(Image&& other) = delete;
    ~Image();

    Image& operator=(const Image& other);
    Image& operator=(Image&& other) = delete;

    void load(const char* filename);
    void free();

    unsigned char* copyData() const;
    int width();
    int height();
    int components();
    int size();
    bool empty();
};

// --------------------------------------------------------------------------------------------------------------------------

/* class IntGenerator
   Provides and manages unique integers on push and removal.
*/
class IntGenerator {
    //main ID vector
    std::vector<bool> _ids;
    //queue of free IDs
    std::queue<unsigned> _free_ids;

public:
    IntGenerator();
    ~IntGenerator();

    // default copy assignment/construction are fine

    /* Occupies an index in IDs (use last index from freeIDs if available),
	   and returns the ID.
    */
    unsigned push();

    /* Sets element i to false and pushes its index to freeIDs. */
    void remove(unsigned i);

    /* Returns a vector of all indices that are true. */
    std::vector<unsigned> getUsed();

    /* Empties IDs and freeIDs. */
    void clear();

    /* Returns whether ID i is active or not. */
    bool at(unsigned i);
    bool operator[](unsigned i);

    /* Returns true if active IDs are empty. */
    bool empty();

    /* Returns count of IDs (includes free IDs). */
    unsigned size();
    /* Returns count of free IDs. */
    unsigned freeSize();
    /* Returns count of active IDs. */
    unsigned activeSize();
};

// --------------------------------------------------------------------------------------------------------------------------

template<typename T>
class IntgenVector {
    std::vector<T> _data;
    IntGenerator _intgen;

public:
    /* Note that construction with a capacity and value does not 
       push to the internal IntGenerator, so active size will still be
       initialized as 0.
    */
    IntgenVector(unsigned capacity, T t) : _data(capacity, t) {}
    IntgenVector() {}

    // default copy assignment/construction are fine
    
    unsigned push(T t) {
        unsigned i = _intgen.push();
        if (i >= _data.size())
            _data.push_back(t);
        else
            _data[i] = t;
        return i;
    }

    void remove(int i) {
        _intgen.remove(i);
    }

    T get(int i) {
        if (!_intgen[i])
            throw InactiveIntException();
        
        return _data[i];
    }

    bool active(int i) {
        return _intgen[i];
    }

    unsigned activeSize() {
        return _intgen.activeSize();
    }

    /* Returns all data (including inactive elements). */
    std::vector<T>& data() { return _data; }
    IntGenerator& intgen() { return _intgen; }
};

// --------------------------------------------------------------------------------------------------------------------------

/* Checks if provided string ends with the provided suffix.
*/
bool endsWith(const std::string& str, const std::string& suffix);

/* Checks if provided string starts with the provided prefix
*/
bool startsWith(const std::string& str, const std::string& prefix);

// --------------------------------------------------------------------------------------------------------------------------

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
    std::unordered_map<void*, T*>::iterator begin() {
        return _Ts.begin();
    }

    /* Returns internal unordered map end() iterator. Key is a void pointer cast of value. */
    std::unordered_map<void*, T*>::iterator end() {
        return _Ts.end();
    }

    /* Erases all entries. */
    void clear() {
        _Ts.clear();
    }
};

// --------------------------------------------------------------------------------------------------------------------------

/* class RefProviderInterface<T, ...BaseTs>
   Templated implementation of RefReceiverInterface, that can provide covariant type references of T to attached
   RefReceiverInterface references. Supported types for must be specified per the variadic template argument.
   The scope of any attached RefReceiverInterface must be equal to or a subset of this instance; it is undefined behavior
   to make calls on this instance or attached receiver instances otherwise.
   T - type received as a reference
   ...BaseTs - types to be supported to be passed to attached RefReceiverInterfaces; must be covariant of T
*/
template<class T, class ...BaseTs>
class RefProviderInterface : RefReceiverInterface<T> {
    std::tuple<std::unordered_set<RefReceiverInterface<BaseTs>*>...> refreceiverlists;

    void _passToReceivers(T* t) {
        // expand refreceiverlists tuple
        std::apply(
            [&](auto&& ...tuple) {
                // define lambda to iterate over each list in tuple
                auto iterate_on = [&](auto& refreceiverlist) {
                    for (auto& refreceiver : refreceiverlist)
                        refreceiver->receiveInstance(t);
                };

                // fold expression
                (iterate_on(tuple), ...);
            },
            refreceiverlists
        );
    };

    void _passToReceiversAddr(void* vt) {
        // expand refreceiverlists tuple
        std::apply(
            [&](auto&& ...tuple) {
                // define lambda to iterate over each list in tuple
                auto iterate_on = [&](auto& refreceiverlist) {
                    for (auto& refreceiver : refreceiverlist)
                        refreceiver->receiveInstanceAddr(vt);
                };

                // fold expression
                (iterate_on(tuple), ...);
            },
            refreceiverlists
        );
    };

public:
    RefProviderInterface() : RefReceiverInterface<T>() {}

    void receiveInstance(T* t) override {
        _passToReceivers(t);
        providerReceiveInstance(t);
    }

    void receiveInstanceAddr(void* vt) override {
        providerReceiveInstanceAddr(vt);
        _passToReceiversAddr(vt);
    }

    virtual void providerReceiveInstance(T* t) = 0;
    virtual void providerReceiveInstanceAddr(void *vt) = 0;

    template<class U>
    void attach(RefReceiverInterface<U>* refreceiver) {
        std::get<std::unordered_set<RefReceiverInterface<U>*>>(refreceiverlists).insert(refreceiver);
    }

    template<class U>
    void detach(RefReceiverInterface<U>* refreceiver) {
        std::get<std::unordered_set<RefReceiverInterface<U>*>>(refreceiverlists).erase(refreceiver);
    }
};

// --------------------------------------------------------------------------------------------------------------------------

/* class GenericRefProvider<T, ...BaseTs>
   Generic implementation of RefProviderInterface<T, ...BaseTs>.
   T - type received as a reference
   ...BaseTs - types to be supported to be passed to attached RefReceiverInterfaces; must be covariant of T
*/

template<class T, class ...BaseTs>
class GenericRefProvider : public RefProviderInterface<T, BaseTs...> {
public:
    GenericRefProvider() : RefProviderInterface<T, BaseTs...>() {}
    void providerReceiveInstance(T* t) override {}
    void providerReceiveInstanceAddr(void *vt) override {}
};

#endif