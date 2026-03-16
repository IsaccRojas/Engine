#include "../include/glenv.hpp"

// _______________________________________ Quad _______________________________________

Quad::Quad() {}
Quad::~Quad() { /* automatic destruction is fine */ }

void Quad::updateBVecs() {
    _bv_pos.update();
    _bv_scale.update();
    _bv_color.update();
    _bv_texpos.update();
    _bv_texsize.update();
}

GLUtil::BVec3& Quad::bv_pos() { return _bv_pos; }
GLUtil::BVec3& Quad::bv_scale() { return _bv_scale; }
GLUtil::BVec4& Quad::bv_color() { return _bv_color; }
GLUtil::BVec3& Quad::bv_texpos() { return _bv_texpos; }
GLUtil::BVec2& Quad::bv_texsize() { return _bv_texsize; }

Transform& Quad::transform() {
    return _transform;
}

AnimationState& Quad::animationstate() {
    return _animationstate;
}

void Quad::writeTransform() {
    // write transform data to quad
    _bv_pos.v = _transform.pos;
    _bv_scale.v = _transform.pos;
}

void Quad::writeAnimation() {
    if (!_animationstate.hasAnimation() || _animationstate.animationEmpty())
        return;
    
    // write frame data to quad
    _bv_texpos.v = _animationstate.current().texpos;
    _bv_texsize.v = _animationstate.current().texsize;
}

// _______________________________________ Shaders _______________________________________

const char* const vert_shader_str = R"(
    #version 460

    layout(location = 0) in vec4 v_model;
    layout(location = 1) in vec3 v_pos;
    layout(location = 2) in vec3 v_scale;
    layout(location = 3) in vec4 v_color;
    layout(location = 4) in vec3 v_texpos;
    layout(location = 5) in vec2 v_texsize;
    layout(location = 6) in float v_draw;

    layout(location = 7) uniform mat4 u_view;
    layout(location = 8) uniform mat4 u_proj;

    out vec3 f_pos;
    out vec3 f_scale;
    out vec4 f_color;
    out vec3 f_texcoords;
    out float f_draw;

    vec2 halfround(vec2 v) {
	    return floor(v) + vec2(0.5);	
    }

    void main() {
        // pass position, scale, and color to fragment shader
        f_pos = v_pos;
        f_scale = v_scale;
        f_color = v_color;

        // get final texture coordinates by adding: texsize multiplied by model positions (are either 0.0 or 1.0), and flip the vertical shift
        f_texcoords = 
            v_texpos 
            + 
            (
                vec3(v_texsize, 0.0)
                * vec3(v_model.x, 1.0 - v_model.y, 0.0)
            )
        ;

        f_type = v_type;
        f_draw = v_draw;
        
        // get final pos by shifting unit model to center, scaling it by attribute scale, and adding attribute pos
        vec4 final_pos = 
            (
                (
                    (v_model + vec4(-0.5f, -0.5f, 0.0f, 0.0f)) 
                    * vec4(v_scale, 1.0f)
                ) 
                + vec4(v_pos, 0.0f)
            ) 
            * v_draw
        ;

        gl_Position = u_proj * u_view * round(final_pos);
    }
)";

const char * const frag_shader_str = R"(
    #version 460

    layout(location = 9) uniform sampler2DArray texsamplerarray;
    layout(location = 10) uniform uvec3 texarraydims;
    layout(location = 11) uniform uvec2 windowspace;
    layout(location = 12) uniform uvec3 pixelspace;

    in vec3 f_pos;
    in vec3 f_scale;
    in vec4 f_color;
    in vec3 f_texcoords;
    in float f_draw;

    out vec4 fragcolor;

    vec2 halfround(vec2 v) {
	    return floor(v) + vec2(0.5);	
    }

    void main() {
        if (f_draw == 0.0)
            discard;

        // normalize "raw" texture coordinates with full texture size
        vec4 texel = texture(texsamplerarray, f_texcoords / vec3(texarraydims.xy, 1));

        if (texel.xyz == vec3(255.0 / 255.0, 0.0, 128.0 / 255.0))
            discard;
        
        fragcolor = texel * f_color;

    }
)";

