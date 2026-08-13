#ifndef GLENV_HPP_
#define GLENV_HPP_

#include "util.hpp"
#include "glutil.hpp"
#include "animation.hpp"
#include <limits>

class GLEnv;

/* class Quad
   Encapsulates Quad-like data for OpenGL environments.
   Uses BVec instances to store basic parameters of quads.

   - position - location of quad in 3D space
   - scale - values to scale width, height and depth of unit quad
   - color - hue to apply to quad
   - texture position - UV coordinates to use in texture space
   - texture size - width and height of texture to use, applied to UV coordinates to get rectangle
*/
class Quad {
   friend GLEnv;

   // BVecs containing references to Quad's associated OpenGL buffers
   GLUtil::BVec3 _bv_pos;
   GLUtil::BVec3 _bv_scale;
   GLUtil::BVec4 _bv_color;
   GLUtil::BVec3 _bv_texpos;
   GLUtil::BVec2 _bv_texsize;

   // variables for updating values
   glm::vec3 _base_pos;
   glm::vec3 _base_scale;
   AnimationState _animationstate;

   Quad();
   
public:
   ~Quad();

   // default copy assignment/construction are fine

   /* Calls update() on all internal BVec instances, writing their respective data into their respective buffers. */
   void updateBVecs();

   /* Resets BVec values to base values. */
   void resetTransformation();

   /* Applies provided Transform to pos and scale BVecs. */
   void applyTransform(Transform transform);

   /* Returns reference to contained Animation state. */
   AnimationState& animationstate();

   /* Writes animation data to related BVecs, if an animation is stored. */
   void writeAnimation();
};

/* struct QuadInfo 
   Collection of information of a Quad that can be used when adding named Quad info to GLEnv instances.
   pos - position of Quad
   scale = scale of Quad
   color = color of Quad
   animation = Animation for Quad to use

*/
struct QuadInfo {
   glm::vec3 pos;
   glm::vec3 scale;
   glm::vec4 color;
   Animation animation;
};

/* class GLEnv
   Encapsulates all OpenGL environment related data and methods. 
   Currently restricted to draw Quads with a simple fragment shader and vertex shader, and parameterized
   view and projection matrices. The following parameters exist per Quad:

   - Quad position (vec3)
   - Quad scale (vec3)
   - Quad color (vec4)
   - Quad texture position (vec3) (multi-level 2D texture space)
   - Quad texture size (vec2) (added to positions to get a rectangle)
   - Quad drawing flag (float) (modified by GLEnv instance)

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
   // color of instance
   GLUtil::GLBuffer _glb_color;
   // texture position of instance
   GLUtil::GLBuffer _glb_texpos;
   // texture size of instance
   GLUtil::GLBuffer _glb_texsize;
   // whether instance should be drawn or zeroed out
   GLUtil::GLBuffer _glb_draw;

   /* environment system variables */
   // offsets to distribute to Quads, and Quads
   IntGenerator _quad_offsets;
   std::vector<Quad> _quads;

   // maximum number of active Quads allowed
   unsigned _max_count;
   unsigned _count;

   // storage of entity info, mapped to names
   std::unordered_map<std::string, QuadInfo> _quadinfos;

   // flag to store if instance was initialized or not
   bool _initialized;

public:
   GLEnv();
   GLEnv(GLEnv&& other);
   GLEnv(const GLEnv& Other) = delete;
   ~GLEnv();

   GLEnv& operator=(GLEnv&& other);
   GLEnv& operator=(const GLEnv&) = delete;

   /* Initializes GLBuffers, GLStage, and GLTexture2DArray, allowing the provided maximum amount of Quads and a map of animations. */
   void init(unsigned max_count);
   void uninit();
   
   /* Stores QuadInfo mapped to provided name for use when generating Quads. */
   void addQuad(QuadInfo quadinfo, const char* name);
   /* Generates an active Quad in system using the specified QuadInfo and Transform (applied on top of QuadInfo transform), or values. 
      This call does not write the new Quad into graphic memory. You must call the updateBVecs() method on the environment or a 
      reference to the Quad itself.
   */
   unsigned genQuad(glm::vec3 pos, glm::vec3 scale, glm::vec4 color, glm::vec3 texpos, glm::vec2 texsize);
   unsigned genQuad(const char* quad_name, Transform transform);
   /* Removes the Quad with the provided offset from the system. This will cause the provided offset to be 
      invalid until returned again by the genQuad() method. Note that this method does not actually
      free any GPU memory; it simply makes the specific offset usable again by the system. Attempting to use
      the same offset after erasing it and before receiving it again by genQuad() will result in undefined 
      behavior.
      offset - offset of Quad to remove
   */
   void remove(unsigned offset);

   void addAnimation(Animation animation, const char* name);

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
   /* Sets the view matrix to a top-down view, at the specified coordinates with the Y axis as up and looking at the negative Z axis. */
   void setViewTopDown(float x, float y, float z);
   /* Sets projection matrix to orthographic volume centered at the origin. */
   void setProjOrthographic(float width, float height, float depth);

   /* Writes data of all quads in system to their respective buffers. */
   void update();
   /* Draws Quads in memory using internal shader program. This is done by drawing a number of unit Quad
      instances corresponding to the number of offsets generated, and using the specific Quad parameters and
      shader matrices to transform them. */
   void drawQuads();

   /* Returns a raw Quad pointer to the Quad with the specified offset. */
   Quad* getQuad(unsigned offset);
   /* Returns all active offsets in system. (note that this instantiates a vector and will take O(n) time) */
   std::vector<unsigned> getOffsets();
   /* Returns true if the provided offset is active. */
   bool hasOffset(unsigned offset);
   /* Returns whether this instance has been initialized or not. */
   bool getInitialized();
};

#endif