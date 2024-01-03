#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

std::string readfile(const char *filename);

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
    Image& operator=(const Image &other);
    ~Image();

    void load(const char *filename);
    void free();

    unsigned char* copyData() const;
    int width();
    int height();
    int components();
    int size();
    bool empty();
};

/* class Partitioner
   Provides and manages unique values on push and removal.
*/
class Partitioner {
    //main ID vector
    std::vector<bool> _IDs;
    //vector of free IDs
    std::vector<int> _freeIDs;

public:
    Partitioner();
    ~Partitioner();

    /* Occupies an index in IDs (use last index from freeIDs if available),
	   and returns the ID.
    */
    int push();

    /* Sets element i to false and pushes its index to freeIDs. */
    void remove(int i);

    /* Returns a vector of all indices that are true. */
    std::vector<int> getUsed();

    /* Empties IDs and freeIDs. */
    void clear();

    /* Returns whether ID i is active or not. */
    bool at(int i);
    bool operator[](int i);

    /* Returns true if active IDs are empty. */
    bool empty();

    /* Returns count of IDs (includes free IDs). */
    unsigned size();
    /* Returns count of free IDs. */
    unsigned freeSize();
    /* Returns size of active IDs (O(n) time). */
    unsigned fillSize();
};

#endif