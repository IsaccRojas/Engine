#include "../include/glutil.hpp"

namespace GLUtil {
    // opengl error handler
    void GLAPIENTRY gl_err_handler(
        GLenum src,
        GLenum type,
        GLuint id,
        GLenum severity,
        GLsizei len,
        const GLchar* msg,
        const void* param
    )  {
        std::cout << "ERR: gl_err_handler(): " << msg << std::endl;
        throw GLUtil::GLErrorException();
    }

    BadGLProgramException::BadGLProgramException() : std::runtime_error("Bad OpenGL program") {}

    GLErrorException::GLErrorException() : std::runtime_error("OpenGL error") {}

    // _______________________________________ GLStage _______________________________________

    GLStage::GLStage(const char *shader_srcs[], GLenum shader_types[], int count) : _program_h(-1), _vao_h(-1) {
        init(shader_srcs, shader_types, count);
    }
    GLStage::GLStage() : _program_h(-1), _vao_h(-1) {}

    GLStage::GLStage(GLStage &&other) {
        operator=(std::move(other));
    }

    GLStage& GLStage::operator=(GLStage &&other) {
        if (this != &other) {
            _program_h = other._program_h;
            other._program_h = -1;
        }
        return *this;
    }

    GLStage::~GLStage() {
        uninit();
    };

    void GLStage::init(const char *shader_srcs[], GLenum shader_types[], int count) {
        if (_program_h != -1 || _vao_h != -1)
            throw InitializedException();
        
        // create shaders
        GLint success = 0;
        GLuint *shaders = new GLuint[count];
        for (int i = 0; i < count; i++) {
            shaders[i] = glCreateShader(shader_types[i]);
            glShaderSource(shaders[i], 1, &shader_srcs[i], NULL);
            glCompileShader(shaders[i]);
            
            success = 0;
            glGetShaderiv(shaders[i], GL_COMPILE_STATUS, &success);
            if (success == GL_FALSE) {
                std::cout << "ERROR: Shader " << i << " failed to compile: ";
                GLint log_size = 0;
                glGetShaderiv(shaders[i], GL_INFO_LOG_LENGTH, &log_size);
                char *log_str = new char[log_size];
                glGetShaderInfoLog(shaders[i], log_size, NULL, log_str);
                std::cout << log_str << std::endl;
                delete[] log_str;
                throw BadGLProgramException();
            }

        }

        // attach to program
        _program_h = glCreateProgram();
        for (int i = 0; i < count; i++)
            glAttachShader(_program_h, shaders[i]);

        // link program
        glLinkProgram(_program_h);

        success = 0;
        glGetProgramiv(_program_h, GL_LINK_STATUS, &success);
        if (success == GL_FALSE) {
            std::cout << "ERROR: Program failed to link: ";
            GLint log_size = 0;
            glGetProgramiv(_program_h, GL_INFO_LOG_LENGTH, &log_size);
            char *log_str = new char[log_size];
            glGetProgramInfoLog(_program_h, log_size, NULL, log_str);
            std::cout << log_str << std::endl;
            delete[] log_str;
            throw BadGLProgramException();
        }

        // detach and delete shaders
        for (int i = 0; i < count; i++) {
            glDetachShader(_program_h, shaders[i]);
            glDeleteShader(shaders[i]);
        }
        delete[] shaders;

        // create VAO
        unsigned vao_h;
        glCreateVertexArrays(1, &vao_h);
        _vao_h = vao_h;
    }

    void GLStage::uninit() {
        if (_program_h != -1)
            glDeleteProgram(_program_h);

        if (_vao_h != -1) {
            unsigned vao_h = _vao_h;
            glDeleteVertexArrays(1, &vao_h);
        }

        _program_h = -1;
        _vao_h = -1;
    }

    void GLStage::setAttribFormat(GLuint index, GLint size, GLenum type, GLuint byte_offset, GLuint divisor) {
        glVertexArrayAttribFormat(_vao_h, index, size, type, false, byte_offset);
        glEnableVertexArrayAttrib(_vao_h, index);
        glVertexArrayBindingDivisor(_vao_h, index, divisor);
    }

    void GLStage::setAttribBufferIndex(GLuint attrib_index, GLuint binding_index) {
        glVertexArrayAttribBinding(_vao_h, attrib_index, binding_index);
    }

    void GLStage::bindBufferToIndex(GLuint buffer_handle, GLuint index, GLintptr offset, GLsizei stride) {
        glVertexArrayVertexBuffer(_vao_h, index, buffer_handle, offset, stride);
    }

    void GLStage::bindElementBuffer(GLuint buffer_handle) {
        glVertexArrayElementBuffer(_vao_h, buffer_handle);
    }

    void GLStage::uniform1i(GLuint index, GLint value) {
        glProgramUniform1i(_program_h, index, value);
    }

    void GLStage::uniform1f(GLuint index, GLfloat value) {
        glProgramUniform1f(_program_h, index, value);
    }

    void GLStage::uniformmat4f(GLuint index, glm::mat4 value) {
        glProgramUniformMatrix4fv(_program_h, index, 1, false, glm::value_ptr(value));
    }

