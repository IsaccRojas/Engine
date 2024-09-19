#ifndef GLUTIL_HPP_
#define GLUTIL_HPP_

#include <GL/glew.h>
#include <cstring>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <memory>
#include <iostream>

#include "commonexcept.hpp"

namespace GLUtil {
    /* class BadGLProgramException
       This exception is thrown when initialization of a GLStage instance fails due to a malformed program.
    */
    class BadGLProgramException : public std::runtime_error {
    public:
        BadGLProgramException();
    };

    /* class GLErrorException
       This exception is thrown when an error is caught by the OpenGL debug error handler.
    */
    class GLErrorException : public std::runtime_error {
    public:
        GLErrorException();
    };

    /* class GLStage
       Represents an execution stage of OpenGL; stores a program handle and VAO.

       It is undefined behavior to make method calls (except for uninit()) on instances 
       of this class without calling init() first.
    */
    class GLStage {
        GLint _program_h;
        GLint _vao_h;

    public:
        /* Calls init() with the provided arguments. */
        GLStage(const char *shader_srcs[], GLenum shader_types[], int count);
        GLStage(GLStage &&other);
        GLStage();
        GLStage(const GLStage&) = delete;
        ~GLStage();

        GLStage& operator=(GLStage &&other);
        GLStage& operator=(const GLStage&) = delete;

        /* Creates a program with the provided shader sources.
        */
        void init(const char *shader_srcs[], GLenum shader_types[], int count);
        void uninit();

        /* Formats attribute in binded program, and enables it.
           index - layout index of attribute
           size - number of values in attribute (e.g. 3 for a vec3)
           type - type of values (e.g. GL_FLOAT)
           offset - offset of index in buffer per element, in bytes (e.g. 12 if values are after a vec3's values)
           divisor - number of instances to process before changing value (0 for per vertex, 1 for per instance, etc.)
        */
        void setAttribFormat(GLuint index, GLint size, GLenum type, GLuint byte_offset, GLuint divisor);
        
        /* Binds attribute to the specified buffer index.
        */
        void setAttribBufferIndex(GLuint attrib_index, GLuint binding_index);

        /* Binds buffer handler to the specified attribute buffer index; size and offset in bytes (e.g. 16 for 4 4-byte vertices).
        */
        void bindBufferToIndex(GLuint buffer_handle, GLuint index, GLintptr offset, GLsizei stride);

        /* Binds this VAO and program to the main VAO and program binding points, enabling it for use on draw calls.
        */
        void use();
    };

    /* class GLBuffer
       Preserves the state of a single OpenGL buffer with a fixed usage and byte size.
    
       It is undefined behavior to make method calls (except for uninit()) on instances 
       of this class without calling init() first.
    */
    class GLBuffer {
        GLint _buf_h;
        GLenum _usage;
        GLuint _size;

    public:
        /* Calls init() with the provided arguments. */
        GLBuffer(GLenum buffer_usage, GLuint buffer_size);
        GLBuffer(GLBuffer &&other);
        GLBuffer();
        GLBuffer(const GLBuffer&) = delete;
        ~GLBuffer();

        GLBuffer& operator=(GLBuffer &&other);
        GLBuffer& operator=(const GLBuffer&) = delete;

        /* Creates buffer with provided size.
        */
        void init(GLenum buffer_usage, GLuint buffer_size);
        void uninit();

        /* Binds buffer handle of GLBuffer to target.
        */
        void bind(GLenum target);

        /* Binds buffer handle of GLBuffer to target at base.
        */
        void bindBase(GLenum target, GLuint index);

        /* Updates sub data in GLBuffer; size and offset in bytes (e.g. 16 for 4 4-byte vertices).
        */
        void subData(GLsizeiptr data_size, const void *data, GLsizeiptr offset);

        GLuint size();
        GLenum usage();
        GLuint handle();

        const char *copy_mem();
    };

    /* class GLTexture2DArray
       Preserves the state of a single mutable OpenGL 2D texture array.

       It is undefined behavior to make method calls (except for uninit()) on instances 
       of this class without calling init() first.
    */
    class GLTexture2DArray {
        GLint _tex_h;
        GLuint _size;

