#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#define PI_UTIL 3.14159265358979323846264338327950288

std::string readfile(const char *filename);

/* class InactiveIDException
   This exception is thrown when an inactive ID is accessed in a SlotVec instance.
*/
class InactiveIDException : public std::logic_error {
public:
    InactiveIDException();
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
    Image(Image &&other) = delete;
    
    Image();

    Image& operator=(const Image &other);
    Image& operator=(Image &&other) = delete;

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

/* class SlotVec
   Provides and manages unique addresses on push and removal.
*/
class SlotVec {
    //main ID vector
    std::vector<bool> _ids;
    //vector of free IDs
    std::vector<int> _free_ids;

public:
    SlotVec();
    ~SlotVec();

    // default copy assignment/construction are fine

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

/* Checks if provided string ends with the provided suffix.
*/
bool endsWith(const std::string& str, const std::string& suffix);

/* Checks if provided string starts with the provided prefix
*/
bool startsWith(const std::string& str, const std::string& prefix);

#endif