#include "glutil.hpp"

// _______________________________________ GLStage _______________________________________

//representation of an execution stage of OpenGL; stores program and associated VAO

GLStage::GLStage() : _program_h(0), _has_elements(false) {}

GLStage::GLStage(GLStage &&other) :
    _program_h(other._program_h),
    _has_elements(other._has_elements)
{
    other._program_h = 0;
    other._has_elements = false;
}

GLStage& GLStage::operator=(GLStage &&other) {
    if (this != &other) {
        glDeleteProgram(_program_h);
        _program_h = other._program_h;
        _has_elements = other._has_elements;
        other._program_h = 0;
        other._has_elements = false;
    }
    return *this;
}

GLStage::~GLStage() {
    glDeleteProgram(_program_h);
    _has_elements = false;
};

void GLStage::program(const char *shader_srcs[], GLenum shader_types[], int count) {
    GLint success = 0;
    //create shaders
    GLuint *shaders = new GLuint[count];
    for (int i = 0; i < count; i++) {
        shaders[i] = glCreateShader(shader_types[i]);
        glShaderSource(shaders[i], 1, &shader_srcs[i], NULL);
        glCompileShader(shaders[i]);
        
        success = 0;
        glGetShaderiv(shaders[i], GL_COMPILE_STATUS, &success);
        if (success == GL_FALSE) {
            std::cout << "ERROR: Shader " << i << " failed to compile: ";
            GLint logsize = 0;
            glGetShaderiv(shaders[i], GL_INFO_LOG_LENGTH, &logsize);
            char *logstr = new char[logsize];
            glGetShaderInfoLog(shaders[i], logsize, NULL, logstr);
            std::cout << logstr << std::endl;
            delete logstr;
        }

    }

    //attach to program
    _program_h = glCreateProgram();
    for (int i = 0; i < count; i++)
        glAttachShader(_program_h, shaders[i]);

    //link program
    glLinkProgram(_program_h);

    success = 0;
    glGetProgramiv(_program_h, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        std::cout << "ERROR: Program failed to link: ";
        GLint logsize = 0;
        glGetProgramiv(_program_h, GL_INFO_LOG_LENGTH, &logsize);
        char *logstr = new char[logsize];
        glGetProgramInfoLog(_program_h, logsize, NULL, logstr);
        std::cout << logstr << std::endl;
        delete logstr;
    }

    //detach and delete shaders
    for (int i = 0; i < count; i++) {
        glDetachShader(_program_h, shaders[i]);
        glDeleteShader(shaders[i]);
    }
    delete[] shaders;
}

void GLStage::use() {
    glUseProgram(_program_h);
}

void GLStage::formatattrib(GLuint index, GLint size, GLenum type, GLuint byteoffset, GLuint divisor) {
    glVertexAttribFormat(index, size, type, false, byteoffset);
    glVertexAttribDivisor(index, divisor);
    glEnableVertexAttribArray(index);
}

void GLStage::bindattrib(GLuint attribindex, GLuint bindingindex) {
    glVertexAttribBinding(attribindex, bindingindex);
}

void GLStage::uniformi(GLuint index, GLint value) {
    glUniform1i(index, value);
}
void GLStage::uniformf(GLuint index, GLfloat value) {
    glUniform1f(index, value);
}
void GLStage::uniformmat4f(GLuint index, glm::mat4 value) {
    glUniformMatrix4fv(index, 1, false, glm::value_ptr(value));
}

void GLStage::element(GLuint buf_h) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf_h);
    _has_elements = true;
}

void GLStage::storage(GLuint buf_h, GLuint index) {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, buf_h);
}

void GLStage::render(GLsizei count) {
    if (_has_elements)
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, NULL);
    else
        glDrawArrays(GL_TRIANGLES, 0, count);
}

void GLStage::renderinst(GLsizei count, GLuint numinst) {
    if (_has_elements)
        glDrawElementsInstanced(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr, numinst);
};