// _______________________________________ GLEnv _______________________________________

GLEnv::GLEnv(unsigned maxcount) : _initialized(false) {
    init(maxcount);
}

GLEnv::GLEnv(GLEnv&& other) {
    operator=(std::move(other));
}

GLEnv::GLEnv() : _max_count(0), _initialized(false) {}
GLEnv::~GLEnv() {
    uninit();
}

GLEnv& GLEnv::operator=(GLEnv&& other) {
    if (this != &other) {
        _stage = std::move(other._stage);
        _texarray = std::move(other._texarray);
        _glb_modelbuf = std::move(other._glb_modelbuf);
        _glb_elembuf = std::move(other._glb_elembuf);
        _glb_pos = std::move(other._glb_pos);
        _glb_scale = std::move(other._glb_scale);
        _glb_color = std::move(other._glb_color);
        _glb_texpos = std::move(other._glb_texpos);
        _glb_texsize = std::move(other._glb_texsize);
        _glb_draw = std::move(other._glb_draw);
        _quad_offsets = other._quad_offsets;
        _quads = other._quads;
        _max_count = other._max_count;
        _count = other._count;
        _quadinfos = other._quadinfos;
        _initialized = other._initialized;
        other._quad_offsets.clear();
        other._quads.clear();
        other._max_count = 0;
        other._quadinfos.clear();
        other._initialized = false;
    }
    return *this;
}

void GLEnv::init(unsigned max_count) {
    if (_initialized)
        throw InitializedException();
    
    /* initialize members */
    _glb_modelbuf = GLUtil::GLBuffer(GL_STATIC_DRAW, 16 * sizeof(GLfloat));
    _glb_elembuf = GLUtil::GLBuffer(GL_STATIC_DRAW, 6 * sizeof(GLuint));
    _glb_pos = GLUtil::GLBuffer(GL_DYNAMIC_DRAW, (max_count * 3) * sizeof(GLfloat));
    _glb_scale = GLUtil::GLBuffer(GL_DYNAMIC_DRAW, (max_count * 3) * sizeof(GLfloat));
    _glb_color = GLUtil::GLBuffer(GL_DYNAMIC_DRAW, (max_count * 4) * sizeof(GLfloat));
    _glb_texpos = GLUtil::GLBuffer(GL_DYNAMIC_DRAW, (max_count * 3) * sizeof(GLfloat));
    _glb_texsize = GLUtil::GLBuffer(GL_DYNAMIC_DRAW, (max_count * 2) * sizeof(GLfloat));
    _glb_draw = GLUtil::GLBuffer(GL_DYNAMIC_DRAW, (max_count * 1) * sizeof(GLfloat));
    _quads = std::vector<Quad>(max_count, Quad());
    _max_count = max_count;
    _count = 0;

    /* setup variables */

    // store shader code and types into arrays for shader program generation call
    const char* shaders[2];
    shaders[0] = vert_shader_str;
    shaders[1] = frag_shader_str;
    GLuint types[2];
    types[0] = GL_VERTEX_SHADER;
    types[1] = GL_FRAGMENT_SHADER;
    // prepare instance model data and elements (unit quad positioned at (0, 0) to (1, 1))
    GLfloat data_model[] = {
        0.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f, 
        1.0f, 1.0f, 0.0f, 1.0f
    };
    GLuint data_elem[] = {
        0, 1, 2, 1, 2, 3
    };

    /* set up shader program */

    // generate and use program
    _stage.init(shaders, types, 2);

    // set format of attributes (model vertices, position, scale, color, texture position, texture size, draw flag)
    _stage.setAttribFormat(0, 4, GL_FLOAT, 0, 0);
    _stage.setAttribFormat(1, 3, GL_FLOAT, 0, 1);
    _stage.setAttribFormat(2, 3, GL_FLOAT, 0, 1);
    _stage.setAttribFormat(3, 4, GL_FLOAT, 0, 1);
    _stage.setAttribFormat(4, 3, GL_FLOAT, 0, 1);
    _stage.setAttribFormat(5, 2, GL_FLOAT, 0, 1);
    _stage.setAttribFormat(6, 1, GL_FLOAT, 0, 1);

    // set attribute buffer indices to 0-6
    _stage.setAttribBufferIndex(0, 0);
    _stage.setAttribBufferIndex(1, 1);
    _stage.setAttribBufferIndex(2, 2);
    _stage.setAttribBufferIndex(3, 3);
    _stage.setAttribBufferIndex(4, 4);
    _stage.setAttribBufferIndex(5, 5);
    _stage.setAttribBufferIndex(6, 6);

    /* set up buffers */

    // write instance model data and elements to buffers
    _glb_modelbuf.subData(sizeof(data_model), data_model, 0 * sizeof(GLfloat));
    _glb_elembuf.subData(sizeof(data_elem), data_elem, 0 * sizeof(GLfloat));

    // bind buffers to attribute buffer indices
    _stage.bindBufferToIndex(_glb_modelbuf.handle(), 0, 0, 4 * sizeof(GLfloat));
    _stage.bindBufferToIndex(_glb_pos.handle(), 1, 0, 3 * sizeof(GLfloat));
    _stage.bindBufferToIndex(_glb_scale.handle(), 2, 0, 3 * sizeof(GLfloat));
    _stage.bindBufferToIndex(_glb_color.handle(), 3, 0, 4 * sizeof(GLfloat));
    _stage.bindBufferToIndex(_glb_texpos.handle(), 4, 0, 3 * sizeof(GLfloat));
    _stage.bindBufferToIndex(_glb_texsize.handle(), 5, 0, 2 * sizeof(GLfloat));
    _stage.bindBufferToIndex(_glb_draw.handle(), 6, 0, 1 * sizeof(GLfloat));
    _stage.bindElementBuffer(_glb_elembuf.handle());

    // use program
    _stage.use();

    // initialize texture array, bind it, set sampler to texture image unit and make that unit active
    _texarray.init();
    _texarray.bind(GL_TEXTURE_2D_ARRAY);
    _stage.uniform1i(9, 0);
    glActiveTexture(GL_TEXTURE0);

    _initialized = true;
}

