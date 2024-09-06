#ifndef GLUTIL_HPP_
#define GLUTIL_HPP_

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstring>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <memory>
#include <iostream>

#include "commonexcept.hpp"

namespace GLUtil {

    /* class BadHandleException
       This exception is thrown when an OpenGL handle wrapper attempts to use a bad handle.
    */
    class BadHandleException : public std::runtime_error {
    public:
        BadHandleException();
    };

    /* class BadGLProgramException
       This exception is thrown when initialization of a GLStage instance fails due to a malformed program.
    */
    class BadGLProgramException : public std::runtime_error {
    public:
        BadGLProgramException();
    };

    /* class GLStage
       Represents an execution stage of OpenGL; stores a program handle.
    */
    class GLStage {
        GLint _program_h;
    public:
        GLStage(const char *shader_srcs[], GLenum shader_types[], int count);
        GLStage(GLStage &&other);
        GLStage();
        
        GLStage(const GLStage&) = delete;
        GLStage& operator=(const GLStage&) = delete;
        
        GLStage& operator=(GLStage &&other);

        ~GLStage();

        /* Creates a program with the provided shader sources.
        */
        void init(const char *shader_srcs[], GLenum shader_types[], int count);

        /* Frees handle referenced. */
        void uninit();

        /* Binds this program to the main program binding point.
        */
        void use();
    };

    /* class GLBuffer
       Preserves the state of a single OpenGL buffer with a fixed usage and byte size.
    */
    class GLBuffer {
        GLint _buf_h;
        GLenum _usage;
        GLuint _size;

    public:
        GLBuffer(GLenum buffer_usage, GLuint buffer_size);
        GLBuffer();
        
        GLBuffer(const GLBuffer&) = delete;
        GLBuffer& operator=(const GLBuffer&) = delete;
        
        GLBuffer(GLBuffer &&other);
        GLBuffer& operator=(GLBuffer &&other);

        ~GLBuffer();

        /* Creates buffer with provided size.
        */
        void init(GLenum buffer_usage, GLuint buffer_size);

        /* Binds buffer handle of GLBuffer to target.
        */
        void bind(GLenum target);

        /* Binds buffer handle of GLBuffer to attribute index binding point; size and offset in bytes (e.g. 16 for 4 4-byte vertices).
        */
        void bindIndex(GLuint index, GLintptr offset, GLsizei stride);

        /* Binds buffer handle of GLBuffer to target at base.
        */
        void bindBase(GLenum target, GLuint index);

        /* Updates sub data in GLBuffer; size and offset in bytes (e.g. 16 for 4 4-byte vertices).
        */
        void subData(GLsizeiptr data_size, const void *data, GLsizeiptr offset);

        void uninit();

        GLuint size();
        GLenum usage();
        GLuint handle();

        const char *copy_mem();
    };

    /* class GLTexture2DArray
       Preserves the state of a single mutable OpenGL 2D texture array.
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
        GLTexture2DArray(bool initialize);
        GLTexture2DArray();

        GLTexture2DArray(const GLTexture2DArray&) = delete;
        GLTexture2DArray& operator=(const GLTexture2DArray&) = delete;

        GLTexture2DArray(GLTexture2DArray &&other);
        GLTexture2DArray& operator=(GLTexture2DArray &&other);

        ~GLTexture2DArray();

        /* Initializes GLTexture2DArray.
        */
        void init();

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

        void uninit();

        GLuint size();
        GLuint width();
        GLuint height();
        GLuint depth();
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
        BVec2& operator=(const BVec2 &other);
        ~BVec2();

        BVec2(BVec2 &&other) = delete;
        BVec2& operator=(BVec2 &&other) = delete;

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
        BVec3& operator=(const BVec3 &other);
        ~BVec3();

        BVec3(BVec3 &&other) = delete;
        BVec3& operator=(BVec3 &&other) = delete;

        void setBuffer(GLBuffer *buffer, GLuint offset);

        void update();
    };

    /* Formats attribute in binded program, and enables it.
       index - layout index of attribute
       size - number of values in attribute (e.g. 3 for a vec3)
       type - type of values (e.g. GL_FLOAT)
       offset - offset of index in buffer per element, in bytes (e.g. 12 if values are after a vec3's values)
       divisor - number of instances to process before changing value (0 for per vertex, 1 for per instance, etc.)
    */
    void formatAttrib(GLuint index, GLint size, GLenum type, GLuint byte_offset, GLuint divisor);
    
    /* Binds attribute to buffer index.
    */
    void bindAttrib(GLuint attrib_index, GLuint binding_index);
    
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

    /* Renders provided number of elements.
    */
    void render(GLsizei count, bool with_elements);

    /* Renders provided number of elements, as numinst instances.
    */
    void renderInst(GLsizei count, GLuint num_inst);

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
};

#endif