void GLStage::compute(GLuint groups_x, GLuint groups_y, GLuint groups_z) {
    glDispatchCompute(groups_x, groups_y, groups_z);    
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

// _______________________________________ GLBuffer _______________________________________

GLBuffer::GLBuffer(GLenum buffer_usage, GLuint buffer_size) :
    _usage(buffer_usage),
    _size(buffer_size)
{
    //create empty data space
    glCreateBuffers(1, &_buf_h);
    glNamedBufferData(_buf_h, _size, NULL, _usage);
}

GLBuffer::GLBuffer(GLBuffer &&other) : 
    _buf_h(other._buf_h),
    _usage(other._usage),
    _size(other._size)
{
    other._buf_h = 0;
}

GLBuffer& GLBuffer::operator=(GLBuffer &&other) {
    if (this != &other) {
        glDeleteBuffers(1, &_buf_h);
        _buf_h = other._buf_h;
        other._buf_h = 0;
    }
    return *this;
}

GLBuffer::GLBuffer() {}

GLBuffer::~GLBuffer() {
    glDeleteBuffers(1, &_buf_h);
}

void GLBuffer::bind(GLenum target) {
    glBindBuffer(target, _buf_h);
}

void GLBuffer::bindindex(GLuint index, GLintptr offset, GLsizei stride) {
    glBindVertexBuffer(index, _buf_h, offset, stride);
}

void GLBuffer::bindbase(GLenum target, GLuint index) {
    glBindBufferBase(target, index, _buf_h); 
}

void GLBuffer::subdata(GLsizeiptr data_size, const void *data, GLsizeiptr offset) {
    glNamedBufferSubData(_buf_h, offset, data_size, data);
}

GLuint GLBuffer::size() { return _size; }
GLenum GLBuffer::usage() { return _usage; }
GLuint GLBuffer::handle() { return _buf_h; }

const char *GLBuffer::copy_mem() {
    void *buf = glMapNamedBuffer(_buf_h, GL_READ_ONLY);
    char *mem = new char[_size];
    std::memcpy(mem, buf, _size);
    bool res = glUnmapNamedBuffer(_buf_h);

    if (res)
        return mem;
    else {
        delete mem;
        return nullptr;
    }
}

// _______________________________________ GLTexture2DArray _______________________________________

void GLTexture2DArray::_init() {
    glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &_tex_h);
    parameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    parameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    parameteri(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    parameteri(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

//create new empty GLTexture2D
GLTexture2DArray::GLTexture2DArray() :
    _size(0),
    _levels(0),
    _storeformat(0),
    _dataformat(0),
    _type(0),
    _width(0),
    _height(0),
    _depth(0),
    _allocated(false)
{
    _init();
}

GLTexture2DArray::GLTexture2DArray(GLTexture2DArray &&other) : 
    _tex_h(other._tex_h),
    _size(other._size),
    _levels(other._levels),
    _storeformat(other._storeformat),
    _dataformat(other._dataformat),
    _type(other._type),
    _width(other._width),
    _height(other._height),
    _depth(other._depth),
    _allocated(other._allocated)
{
    other._tex_h = 0;
    other._size = 0;
    other._levels = 0;
    other._storeformat = 0;
    other._dataformat = 0;
    other._type = 0;
    other._width = 0;
    other._height = 0;
    other._depth = 0;
    other._allocated = false;
}

GLTexture2DArray& GLTexture2DArray::operator=(GLTexture2DArray &&other) {
    if (this != &other) {
        glDeleteTextures(1, &_tex_h);
        _tex_h = other._tex_h;
        _size = other._size;
        _levels = other._levels;
        _storeformat = other._storeformat;
        _dataformat = other._dataformat;
        _type = other._type;
        _width = other._width;
        _height = other._height;
        _depth = other._depth;
        _allocated = other._allocated;
        other._tex_h = 0;
        other._size = 0;
        other._levels = 0;
        other._storeformat = 0;
        other._dataformat = 0;
        other._type = 0;
        other._width = 0;
        other._height = 0;
        other._depth = 0;
        other._allocated = false;
    }
    return *this;
}

GLTexture2DArray::~GLTexture2DArray() {
    glDeleteTextures(1, &_tex_h);
}

void GLTexture2DArray::bind(GLenum target) {
    glBindTexture(target, _tex_h);
}

void GLTexture2DArray::parameteri(GLenum param, GLint value) {
    glTextureParameteri(_tex_h, param, value);
}

void GLTexture2DArray::alloc(GLint levels, GLenum storeformat, GLenum dataformat, GLenum type, GLsizei width, GLsizei height, GLsizei depth) {
    if (_allocated)
        return;
    
    _levels = levels;
    _storeformat = storeformat;
    _dataformat = dataformat;
    _type = type;

    _width = width;
    _height = height;
    _depth = depth;

    bind(GL_TEXTURE_2D_ARRAY);
    GLsizei levelwidth;
    GLsizei levelheight;
    // allocate 3D memory for the texture, with a specified level of detail
    for (int i = 0; i < levels; i++) {
        levelwidth = _width / glm::exp(i);
        levelheight = _height / glm::exp(i);
        glTexImage3D(GL_TEXTURE_2D_ARRAY, i, _storeformat, levelwidth, levelheight, _depth, 0, _dataformat, _type, NULL);
    }
    _size = _width * _height * _depth;

    _allocated = true;
}

void GLTexture2DArray::subimage(GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, const void *data) {
    glTextureSubImage3D(_tex_h, level, xoffset, yoffset, zoffset, width, height, depth, _dataformat, _type, data);
}

void GLTexture2DArray::clear() {
    glDeleteTextures(1, &_tex_h);
    _size = 0;
    _levels = 0;
    _storeformat = 0;
    _dataformat = 0;
    _type = 0;
    _width = 0;
    _height = 0;
    _depth = 0;
    _allocated = false;
    _init();
}

GLuint GLTexture2DArray::size() { return _size; }
GLuint GLTexture2DArray::width() { return _width; }
GLuint GLTexture2DArray::height() { return _height; }
GLuint GLTexture2DArray::depth() { return _depth; }

// _______________________________________ BVec2 _______________________________________

BVec2::BVec2(GLBuffer *buffer, GLuint offset) : 
    _buf(buffer), 
    _off(offset)
{}
BVec2::BVec2(const BVec2 &other) {
    _buf = other._buf;
    _off = other._off;
    _data[0] = other._data[0];
    _data[1] = other._data[1];
    v = other.v;
}
BVec2::BVec2() {}
BVec2::~BVec2() {}

BVec2& BVec2::operator=(const BVec2 &other) {
    _buf = other._buf;
    _off = other._off;
    _data[0] = other._data[0];
    _data[1] = other._data[1];
    v = other.v;

    return *this;
}

void BVec2::update() {
    _data[0] = v.x;
    _data[1] = v.y;
    _buf->subdata(sizeof(_data), _data, _off);
}

// _______________________________________ BVec3 _______________________________________

BVec3::BVec3(GLBuffer *buffer, GLuint offset) : 
    _buf(buffer), 
    _off(offset)
{}
BVec3::BVec3(const BVec3 &other) {
    _buf = other._buf;
    _off = other._off;
    _data[0] = other._data[0];
    _data[1] = other._data[1];
    _data[2] = other._data[2];
    v = other.v;
}
BVec3::BVec3() {}
BVec3::~BVec3() {}

BVec3& BVec3::operator=(const BVec3 &other) {
    _buf = other._buf;
    _off = other._off;
    _data[0] = other._data[0];
    _data[1] = other._data[1];
    _data[2] = other._data[2];
    v = other.v;

    return *this;
}

void BVec3::update() {
    _data[0] = v.x;
    _data[1] = v.y;
    _data[2] = v.z;
    _buf->subdata(sizeof(_data), _data, _off);
}