void GLEnv::uninit() {
    if (!_initialized)
        return;
    
    _stage.uninit();
    _texarray.uninit();
    _glb_modelbuf.uninit();
    _glb_elembuf.uninit();
    _glb_pos.uninit();
    _glb_scale.uninit();
    _glb_color.uninit();
    _glb_texpos.uninit();
    _glb_texsize.uninit();
    _glb_draw.uninit();
    _quad_offsets.clear();
    _quads.clear();
    _max_count = 0;
    _quadinfos.clear();
    _initialized = false;
}

void GLEnv::addQuad(QuadInfo quadinfo, const char* name) {
    _quadinfos[name] = quadinfo;
}

unsigned GLEnv::genQuad(glm::vec3 pos, glm::vec3 scale, glm::vec4 color, glm::vec3 texpos, glm::vec2 texsize) {
    // if number of active offsets is greater than or equal to maximum allowed count, throw
    if (_count >= _max_count)
        throw CountLimitException();

    // get a new unique offset and prepare clean Quad instance
    unsigned offset = _quad_offsets.push();
    _quads[offset] = Quad();
    Quad &q = _quads[offset];

    // set BVec buffers and offsets into them
    q._bv_pos.setBuffer(&_glb_pos, offset * (3 * sizeof(GLfloat)));
    q._bv_scale.setBuffer(&_glb_scale, offset * (3 * sizeof(GLfloat)));
    q._bv_color.setBuffer(&_glb_color, offset * (4 * sizeof(GLfloat)));
    q._bv_texpos.setBuffer(&_glb_texpos, offset * (3 * sizeof(GLfloat))); 
    q._bv_texsize.setBuffer(&_glb_texsize, offset * (2 * sizeof(GLfloat)));

    q.transform() = Transform{pos, scale};
    q._bv_color.v = color;
    q._bv_texpos.v = texpos;
    q._bv_texsize.v = texsize;

    q.writeTransform();
    q.writeAnimation();

    // set the draw flag
    GLfloat draw = 1.0f;
    _glb_draw.subData(sizeof(GLfloat), &draw, offset * (1 * sizeof(GLfloat)));

    _count++;
    return offset;
}
unsigned GLEnv::genQuad(const char* quad_name) {
    // if number of active offsets is greater than or equal to maximum allowed count, throw
    if (_count >= _max_count)
        throw CountLimitException();

    // get a new unique offset and prepare clean Quad instance
    unsigned offset = _quad_offsets.push();
    _quads[offset] = Quad();
    Quad &q = _quads[offset];

    // set BVec buffers and offsets into them
    q._bv_pos.setBuffer(&_glb_pos, offset * (3 * sizeof(GLfloat)));
    q._bv_scale.setBuffer(&_glb_scale, offset * (3 * sizeof(GLfloat)));
    q._bv_color.setBuffer(&_glb_color, offset * (4 * sizeof(GLfloat)));
    q._bv_texpos.setBuffer(&_glb_texpos, offset * (3 * sizeof(GLfloat))); 
    q._bv_texsize.setBuffer(&_glb_texsize, offset * (2 * sizeof(GLfloat)));

    QuadInfo &qi = _quadinfos[quad_name];

    q.transform() = qi.transform;
    q._bv_color.v = qi.color;
    q.animationstate().setAnimation(&(qi.animation));

    q.writeTransform();
    q.writeAnimation();

    // set the draw flag
    GLfloat draw = 1.0f;
    _glb_draw.subData(sizeof(GLfloat), &draw, offset * (1 * sizeof(GLfloat)));

    _count++;
    return offset;
}

