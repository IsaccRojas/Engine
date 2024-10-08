#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <queue>

#define PI_UTIL 3.14159265358979323846264338327950288

std::string readfile(const char *filename);

/* class InactiveIntException
   This exception is thrown when an inactive integer provided by an IntGenerator is accessed.
*/
class InactiveIntException : public std::logic_error {
public:
    InactiveIntException();
};

class Image {
    unsigned char *_data;
    int _w;
    int _h;
    int _components;
    int _size;
public:
    Image(const char *filename);
    Image(const Image &other);
    Image();
    Image(Image &&other) = delete;
    ~Image();

    Image& operator=(const Image &other);
    Image& operator=(Image &&other) = delete;

    void load(const char *filename);
    void free();

    unsigned char* copyData() const;
    int width();
    int height();
    int components();
    int size();
    bool empty();
};

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
    std::vector<T> &data() { return _data; }
    IntGenerator &intgen() { return _intgen; }
};

/* Checks if provided string ends with the provided suffix.
*/
bool endsWith(const std::string& str, const std::string& suffix);

/* Checks if provided string starts with the provided prefix
*/
bool startsWith(const std::string& str, const std::string& prefix);

#endif