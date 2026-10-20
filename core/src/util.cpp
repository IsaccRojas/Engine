#include "../include/util.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"

Transform& Transform::apply(Transform& other) {
    pos += other.pos;
    scale *= other.scale;
    return *this;
}

Transform Transform::apply(const Transform& first, const Transform& second) {
    return Transform{first.pos + second.pos, first.scale * second.scale};
}

InactiveIntException::InactiveIntException() : std::logic_error("Inactive IntGenerator integer accessed") {}

std::string readfile(const char* filename) {
    std::ifstream infile(filename);
    std::stringstream buf;
    buf << infile.rdbuf();
    return buf.str();
}

Image::Image(const char* filename) {
    load(filename);
}
Image::Image(const Image& other) :
    _data(other.copyData()),
    _w(other._w),
    _h(other._h),
    _components(other._components),
    _size(other._size)
{}
Image::Image() : _data(NULL), _w(0), _h(0), _size(0) {};
Image& Image::operator=(const Image& other) {
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

void Image::load(const char* filename) {
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
    
    char* copy = new char[_size];
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

std::string hasDuplicate(const std::list<std::string>& list) {
    for (auto iter1 = list.begin(); iter1 != list.end(); iter1++)
        for (auto iter2 = std::next(iter1); iter2 != list.end(); iter2++)
            if (*iter1 == *iter2)
                return *iter1;
    return "";
}

// --------------------------------------------------------------------------------------------------------------------------

bool computeCollisionAABB(Transform transf1, Transform transf2) {
    glm::vec3& pos1 = transf1.pos;
    glm::vec3& dim1 = transf1.scale;
    glm::vec3& pos2 = transf2.pos;
    glm::vec3& dim2 = transf2.scale;

    // get current collision
    float coll_x_space = glm::abs(pos1.x - pos2.x) - ((dim1.x + dim2.x) / 2.0f);
    float coll_y_space = glm::abs(pos1.y - pos2.y) - ((dim1.y + dim2.y) / 2.0f);
    float coll_z_space = glm::abs(pos1.z - pos2.z) - ((dim1.z + dim2.z) / 2.0f);

    if (coll_x_space < 0.0f && coll_y_space < 0.0f && coll_z_space < 0.0f)
        return true;
    return false;
}

float computeDistanceAABB(Transform transf1, Transform transf2, glm::bvec3 axis) {
    glm::vec3& pos1 = transf1.pos;
    glm::vec3& dim1 = transf1.scale;
    glm::vec3& pos2 = transf2.pos;
    glm::vec3& dim2 = transf2.scale;

    // get current collision
    if (axis.x) {
        float coll_x_space = glm::abs(pos1.x - pos2.x) - ((dim1.x + dim2.x) / 2.0f);
        return coll_x_space;
    }

    if (axis.y) {
        float coll_y_space = glm::abs(pos1.y - pos2.y) - ((dim1.y + dim2.y) / 2.0f);
        return coll_y_space;
    }
    
    if (axis.z) {
        float coll_z_space = glm::abs(pos1.z - pos2.z) - ((dim1.z + dim2.z) / 2.0f);
        return coll_z_space;
    }

    return 0.0f;
}

glm::vec3 toVec3(glm::vec2 v, float z) { return glm::vec3(v.x, v.y, z); }

glm::vec2 toVec2(glm::vec3 v) { return glm::vec2(v.x, v.y); }

glm::vec3 randomAngle(glm::vec3 v, float deg_range) {
    if (deg_range == 0.0f)
        return v;
    return glm::rotate(v, glm::radians((-1.0f * deg_range) + float(rand() % int(deg_range * 2.0f))), glm::vec3(0.0f, 0.0f, 1.0f));
}

bool isEven(int x) {
    return (float(x) / 2.0f) == glm::floor(float(x) / 2.0f);
}

float absMin(float a, float b) {
    return (glm::abs(a) >= glm::abs(b)) ? b : a;
}