void GLEnv::remove(unsigned offset) {
    if (offset >= _quad_offsets.size())
        throw std::out_of_range("Index out of range");

    // call _quad_offsets to make the offset usable again
    _quad_offsets.remove(offset);

    // unset the draw flag for this specific offset, zeroing out its instance
    GLfloat draw = 0.0f;
    _glb_draw.subData(sizeof(GLfloat), &draw, offset * (1 * sizeof(GLfloat)));

    _count--;
}

void GLEnv::setTexArray(GLuint width, GLuint height, GLuint depth) {
    _texarray.alloc(1, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, width, height, depth);
    _stage.uniform3ui(10, glm::uvec3(width, height, depth));
}

void GLEnv::setTexture(Image img, GLuint xoffset, GLuint yoffset, GLuint zoffset) {
    unsigned char* image_data = img.copyData();
    _texarray.subImage(0, xoffset, yoffset, zoffset, img.width(), img.height(), 1, image_data);
    delete image_data;
}

void GLEnv::setView(glm::mat4 view) {
    _stage.uniformmat4f(7, view);
}

void GLEnv::setProj(glm::mat4 proj) {
    _stage.uniformmat4f(8, proj);
}

void GLEnv::setWindowSpace(GLuint width, GLuint height) {
    _stage.uniform2ui(11, glm::uvec2(width, height));
}

void GLEnv::setPixelSpace(GLuint width, GLuint height, GLuint depth) {
    _stage.uniform3ui(12, glm::uvec3(width, height, depth));
}

void GLEnv::update() {
    for (unsigned i = 0; i < _quad_offsets.size(); i++)
        // only try calling update on index i if it is an active offset in _quad_offsets
        if (_quad_offsets[i])
            _quads[i].updateBVecs();
}

void GLEnv::drawQuads() {
    // draw a number of instances equal to the number of offsets in system, active or not, using the vertices in the element buffer
    // (Instances with inactive offets will be zeroed out per the draw flag)
    GLUtil::renderInst(GL_TRIANGLES, 6, _quad_offsets.size(), true);
}

Quad *GLEnv::getQuad(unsigned offset) {
    if (offset >= _quad_offsets.size())
        throw std::out_of_range("Index out of range");

    if (_quad_offsets.at(offset))
        return &_quads[offset];
    
    throw InactiveIntException();
}

std::vector<unsigned> GLEnv::getOffsets() { return _quad_offsets.getUsed(); }

bool GLEnv::hasOffset(unsigned offset) { return _quad_offsets.at(offset); }

bool GLEnv::getInitialized() { return _initialized; }