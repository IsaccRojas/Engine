#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <queue>
#include <unordered_set>
#include <list>
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

template <class T>
class RefContainer;

/* abstract class RefReceiverInterface<T>
   Specifies an interface for receiving pointers of type T.
   T - type received as a reference
*/
template<class T>
class RefReceiverInterface {
public:
    virtual ~RefReceiverInterface() {}
    virtual void receiveInstance(T* t) = 0;
    virtual void receiveInstanceAddr(void* vt) = 0;
};

/* class RefReceiverConverter<BaseT, ConvertT>
   Templated converter class that wraps a RefReceiverInterface reference of type BaseT with implementations of 
   RefReceiverInterface of type ConvertT.
   BaseT - Base type of receiver
   ConvertT - Type to convert into BaseT
*/
template<class BaseT, class ConvertT>
class RefReceiverConverter : public RefReceiverInterface<ConvertT> {
	RefReceiverInterface<BaseT>* _refreceiver;
public:
	RefReceiverConverter(RefReceiverInterface<BaseT>* refreceiver) : _refreceiver(refreceiver) {}
    RefReceiverConverter() : _refreceiver(nullptr) {}
	void receiveInstance(ConvertT* t) override {
        if (!_refreceiver)
            throw std::runtime_error("Attempt to receive instance on converter with null receiver reference");
		_refreceiver->receiveInstance(t);
	}
    void receiveInstanceAddr(void* vt) override {
        if (!_refreceiver)
            throw std::runtime_error("Attempt to receive instance handle with null receiver reference");
        _refreceiver->receiveInstanceAddr(vt);
    }
    void setReceiver(RefReceiverInterface<BaseT>* refreceiver) {
        _refreceiver = refreceiver;
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
    std::unordered_map<uintptr_t, T*> _Ts;
    T* _last_inst;

public:
    RefContainer() : RefReceiverInterface<T>(), _last_inst(nullptr) {}

    /* Stores instance passed. */
    void receiveInstance(T* t) override {
        _Ts[reinterpret_cast<uintptr_t>(t)] = t;
        _last_inst = t;
    }

    /* Removes instance passed. */
    void receiveInstanceAddr(void* vt) override {
        _Ts.erase(reinterpret_cast<uintptr_t>(vt));
        if (vt == (void*)(_last_inst))
            _last_inst = nullptr;
    }

    /* Returns instance of type T corresponding to address vt. */
    T* getInstance(void* vt) {
        if (_Ts.empty())
            throw std::runtime_error("Attempt to get instance from empty Container");
        if (_Ts.find(reinterpret_cast<uintptr_t>(vt)) == _Ts.end())
            throw std::runtime_error("Attempt to get instance from Container with address it does not contain");
        return _Ts[reinterpret_cast<uintptr_t>(vt)];
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

/* class RefProvider<T>
   Templated implementation of RefReceiverInterface, that can provide covariant type references of T to attached
   RefReceiverInterface references. Supported types for must be specified per the variadic template argument.
   T - type received as a reference
*/
template<class T>
class RefProvider : RefReceiverInterface<T> {
    struct _ConverterData {
        RefReceiverInterface<T>* converter;
        uintptr_t inner_receiver;
    };

    // attached receivers and converters
    std::unordered_set<RefReceiverInterface<T>*> _receivers;
    std::list<_ConverterData> _converters;

    void _passToReceivers(T* t) {
        for (auto& r : _receivers)
            r->receiveInstance(t);
        for (auto& c : _converters)
            c.converter->receiveInstance(t);
    };

    void _passToReceiversAddr(void* vt) {
        for (auto& r : _receivers)
            r->receiveInstanceAddr(vt);
        for (auto& c : _converters)
            c.converter->receiveInstanceAddr(vt);
    };

    typename std::unordered_set<RefReceiverInterface<T>*>::iterator _getReceiverIter(void* refreceiver) {
        // find handle whose inner uintptr_t value matches the provided receiver
        for (auto iter = _receivers.begin(); iter != _receivers.end(); iter++)
            if (reinterpret_cast<uintptr_t>(*iter) == reinterpret_cast<uintptr_t>(refreceiver))
                return iter;
        return _receivers.end();
    }

    typename std::list<_ConverterData>::iterator _getConverterDataIter(void* refreceiver) {
        // find handle whose inner uintptr_t value matches the provided receiver
        for (auto iter = _converters.begin(); iter != _converters.end(); iter++)
            if (iter->inner_receiver == reinterpret_cast<uintptr_t>(refreceiver))
                return iter;
        return _converters.end();
    }

public:
    RefProvider() : RefReceiverInterface<T>() {}
    ~RefProvider() {
        for (auto& c : _converters)
            delete c.converter;
    }

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
        if (has(refreceiver))
            throw std::runtime_error("Attempt to attach RefReceiver to Provider it was already attached to");

        _receivers.insert(refreceiver);
    }

    /* Attaches RefReceiver of a specified compatible type. */
    template<class U>
    void attachType(RefReceiverInterface<U>* refreceiver) {
        if (has(refreceiver))
            throw std::runtime_error("Attempt to attach RefReceiver to Provider it was already attached to");

        RefReceiverConverter<U, T>* converter = new RefReceiverConverter<U, T>(refreceiver);
        _converters.push_back({converter, reinterpret_cast<uintptr_t>(refreceiver)});
    }

    /* Detaches RefReceiver, which will no longer receive instances from this provider. */
    void detach(void* refreceiver) {
        // try removing receiver first, then converter
        auto iter = _getReceiverIter(refreceiver);
        if (iter != _receivers.end())
            _receivers.erase(iter);

        else {
            auto iter = _getConverterDataIter(refreceiver);
            if (iter == _converters.end())
                throw std::runtime_error("Attempt to detach RefReceiver from Provider it is not attached to");
            
            delete iter->converter;
            _converters.erase(iter);
        }
    }

    bool has(void* refreceiver) {
        return ((_getReceiverIter(refreceiver) != _receivers.end()) || (_getConverterDataIter(refreceiver) != _converters.end()));
    }
};

template<class T>
class RefProviderView {
    RefProvider<T>* _provider;
public:
    RefProviderView(RefProvider<T>* provider) : _provider(provider) {}
    
    void attach(RefReceiverInterface<T>* refreceiver) {
        _provider->attach(refreceiver);
    }

    template<class U>
    void attachType(RefReceiverInterface<U>* refreceiver) {
        _provider->template attachType<U>(refreceiver);
    }

    void detach(void* refreceiver) {
        _provider->detach(refreceiver);
    }
};

#endif