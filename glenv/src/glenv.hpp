#pragma once

#ifndef GLENV_HPP_
#define GLENV_HPP_

#include "util.hpp"
#include "glutil.hpp"

/* Class to encapsulate Quad-like data for OpenGL environments.
   Uses BVec instances to store basic parameters of quads:

   - position - location of quad in 3D space
   - scale - values to scale width, height and depth of unit quad
   - texture position - UV coordinates to use in texture space
   - texture size - width and height of texture to use, applied to UV coordinates to get rectangle
*/
class Quad {
public:
   /* BVecs containing references to Quad's associated OpenGL buffers */
    BVec3 pos;
    BVec3 scale;
    BVec3 texpos;
    BVec2 texsize;
    /* position - location of quad in 3D space
       quadscale - scaling values for x, y, and z coordinates of quad vertices
       textureposition - UV coordinates to use in texture space
       texturesize - width and height of texture to use, applied to UV coordinates to get rectangle 
    */
    Quad(BVec3 position, BVec3 quadscale, BVec3 textureposition, BVec2 texturesize);
    Quad(const Quad& other);
    Quad();
    Quad& operator=(const Quad& other);

    /* Calls update() on all internal BVec instances, writing their respective data into their respective buffers. */
    void update();
};

/* Class to encapsulate all OpenGL environment related data and methods. 
   Currently restricted to draw quads with a simple fragment shader and vertex shader, and parameterized
   view and projection matrices. The following parameters exist per quad:

   - Quad position (vec3)
   - Quad scale (vec3)
   - Quad texture position (vec3) (multi-level 2D texture space)
   - Quad texture size (vec2) (added to positions to get a rectangle)
   - Quad drawing flag (float) (NOT controllable by user; GLEnv automatically sets/unsets this)

   The maximum amount of quads allowed by the system can be specified. This also
   guarantees that no more than max_count IDs will be generated and tracked. The environment will
   throw an exception if more than the allowed amount is generated.
*/
class GLEnv {
    /* environment structures */
    // shader program structure
    GLStage _stage;
    // texture array structure
    GLTexture2DArray _texarray;

    /* model data buffers */
    // model to be used per instance
    GLBuffer _glb_modelbuf;
    // position elements of model
    GLBuffer _glb_elembuf;

    /* per instance data buffers */
    // position of instance
    GLBuffer _glb_pos;
    // scale of instance
    GLBuffer _glb_scale;
    // texture position of instance
    GLBuffer _glb_texpos;
    // texture size of instance
    GLBuffer _glb_texsize;
    // whether instance should be drawn or zeroed out
    GLBuffer _glb_draw;

    /* environment system variables */
    // IDs to distribute to quads
    Partitioner _ids;
    // quad pointers
    std::vector<Quad> _quads;
    // maximum number of active quads allowed
    int _maxcount;

public:
    /* max_count - maximum number of quads allowed to be active */
    GLEnv(int maxcount);

    /* Initializes texture array space with unsigned byte storage in RGBA format.
       width - width of space
       height - height of space
       depth - depth of space
    */
    void settexarray(GLuint width, GLuint height, GLuint depth);

    /* Loads image into texture space using Image structure (uses complete width and height of passed image).
       img - Image structure containing RGBA unsigned byte image data
       xoffset - x offset in image space to write image data into
       yoffset - y offset in image space to write image data into
       zoffset - z offset in image space to write image data into
    */
    void settexture(Image img, GLuint xoffset, GLuint yoffset, GLuint zoffset);

    /* Sets OpenGL viewport.
       x - x coordinate of viewport
       y - y coordinate of viewport
       width - width of viewport
       height - height of viewport
    */
    void setviewport(GLint x, GLint y, GLint width, GLint height);

    /* Sets view matrix for vertex shader.
       view - GLM mat4 matrix
    */
    void setview(glm::mat4 view);

    /* Sets projection matrix for vertex shader.
       proj - GLM mat4 matrix
    */
    void setproj(glm::mat4 proj);

    /* Generates an active quad in system. This call does not write the new quad into graphic memory. You 
       must call the update() method on the environment or a reference to the quad itself.
       pos - GLM vec3 position of quad
       scale - GLM vec3 scale of quad
       texpos - GLM vec3 texture position of quad (multi-level 2D texture space)
       texsize - GLM vec2 texture size of quad (added to positions to get a rectangle)
       Returns the integer ID of quad. This number can be used to index into the internal quad container and
       obtain a reference (see the get() method). This ID is unique and will be valid for the lifetime 
       of the quad (see the erase() method). If the maximum number of active quads allowed is exceeded, -1 is
       returned instead.
    */
    int genQuad(glm::vec3 pos, glm::vec3 scale, glm::vec3 texpos, glm::vec2 texsize);

    /* Returns a raw Quad pointer to the quad with the specified ID. 
       i - ID of quad to get reference of
    */
    Quad *get(int i);

    /* Writes data of all quads in system to their respective buffers. */
    void update();

    /* Erases the quad with the provided ID from the system. This will cause the provided ID to be 
       invalid until returned again by the genQuad() method. Note that this method does not actually
       free any GPU memory; it simply makes the specific ID usable again by the system. Attempting to use
       the same ID after erasing it and before receiving it again by genQuad() will result in undefined 
       behavior.
       id - ID of quad to erase
       Returns 0 on success, -1 on failure.
    */
    int erase(int id);

    /* Draws quads in memory using internal shader program. This is done by drawing a number of unit quad
       instances corresponding to the number of IDs generated, and using the specific quad parameters and
       shader matrices to transform them. */
    void draw();

    /* Returns all active IDs in system. (note that this allocates a vector and will take O(n) time) */
    std::vector<int> getids();
};

#endif