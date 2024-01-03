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

// representation of an execution stage of OpenGL; stores program and associated VAO
class GLStage {
    GLuint _program_h;
    bool _has_elements;
public:
    GLStage();
    
    GLStage(const GLStage&) = delete;
    GLStage& operator=(const GLStage&) = delete;
    
    GLStage(GLStage &&other);
    GLStage& operator=(GLStage &&other);

    ~GLStage();

    // create program using provided source(s)
    void program(const char *shader_srcs[], GLenum shader_types[], int count);

    // bind this program to the main program binding point
    void use();
    
    // format attribute in program, and enables it
    /*
        index - layout index of attribute
        size - number of values in attribute (e.g. 3 for a vec3)
        type - type of values (e.g. GL_FLOAT)
        offset - offset of index in buffer per element, in bytes (e.g. 12 if values are after a vec3's values)
        divisor - number of instances to process before changing value (0 for per vertex, 1 for per instance, etc.)
    */
    void formatAttrib(GLuint index, GLint size, GLenum type, GLuint byteoffset, GLuint divisor);
    
    // bind attribute to buffer index
    void bindAttrib(GLuint attribindex, GLuint bindingindex);
    
    // set uniform value in program
    void uniformi(GLuint index, GLint value);
    void uniformf(GLuint index, GLfloat value);
    void uniformmat4f(GLuint index, glm::mat4 value);

    // provide element buffer to use for rendering
    void element(GLuint buf_h);

    // provide buffer storage for compute shading
    void storage(GLuint buf_h, GLuint index);

    // render provided number of elements
    void render(GLsizei count);

    // render provided number of elements, as numinst instances
    void renderInst(GLsizei count, GLuint numinst);

    // run compute shader on specified groups
    void compute(GLuint groups_x, GLuint groups_y, GLuint groups_z);
};

//class to preserve state of a single OpenGL buffer with a fixed usage and byte size
class GLBuffer {
    GLuint _buf_h;
    GLenum _usage;
    GLuint _size;

public:
    //create new GLBuffer with fixed usage and byte size
    GLBuffer(GLenum buffer_usage, GLuint buffer_size);
    
    GLBuffer(const GLBuffer&) = delete;
    GLBuffer& operator=(const GLBuffer&) = delete;
    
    GLBuffer(GLBuffer &&other);
    GLBuffer& operator=(GLBuffer &&other);

    GLBuffer();

    ~GLBuffer();

    //binds buffer handle of GLBuffer to target
    void bind(GLenum target);

    //binds buffer handle of GLBuffer to attribute index binding point; size and offset in bytes (e.g. 16 for 4 4-byte vertices)
    void bindIndex(GLuint index, GLintptr offset, GLsizei stride);

    //binds buffer handle of GLBuffer to target at base
    void bindBase(GLenum target, GLuint index);

    //update sub data in GLBuffer; size and offset in bytes (e.g. 16 for 4 4-byte vertices)
    void subData(GLsizeiptr data_size, const void *data, GLsizeiptr offset);

    GLuint size();
    GLenum usage();
    GLuint handle();

    const char *copy_mem();
};

//class to preserve state of a single mutable OpenGL 2D texture array
class GLTexture2DArray {
    GLuint _tex_h;
    GLuint _size;

    GLenum _levels;
    GLenum _storeformat;
    GLenum _dataformat;
    GLenum _type;

    GLuint _width;
    GLuint _height;
    GLuint _depth;

    bool _allocated;

    void _init();
public:
    //create new empty GLTexture
    GLTexture2DArray();

    GLTexture2DArray(const GLTexture2DArray&) = delete;
    GLTexture2DArray& operator=(const GLTexture2DArray&) = delete;

    GLTexture2DArray(GLTexture2DArray &&other);
    GLTexture2DArray& operator=(GLTexture2DArray &&other);

    ~GLTexture2DArray();

    //binds texture handle of GLTexture2D to target
    void bind(GLenum target);

    void parameteri(GLenum param, GLint value);

    //allocate memory storage
    void alloc(GLint levels, GLenum storeformat, GLenum dataformat, GLenum type, GLsizei width, GLsizei height, GLsizei depth);

    //write sub image data into level of allocated storage
    void subImage(GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, const void *data);

    void clear();

    GLuint size();
    GLuint width();
    GLuint height();
    GLuint depth();
};

//int bcount = 0;

//class to wrap a vec2 with a specific offset into a GLBuffer
class BVec2 {
    GLBuffer *_buf;
    GLuint _off;
    GLfloat _data[2];
public:
    glm::vec2 v;

    BVec2(GLBuffer *buffer, GLuint offset);
    BVec2(const BVec2 &other);
    BVec2& operator=(const BVec2 &other);
    BVec2();
    ~BVec2();

    BVec2(BVec2 &&other) = delete;
    BVec2& operator=(BVec2 &&other) = delete;

    void update();
};

//class to wrap a vec3 with a specific offset into a GLBuffer
class BVec3 {
    GLBuffer *_buf;
    GLuint _off;
    GLfloat _data[3];
public:
    glm::vec3 v;

    BVec3(GLBuffer *buffer, GLuint offset);
    BVec3(const BVec3 &other);
    BVec3& operator=(const BVec3 &other);
    BVec3();
    ~BVec3();

    BVec3(BVec3 &&other) = delete;
    BVec3& operator=(BVec3 &&other) = delete;

    void update();
};

#endif