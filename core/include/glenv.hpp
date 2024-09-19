#ifndef GLENV_HPP_
#define GLENV_HPP_

#include "util.hpp"
#include "glutil.hpp"

/* class Quad
   Encapsulates Quad-like data for OpenGL environments.
   Uses BVec instances to store basic parameters of quads:

   - position - location of quad in 3D space
   - scale - values to scale width, height and depth of unit quad
   - texture position - UV coordinates to use in texture space
   - texture size - width and height of texture to use, applied to UV coordinates to get rectangle
*/
class Quad {
public:
   /* BVecs containing references to Quad's associated OpenGL buffers */
    GLUtil::BVec3 bv_pos;
    GLUtil::BVec3 bv_scale;
    GLUtil::BVec3 bv_texpos;
    GLUtil::BVec2 bv_texsize;
    /* position - location of quad in 3D space
       quadscale - scaling values for x, y, and z coordinates of quad vertices
       textureposition - UV coordinates to use in texture space
       texturesize - width and height of texture to use, applied to UV coordinates to get rectangle 
    */
    Quad(GLUtil::BVec3 position, GLUtil::BVec3 quadscale, GLUtil::BVec3 textureposition, GLUtil::BVec2 texturesize);
    Quad();
    ~Quad();

    // default copy assignment/construction are fine

    /* Calls update() on all internal BVec instances, writing their respective data into their respective buffers. */
    void update();
};

/* class Point
   Encapsulates Point-like data for OpenGL environments.
   Uses BVec instances to store basic parameters of points:

   - position - location of point in 3D space
   - radius - radius of point
   - color - RGB color of point
*/
class Point {
public:
   /* BVecs containing references to Point's associated OpenGL buffers */
    GLUtil::BVec3 bv_pos;
    GLUtil::BFloat bf_radius;
    GLUtil::BVec3 bv_color;
    /* position - location of point in 3D space
       radius - radius of point
       color - color of point
       texturesize - width and height of texture to use, applied to UV coordinates to get rectangle 
    */
    Point(GLUtil::BVec3 position, GLUtil::BFloat radius, GLUtil::BVec3 color);
    Point();
    ~Point();

    // default copy assignment/construction are fine

    /* Calls update() on all internal BVec instances, writing their respective data into their respective buffers. */
    void update();
};

/* class GLEnv
   Encapsulates all OpenGL environment related data and methods. 
   Currently restricted to draw Quads with a simple fragment shader and vertex shader, and parameterized
   view and projection matrices. The following parameters exist per Quad:

   - Quad position (vec3)
   - Quad scale (vec3)
   - Quad texture position (vec3) (multi-level 2D texture space)
   - Quad texture size (vec2) (added to positions to get a rectangle)
   - Quad drawing flag (float) (NOT controllable by user; GLEnv automatically sets/unsets this)

   The maximum amount of Quads allowed by the system can be specified. This also
   guarantees that no more than max_count IDs will be generated and tracked. The environment will
   throw an exception if more than the allowed amount is generated.

   It is undefined behavior to make method calls (except for uninit()) on instances 
   of this class without calling init() first.
*/
class GLEnv {
    /* environment structures */
    // shader program structure
    GLUtil::GLStage _stage;
    // texture array structure
    GLUtil::GLTexture2DArray _texarray;

    /* model data buffers */
    // model to be used per instance
    GLUtil::GLBuffer _glb_modelbuf;
    // position elements of model
    GLUtil::GLBuffer _glb_elembuf;

    /* per instance data buffers */
    // position of instance
    GLUtil::GLBuffer _glb_pos;
    // scale of instance
    GLUtil::GLBuffer _glb_scale;
    // texture position of instance
    GLUtil::GLBuffer _glb_texpos;
    // texture size of instance
    GLUtil::GLBuffer _glb_texsize;
    // whether instance should be drawn or zeroed out
    GLUtil::GLBuffer _glb_draw;

    /* environment system variables */
    // Offsets to distribute to Quads, and Quads
    IntGenerator _quad_offsets;
    std::vector<Quad> _quads;

    // maximum number of active Quads allowed
    unsigned _max_count;
    unsigned _count;

    // flag to prevent moved GLEnv instances from doing anything
    bool _initialized;
public:
    /* Calls init() with the provided arguments. */
    GLEnv(unsigned max_count);
    GLEnv(GLEnv &&other);
    GLEnv();
    GLEnv(const GLEnv &Other) = delete;
    ~GLEnv();

    GLEnv& operator=(GLEnv &&other);
    GLEnv& operator=(const GLEnv&) = delete;

    /* Initializes GLBuffers, GLStage, and GLTexture2DArray, allowing the provided maximum amount of Quads. */
    void init(unsigned max_count);
    void uninit();

    /* Generates an active Quad in system. This call does not write the new Quad into graphic memory. You 
       must call the update() method on the environment or a reference to the Quad itself.
       pos - GLM vec3 position of Quad
       scale - GLM vec3 scale of Quad
       texpos - GLM vec3 texture position of Quad (multi-level 2D texture space)
       texsize - GLM vec2 texture size of Quad (added to positions to get a rectangle)
       Returns the integer offset of Quad. This number can be used to index into the internal Quad container and
       obtain a reference (see the get() method). This offset is unique and will be valid for the lifetime 
       of the Quad (see the erase() method). If the maximum number of active Quads allowed is exceeded, -1 is
       returned instead.
    */
    unsigned genQuad(glm::vec3 pos, glm::vec3 scale, glm::vec3 texpos, glm::vec2 texsize);
    /* Removes the Quad with the provided offset from the system. This will cause the provided offset to be 
       invalid until returned again by the genQuad() method. Note that this method does not actually
       free any GPU memory; it simply makes the specific offset usable again by the system. Attempting to use
       the same offset after erasing it and before receiving it again by genQuad() will result in undefined 
       behavior.
       offset - offset of Quad to remove
    */
    void remove(unsigned offset);

    /* Initializes texture array space with unsigned byte storage in RGBA format.
       width - width of space
       height - height of space
       depth - depth of space
    */
    void setTexArray(GLuint width, GLuint height, GLuint depth);
    /* Loads image into texture space using Image structure (uses complete width and height of passed image).
       img - Image structure containing RGBA unsigned byte image data
       x_offset - x offset in image space to write image data into
       y_offset - y offset in image space to write image data into
       z_offset - z offset in image space to write image data into
    */
    void setTexture(Image img, GLuint x_offset, GLuint y_offset, GLuint z_offset);
    /* Sets view matrix for vertex shader.
       view - GLM mat4 matrix
    */
    void setView(glm::mat4 view);
    /* Sets projection matrix for vertex shader.
       proj - GLM mat4 matrix
    */
    void setProj(glm::mat4 proj);
    /* Writes data of all quads in system to their respective buffers. */
    void update();
    /* Draws Quads in memory using internal shader program. This is done by drawing a number of unit Quad
       instances corresponding to the number of offsets generated, and using the specific Quad parameters and
       shader matrices to transform them. */
    void drawQuads();

    /* Returns a raw Quad pointer to the Quad with the specified offset. */
    Quad *getQuad(unsigned offset);
    /* Returns all active offsets in system. (note that this instantiates a vector and will take O(n) time) */
    std::vector<unsigned> getOffsets();
    /* Returns true if the provided offset is active. */
    bool hasOffset(unsigned offset);
    /* Returns whether this instance has been initialized or not. */
    bool getInitialized();
};

#endif