        GLenum _levels;
        GLenum _store_format;
        GLenum _data_format;
        GLenum _type;

        GLuint _width;
        GLuint _height;
        GLuint _depth;

        bool _allocated;
    public:
        /* Calls init() with the provided arguments. */
        GLTexture2DArray(bool initialize);
        GLTexture2DArray(GLTexture2DArray &&other);
        GLTexture2DArray();
        GLTexture2DArray(const GLTexture2DArray&) = delete;
        ~GLTexture2DArray();
        
        GLTexture2DArray& operator=(GLTexture2DArray &&other);
        GLTexture2DArray& operator=(const GLTexture2DArray&) = delete;

        /* Initializes GLTexture2DArray.
        */
        void init();
        void uninit();

        /* Binds texture handle of GLTexture2D to target.
        */
        void bind(GLenum target);

        void parameteri(GLenum param, GLint value);

        /* Allocate memory storage; note that this method binds this texture to GL_TEXTURE_2D_ARRAY.
        */
        void alloc(GLint levels, GLenum store_format, GLenum data_format, GLenum type, GLsizei width, GLsizei height, GLsizei depth);

        /* Write sub image data into level of allocated storage.
        */
        void subImage(GLint level, GLint x_offset, GLint y_offset, GLint z_offset, GLsizei width, GLsizei height, GLsizei depth, const void *data);

        GLuint size();
        GLuint width();
        GLuint height();
        GLuint depth();
    };

    /* class BFloat
       Wraps a float with a specific offset into a GLBuffer.
    */
    class BFloat {
        GLBuffer *_buf;
        GLuint _off;
    public:
        GLfloat v;

        BFloat(GLBuffer *buffer, GLuint offset);
        BFloat(const BFloat &other);
        BFloat();
        ~BFloat();

        BFloat& operator=(const BFloat &other);

        void setBuffer(GLBuffer *buffer, GLuint offset);

        void update();
    };

    /* class BVec2
       Wraps a glm::vec2 with a specific offset into a GLBuffer.
    */
    class BVec2 {
        GLBuffer *_buf;
        GLuint _off;
        GLfloat _data[2];
    public:
        glm::vec2 v;

        BVec2(GLBuffer *buffer, GLuint offset);
        BVec2(const BVec2 &other);
        BVec2();
        ~BVec2();

        BVec2& operator=(const BVec2 &other);

        void setBuffer(GLBuffer *buffer, GLuint offset);

        void update();
    };

    /* class BVec3
       Wraps a glm::vec3 with a specific offset into a GLBuffer.
    */
    class BVec3 {
        GLBuffer *_buf;
        GLuint _off;
        GLfloat _data[3];
    public:
        glm::vec3 v;

        BVec3(GLBuffer *buffer, GLuint offset);
        BVec3(const BVec3 &other);
        BVec3();
        ~BVec3();

        BVec3& operator=(const BVec3 &other);

        void setBuffer(GLBuffer *buffer, GLuint offset);

        void update();
    };
    
    /* Sets uniform value in program.
    */
    void uniformi(GLuint index, GLint value);
    void uniformf(GLuint index, GLfloat value);
    void uniformmat4f(GLuint index, glm::mat4 value);

    /* Sets element buffer to use for rendering.
    */
    void setElementBuffer(GLuint buf_h);

    /* Binds buffer storage to index for compute shading.
    */
    void storage(GLuint buf_h, GLuint index);

    /* Renders provided number of elements in the specified mode.
    */
    void render(GLenum mode, GLsizei count, bool with_elements);

    /* Renders provided number of elements in the specified mode, as numinst instances.
    */
    void renderInst(GLenum mode, GLsizei count, GLuint num_inst);

    /* Runs compute shader on specified groups.
    */
    void compute(GLuint groups_x, GLuint groups_y, GLuint groups_z);

    /* Sets OpenGL viewport.
       x - x coordinate of viewport
       y - y coordinate of viewport
       width - width of viewport
       height - height of viewport
    */
    void setViewport(GLint x, GLint y, GLint width, GLint height);

    /* Initializes OpenGL. */
    void glinit(bool debug);
};

#endif