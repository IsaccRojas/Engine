#include "../include/glutil.hpp"

namespace GLUtil {

    // _______________________________________ GLStage _______________________________________

    GLStage::GLStage(const char *shader_srcs[], GLenum shader_types[], int count) : _program_h(-1) {
        init(shader_srcs, shader_types, count);
    }
    GLStage::GLStage() : _program_h(-1) {}

    GLStage::GLStage(GLStage &&other) {
        if (this != &other) {
            _program_h = other._program_h;
            other._program_h = -1;
        }
    }

    GLStage& GLStage::operator=(GLStage &&other) {
        if (this != &other) {
            _program_h = other._program_h;
            other._program_h = -1;
        }
        return *this;
    }

    GLStage::~GLStage() {
        if (_program_h != -1)
            glDeleteProgram(_program_h);
    };

    void GLStage::init(const char *shader_srcs[], GLenum shader_types[], int count) {
        if (_program_h != -1) {
            std::cerr << "WARN: attempt to initialize already initialized program in GLStage instance " << this << std::endl;
            return;
        }
        
        GLint success = 0;
        // create shaders
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
        }

        // detach and delete shaders
        for (int i = 0; i < count; i++) {
            glDetachShader(_program_h, shaders[i]);
            glDeleteShader(shaders[i]);
        }
        delete[] shaders;
    }

    void GLStage::use() {
        if (_program_h < 0) {
            std::cerr << "WARN: attempt to bind program with unset program handle in GLStage instance " << this << std::endl;
            return;
        }

         glUseProgram(_program_h); 
    }

    // _______________________________________ GLBuffer _______________________________________

    GLBuffer::GLBuffer(GLenum buffer_usage, GLuint buffer_size) : _buf_h(-1), _usage(0), _size(0) {
        init(buffer_usage, buffer_size);
    }
    GLBuffer::GLBuffer() : _buf_h(-1), _usage(0), _size(0) {}

    GLBuffer::GLBuffer(GLBuffer &&other) {
        if (this != &other) {
            _buf_h = other._buf_h;
            _usage = other._usage;
            _size = other._size;
            other._buf_h = -1;
            other._usage = 0;
            other._size = 0;
        }
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
        clear();
    }

    void GLBuffer::init(GLenum buffer_usage, GLuint buffer_size) {
        if (_buf_h != -1) {
            std::cerr << "WARN: attempt to initialize already initialized buffer in GLBuffer instance " << this << std::endl;
            return;
        }

        // create empty data space
        GLuint buf_h = _buf_h;

        glCreateBuffers(1, &buf_h);
        glNamedBufferData(buf_h, buffer_size, NULL, buffer_usage);

        _buf_h = buf_h;
        _usage = buffer_usage;
        _size = buffer_size;
    }

    void GLBuffer::bind(GLenum target) {
        if (_buf_h < 0) {
            std::cerr << "WARN: attempt to bind buffer to target with unset buffer handle in GLBuffer instance " << this << std::endl;
            return;
        }

        GLuint buf_h = _buf_h;
        glBindBuffer(target, buf_h);
    }

    void GLBuffer::bindIndex(GLuint index, GLintptr offset, GLsizei stride) {
        if (_buf_h < 0) {
            std::cerr << "WARN: attempt to bind buffer to index with unset buffer handle in GLBuffer instance " << this << std::endl;
            return;
        }

        GLuint buf_h = _buf_h;
        glBindVertexBuffer(index, buf_h, offset, stride);
    }

    void GLBuffer::bindBase(GLenum target, GLuint index) {
        if (_buf_h < 0) {
            std::cerr << "WARN: attempt to bind buffer to base with unset buffer handle in GLBuffer instance " << this << std::endl;
            return;
        }

        GLuint buf_h = _buf_h;
        glBindBufferBase(target, index, buf_h); 
    }

    void GLBuffer::subData(GLsizeiptr data_size, const void *data, GLsizeiptr offset) {
        if (_buf_h < 0) {
            std::cerr << "WARN: attempt to write to sub data with unset buffer handle in GLBuffer instance " << this << std::endl;
            return;
        }

        GLuint buf_h = _buf_h;
        glNamedBufferSubData(buf_h, offset, data_size, data);
    }

    void GLBuffer::clear() {
        if (_buf_h != -1) {
            GLuint buf_h = _buf_h;
            glDeleteBuffers(1, &buf_h);
        }

        _buf_h = -1;
        _usage = 0;
        _size = 0;
    }

    GLuint GLBuffer::size() { return _size; }
    GLenum GLBuffer::usage() { return _usage; }
    GLuint GLBuffer::handle() { return _buf_h; }

    const char *GLBuffer::copy_mem() {
        if (_buf_h < 0) {
            std::cerr << "WARN: attempt to copy buffer memory with unset buffer handle in GLBuffer instance " << this << std::endl;
            return nullptr;
        }

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
        clear();
    }

    void GLTexture2DArray::init() {
        if (_tex_h != -1) {
            std::cerr << "WARN: attempt to initialize already initialized texture in GLTexture2DArray instance " << this << std::endl;
            return;
        }

        GLuint tex_h = _tex_h;
        glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &tex_h);
        _tex_h = tex_h;

        parameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        parameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        parameteri(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        parameteri(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    void GLTexture2DArray::bind(GLenum target) {
        if (_tex_h < 0) {
            std::cerr << "WARN: attempt to bind texture with unset texture handle in GLTexture2DArray instance " << this << std::endl;
            return;
        }

        GLuint tex_h = _tex_h;
        glBindTexture(target, tex_h);
    }

    void GLTexture2DArray::parameteri(GLenum param, GLint value) {
        if (_tex_h < 0) {
            std::cerr << "WARN: attempt to set parameter with unset texture handle in GLTexture2DArray instance " << this << std::endl;
            return;
        }

        GLuint tex_h = _tex_h;
        glTextureParameteri(tex_h, param, value);
    }

    void GLTexture2DArray::alloc(GLint levels, GLenum store_format, GLenum data_format, GLenum type, GLsizei width, GLsizei height, GLsizei depth) {
        if (_tex_h < 0) {
            std::cerr << "WARN: attempt to allocate memory in texture with unset texture handle in GLTexture2DArray instance " << this << std::endl;
            return;
        }

        if (_allocated) {
            std::cerr << "WARN: attempt to allocate memory in texture that has already allocated memory in GLTexture2DArray instance " << this << std::endl;
            return;
        }
        
        _levels = levels;
        _store_format = store_format;
        _data_format = data_format;
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
            glTexImage3D(GL_TEXTURE_2D_ARRAY, i, _store_format, levelwidth, levelheight, _depth, 0, _data_format, _type, NULL);
        }
        _size = _width * _height * _depth;

        _allocated = true;
    }

    void GLTexture2DArray::subImage(GLint level, GLint x_offset, GLint y_offset, GLint z_offset, GLsizei width, GLsizei height, GLsizei depth, const void *data) {
        if (_tex_h < 0) {
            std::cerr << "WARN: attempt to write sub-image data with unset texture handle in GLTexture2DArray instance " << this << std::endl;
            return;
        }

        GLuint tex_h = _tex_h;
        glTextureSubImage3D(tex_h, level, x_offset, y_offset, z_offset, width, height, depth, _data_format, _type, data);
    }

    void GLTexture2DArray::clear() {
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
    BVec2::BVec2() : _buf(nullptr), _off(0) {}
    BVec2::~BVec2() {}

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
        if (!_buf) {
            std::cerr << "WARN: attempt to write sub data data with null buffer reference in BVec2 instance " << this << std::endl;
            return;
        }

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
        _buf = other._buf;
        _off = other._off;
        _data[0] = other._data[0];
        _data[1] = other._data[1];
        _data[2] = other._data[2];
        v = other.v;
    }
    BVec3::BVec3() : _buf(nullptr), _off(0) {}
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

    void BVec3::setBuffer(GLBuffer *buffer, GLuint offset) {
        _buf = buffer;
        _off = offset;
    }

    void BVec3::update() {
        if (!_buf) {
            std::cerr << "WARN: attempt to write sub data data with null buffer reference in BVec3 instance " << this << std::endl;
            return;
        }

        _data[0] = v.x;
        _data[1] = v.y;
        _data[2] = v.z;
        _buf->subData(sizeof(_data), _data, _off);
    }

    // ______________________________________________________________________________

    void formatAttrib(GLuint index, GLint size, GLenum type, GLuint byte_offset, GLuint divisor) {
        glVertexAttribFormat(index, size, type, false, byte_offset);
        glVertexAttribDivisor(index, divisor);
        glEnableVertexAttribArray(index);
    }

    void bindAttrib(GLuint attrib_index, GLuint binding_index) {
        glVertexAttribBinding(attrib_index, binding_index);
    }

    void uniformi(GLuint index, GLint value) {
        glUniform1i(index, value);
    }
    void uniformf(GLuint index, GLfloat value) {
        glUniform1f(index, value);
    }
    void uniformmat4f(GLuint index, glm::mat4 value) {
        glUniformMatrix4fv(index, 1, false, glm::value_ptr(value));
    }

    void setElementBuffer(GLuint buf_h) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf_h);
    }

    void storage(GLuint buf_h, GLuint index) {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, buf_h);
    }

    void render(GLsizei count, bool with_elements) {
        if (with_elements)
            glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, NULL);
        else
            glDrawArrays(GL_TRIANGLES, 0, count);
    }

    void renderInst(GLsizei count, GLuint numinst) {
        glDrawElementsInstanced(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr, numinst);
    };

    void compute(GLuint groups_x, GLuint groups_y, GLuint groups_z) {
        glDispatchCompute(groups_x, groups_y, groups_z);    
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    void setViewport(GLint x, GLint y, GLint width, GLint height) {
        glViewport(x, y, width, height);
    }

};