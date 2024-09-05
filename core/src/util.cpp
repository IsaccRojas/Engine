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
    
    if (_data == NULL) {
        std::cerr << "WARN: Image::load: failed to load file '" << filename <<"' into Image instance " << this << std::endl;
        return;
    }

    _size = _w * _h * 4;
}

void Image::free() {
    if (!_data) {
        std::cerr << "WARN: Image::free: attempt to free data from empty Image instance " << this << std::endl;
        return;
    }

    stbi_image_free(_data);
    _data = NULL;
}

unsigned char* Image::copyData() const {
    if (_data == NULL) {
        std::cerr << "WARN: Image::copyData: attempt to copy data from empty Image instance " << this << std::endl;
        return NULL;
    }
    
    char *copy = new char[_size];
    return (unsigned char*)memcpy((void*)copy, (void*)_data, _size * sizeof(unsigned char));
}

int Image::width() { return _w; }
int Image::height() { return _h; }
int Image::components() { return _components; }
int Image::size() { return _size; }
bool Image::empty() { return (_data == NULL); }

SlotVec::SlotVec() {}
SlotVec::~SlotVec() { /* automatic destruction is fine */ }

int SlotVec::push() {
    if (_free_ids.empty()) {
        _ids.push_back(true);
        return _ids.size() - 1;
    }

    int i = _free_ids.back();
    _free_ids.pop_back();
    _ids[i] = true;
    return i;
}

void SlotVec::remove(int i) {
    if (_ids[i]) {
        _ids[i] = false;
        _free_ids.push_back(i);
    } else {
        std::cerr << "WARN: SlotVec::remove: attempt to remove inactive index from SlotVec index " << this << std::endl;
    }
}

std::vector<int> SlotVec::getUsed() {
    std::vector<int> indices;
    for (unsigned i = 0; i < _ids.size(); i++)
        if (_ids[i])
            indices.push_back(i);
    
    return indices;
}

void SlotVec::clear() {
    _ids.clear();
    _free_ids.clear();
}

bool SlotVec::at(int i) { return _ids[i]; }
bool SlotVec::operator[](int i) { return _ids[i]; }
bool SlotVec::empty() { return (_ids.size() == 0); }
unsigned SlotVec::size() { return _ids.size(); }
unsigned SlotVec::freeSize() { return _free_ids.size(); }
unsigned SlotVec::fillSize() { return _ids.size() - _free_ids.size(); }

bool endsWith(const std::string& str, const std::string& suffix)
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

bool startsWith(const std::string& str, const std::string& prefix)
{
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}