    void GLStage::uniform3ui(GLuint index, glm::uvec3 value) {
        glProgramUniform3ui(_program_h, index, value.x, value.y, value.z);
    }

    void GLStage::use() {
        glBindVertexArray(_vao_h);
        glUseProgram(_program_h); 
    }

    // _______________________________________ GLBuffer _______________________________________

    GLBuffer::GLBuffer(GLenum buffer_usage, GLuint buffer_size) : _buf_h(-1), _usage(0), _size(0) {
        init(buffer_usage, buffer_size);
    }
    GLBuffer::GLBuffer() : _buf_h(-1), _usage(0), _size(0) {}

    GLBuffer::GLBuffer(GLBuffer &&other) {
        operator=(std::move(other));
    }

    GLBuffer& GLBuffer::operator=(GLBuffer &&other) {
        if (this != &other) {
            _buf_h = other._buf_h;
            _usage = other._usage;
            _size = other._size;
            other._buf_h = -1;
            other._usage = 0;
            other._size = 0;
        }
        return *this;
    }

    GLBuffer::~GLBuffer() {
        uninit();
    }

    void GLBuffer::init(GLenum buffer_usage, GLuint buffer_size) {
        if (_buf_h != -1)
            throw InitializedException();
        
        // create empty data space
        GLuint buf_h = _buf_h;

        glCreateBuffers(1, &buf_h);
        glNamedBufferData(buf_h, buffer_size, NULL, buffer_usage);

        _buf_h = buf_h;
        _usage = buffer_usage;
        _size = buffer_size;
    }

    void GLBuffer::uninit() {
        if (_buf_h != -1) {
            GLuint buf_h = _buf_h;
            glDeleteBuffers(1, &buf_h);
        }

        _buf_h = -1;
        _usage = 0;
        _size = 0;
    }

    void GLBuffer::bind(GLenum target) {
        GLuint buf_h = _buf_h;
        glBindBuffer(target, buf_h);
    }

    void GLBuffer::bindBase(GLenum target, GLuint index) {
        GLuint buf_h = _buf_h;
        glBindBufferBase(target, index, buf_h); 
    }

