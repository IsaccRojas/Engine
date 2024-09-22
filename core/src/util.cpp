#include "../include/util.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"

InactiveIntException::InactiveIntException() : std::logic_error("Inactive IntGenerator integer accessed") {}

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
    
    if (!_data)
        throw std::bad_alloc();

    _size = _w * _h * 4;
}

void Image::free() {
    if (!_data)
        throw std::runtime_error("Attempt to free Image with null data");

    stbi_image_free(_data);
    _data = NULL;
}

unsigned char* Image::copyData() const {
    if (!_data)
        throw std::runtime_error("Attempt to copy Image with null data");
    
    char *copy = new char[_size];
    return (unsigned char*)memcpy((void*)copy, (void*)_data, _size * sizeof(unsigned char));
}

int Image::width() { return _w; }
int Image::height() { return _h; }
int Image::components() { return _components; }
int Image::size() { return _size; }
bool Image::empty() { return (_data == NULL); }

IntGenerator::IntGenerator() {}
IntGenerator::~IntGenerator() { /* automatic destruction is fine */ }

unsigned IntGenerator::push() {
    if (_free_ids.empty()) {
        _ids.push_back(true);
        return _ids.size() - 1;
    }

    unsigned i = _free_ids.front();
    _free_ids.pop();
    _ids[i] = true;
    return i;
}

void IntGenerator::remove(unsigned i) {
    if (_ids[i]) {
        _ids[i] = false;
        _free_ids.push(i);
    } else
        throw InactiveIntException();
}

std::vector<unsigned> IntGenerator::getUsed() {
    std::vector<unsigned> indices;
    for (unsigned i = 0; i < _ids.size(); i++)
        if (_ids[i])
            indices.push_back(i);
    
    return indices;
}

void IntGenerator::clear() {
    _ids.clear();
    std::queue<unsigned> empty;
    _free_ids.swap(empty);
}

bool IntGenerator::at(unsigned i) { return _ids[i]; }
bool IntGenerator::operator[](unsigned i) { return _ids[i]; }
bool IntGenerator::empty() { return (_ids.size() == 0); }
unsigned IntGenerator::size() { return _ids.size(); }
unsigned IntGenerator::freeSize() { return _free_ids.size(); }
unsigned IntGenerator::activeSize() { return _ids.size() - _free_ids.size(); }

bool endsWith(const std::string& str, const std::string& suffix)
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

bool startsWith(const std::string& str, const std::string& prefix)
{
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}