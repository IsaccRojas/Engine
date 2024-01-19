#include "../include/util.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"

std::string readfile(const char *filename) {
    std::ifstream infile(filename);
    std::stringstream buf;
    buf << infile.rdbuf();
    return buf.str();
}

Image::Image(const char *filename) {
    load(filename);
}
Image::Image(const Image &other) :
    _data(other.copyData()),
    _w(other._w),
    _h(other._h),
    _components(other._components),
    _size(other._size)
{}
Image::Image() : _data(NULL), _w(0), _h(0), _size(0) {};
Image& Image::operator=(const Image &other) {
    _data = other.copyData();
    _w = other._w;
    _h = other._h;
    _components = other._components;
    _size = other._size;
    return *this;
}
Image::~Image() {
    free();
}

void Image::load(const char *filename) {
    _data = stbi_load(filename, &_w, &_h, &_components, 4);
    _size = _w * _h * 4;
}

void Image::free() {
    stbi_image_free(_data);
    _data = NULL;
}

unsigned char* Image::copyData() const {
    if (_data == NULL)
        return NULL;
    
    char *copy = new char[_size];
    return (unsigned char*)memcpy((void*)copy, (void*)_data, _size * sizeof(unsigned char));
}

int Image::width() { return _w; }
int Image::height() { return _h; }
int Image::components() { return _components; }
int Image::size() { return _size; }
bool Image::empty() { return (_data == NULL); }

Partitioner::Partitioner() {}
Partitioner::~Partitioner() {}

//occupies an index in IDs (use last index from freeIDs if available),
//and return ID
int Partitioner::push() {
    if (_freeIDs.empty()) {
        _IDs.push_back(true);
        return _IDs.size() - 1;
    }

    int i = _freeIDs.back();
    _freeIDs.pop_back();
    _IDs[i] = true;
    return i;
}

//sets element i to false and pushes its index to freeIDs
void Partitioner::remove(int i) {
    if (_IDs[i]) {
        _IDs[i] = false;
        _freeIDs.push_back(i);
    }
}

//get vector of all indices that are true
std::vector<int> Partitioner::getUsed() {
    std::vector<int> indices;
    for (unsigned i = 0; i < _IDs.size(); i++)
        if (_IDs[i])
            indices.push_back(i);
    
    return indices;
}

void Partitioner::clear() {
    _IDs.clear();
    _freeIDs.clear();
}

//access element i
bool Partitioner::at(int i) { return _IDs[i]; }
bool Partitioner::operator[](int i) { return _IDs[i]; }
//get whether used IDs are empty
bool Partitioner::empty() { return (_IDs.size() == 0); }
//get size of IDs (includes free IDs)
unsigned Partitioner::size() { return _IDs.size(); }
//get size of free IDs
unsigned Partitioner::freeSize() { return _freeIDs.size(); }
//get size of used IDs
unsigned Partitioner::fillSize() { return _IDs.size() - _freeIDs.size(); }

/* Checks if provided string ends with the provided suffix.
*/
bool endsWith(const std::string& str, const std::string& suffix)
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

/* Checks if provided string starts with the provided prefix
*/
bool startsWith(const std::string& str, const std::string& prefix)
{
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}