    void GLBuffer::subData(GLsizeiptr data_size, const void *data, GLsizeiptr offset) {
        GLuint buf_h = _buf_h;
        glNamedBufferSubData(buf_h, offset, data_size, data);
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
            delete[] mem;
            return nullptr;
        }
    }

    // _______________________________________ GLTexture2DArray _______________________________________

    GLTexture2DArray::GLTexture2DArray(bool initialize) :
        _size(0),
        _levels(0),
        _store_format(0),
        _data_format(0),
        _type(0),
        _width(0),
        _height(0),
        _depth(0),
        _allocated(false)
    {
        if (initialize)
            init();
    }

    GLTexture2DArray::GLTexture2DArray() : 
        _tex_h(-1),
        _size(0),
        _levels(0),
        _store_format(0),
        _data_format(0),
        _type(0),
        _width(0),
        _height(0),
        _depth(0),
        _allocated(false)
    {}
    
    GLTexture2DArray::GLTexture2DArray(GLTexture2DArray &&other) {
        operator=(std::move(other));
    }

    GLTexture2DArray& GLTexture2DArray::operator=(GLTexture2DArray &&other) {
        if (this != &other) {
            _tex_h = other._tex_h;
            _size = other._size;
            _levels = other._levels;
            _store_format = other._store_format;
            _data_format = other._data_format;
            _type = other._type;
            _width = other._width;
            _height = other._height;
            _depth = other._depth;
            _allocated = other._allocated;
            other._tex_h = -1;
            other._size = 0;
            other._levels = 0;
            other._store_format = 0;
            other._data_format = 0;
            other._type = 0;
            other._width = 0;
            other._height = 0;
            other._depth = 0;
            other._allocated = false;
        }
        return *this;
    }

    GLTexture2DArray::~GLTexture2DArray() {
        uninit();
    }

    void GLTexture2DArray::init() {
        if (_tex_h != -1)
            throw InitializedException();

        GLuint tex_h = _tex_h;
        glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &tex_h);
        _tex_h = tex_h;

        parameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        parameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        parameteri(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        parameteri(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    void GLTexture2DArray::uninit() {
        if (_tex_h != -1) {
            GLuint tex_h = _tex_h;
            glDeleteTextures(1, &tex_h);
        }

        _tex_h = -1;
        _size = 0;
        _levels = 0;
        _store_format = 0;
        _data_format = 0;
        _type = 0;
        _width = 0;
        _height = 0;
        _depth = 0;
        _allocated = false;
    }

    void GLTexture2DArray::bind(GLenum target) {
        GLuint tex_h = _tex_h;
        glBindTexture(target, tex_h);
    }

    void GLTexture2DArray::parameteri(GLenum param, GLint value) {
        GLuint tex_h = _tex_h;
        glTextureParameteri(tex_h, param, value);
    }

    void GLTexture2DArray::alloc(GLint levels, GLenum store_format, GLenum data_format, GLenum type, GLsizei width, GLsizei height, GLsizei depth) {
        if (_allocated)
            throw std::runtime_error("Attempt to allocate already allocated texture data");
        
        _levels = levels;
        _store_format = store_format;
        _data_format = data_format;
        _type = type;
        _width = width;
        _height = height;
        _depth = depth;
        glTextureStorage3D(_tex_h, levels, store_format, width, height, depth);

        _allocated = true;
    }

    void GLTexture2DArray::subImage(GLint level, GLint x_offset, GLint y_offset, GLint z_offset, GLsizei width, GLsizei height, GLsizei depth, const void *data) {
        GLuint tex_h = _tex_h;
        glTextureSubImage3D(tex_h, level, x_offset, y_offset, z_offset, width, height, depth, _data_format, _type, data);
    }

    GLuint GLTexture2DArray::size() { return _size; }
    GLuint GLTexture2DArray::width() { return _width; }
    GLuint GLTexture2DArray::height() { return _height; }
    GLuint GLTexture2DArray::depth() { return _depth; }

    // _______________________________________ BFloat _______________________________________

    BFloat::BFloat(GLBuffer *buffer, GLuint offset) : 
        _buf(buffer), 
        _off(offset),
        v(0.0f)
    {}
    BFloat::BFloat(const BFloat &other) {
        operator=(other);
    }
    BFloat::BFloat() : _buf(nullptr), _off(0), v(0.0f) {}
    BFloat::~BFloat() { /* automatic destruction is fine */ }

    BFloat& BFloat::operator=(const BFloat &other) {
        _buf = other._buf;
        _off = other._off;
        v = other.v;

        return *this;
    }

    void BFloat::setBuffer(GLBuffer *buffer, GLuint offset) {
        _buf = buffer;
        _off = offset;
    }

    void BFloat::update() {
        if (!_buf)
            throw std::runtime_error("Attempt to write subdata with null GLBuffer reference");

        _buf->subData(sizeof(v), &v, _off);
    }

    // _______________________________________ BVec2 _______________________________________

    BVec2::BVec2(GLBuffer *buffer, GLuint offset) : 
        _buf(buffer), 
        _off(offset)
    {}
    BVec2::BVec2(const BVec2 &other) {
        operator=(other);
    }
    BVec2::BVec2() : _buf(nullptr), _off(0) {}
    BVec2::~BVec2() { /* automatic destruction is fine */ }

    BVec2& BVec2::operator=(const BVec2 &other) {
        _buf = other._buf;
        _off = other._off;
        _data[0] = other._data[0];
        _data[1] = other._data[1];
        v = other.v;

        return *this;
    }

    void BVec2::setBuffer(GLBuffer *buffer, GLuint offset) {
        _buf = buffer;
        _off = offset;
    }

    void BVec2::update() {
        if (!_buf)
            throw std::runtime_error("Attempt to write subdata with null GLBuffer reference");

        _data[0] = v.x;
        _data[1] = v.y;

        _buf->subData(sizeof(_data), _data, _off);
    }

    // _______________________________________ BVec3 _______________________________________

    BVec3::BVec3(GLBuffer *buffer, GLuint offset) : 
        _buf(buffer), 
        _off(offset)
    {}
    BVec3::BVec3(const BVec3 &other) {
        operator=(other);
    }
    BVec3::BVec3() : _buf(nullptr), _off(0) {}
    BVec3::~BVec3() { /* automatic destruction is fine */ }

    BVec3& BVec3::operator=(const BVec3 &other) {
        _buf = other._buf;
        _off = other._off;
        _data[0] = other._data[0];
        _data[1] = other._data[1];
        _data[2] = other._data[2];
        v = other.v;

        return *this;
    }

    void BVec3::setBuffer(GLBuffer *buffer, GLuint offset) {
        _buf = buffer;
        _off = offset;
    }

    void BVec3::update() {
        if (!_buf)
            throw std::runtime_error("Attempt to write subdata with null GLBuffer reference");

        _data[0] = v.x;
        _data[1] = v.y;
        _data[2] = v.z;
        _buf->subData(sizeof(_data), _data, _off);
    }

    // ______________________________________________________________________________

    void render(GLenum mode, GLsizei count, bool with_elements) {
        if (with_elements)
            glDrawElements(mode, count, GL_UNSIGNED_INT, nullptr);
        else
            glDrawArrays(mode, 0, count);
    }

    void renderInst(GLenum mode, GLsizei count, GLuint numinst, bool with_elements) {
        if (with_elements)
            glDrawElementsInstanced(mode, count, GL_UNSIGNED_INT, nullptr, numinst);
        else
            glDrawArraysInstanced(mode, 0, count, numinst);
    }

    void setViewport(GLint x, GLint y, GLint width, GLint height) {
        glViewport(x, y, width, height);
    }

    void glinit(bool debug) {
        // initialize glew
        glewInit();
        if (debug)
            glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(gl_err_handler, NULL);
    }
};