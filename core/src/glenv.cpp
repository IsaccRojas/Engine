#include "../include/glenv.hpp"

// _______________________________________ Quad _______________________________________

Quad::Quad(GLUtil::BVec3 position, GLUtil::BVec3 quadscale, GLUtil::BVec3 textureposition, GLUtil::BVec2 texturesize) : bv_pos(position), bv_scale(quadscale), bv_texpos(textureposition), bv_texsize(texturesize) {}
Quad::Quad() {}
Quad::~Quad() { /* automatic destruction is fine */ }

void Quad::update() {
    bv_pos.update();
    bv_scale.update();
    bv_texpos.update();
    bv_texsize.update();
}

// _______________________________________ Point _______________________________________

Point::Point(GLUtil::BVec3 position, GLUtil::BFloat radius, GLUtil::BVec3 color) : bv_pos(position), bf_radius(radius), bv_color(color) {}
Point::Point() {}
Point::~Point() { /* automatic destruction is fine */ }

void Point::update() {
    bv_pos.update();
    bf_radius.update();
    bv_color.update();
}

// _______________________________________ GLEnv _______________________________________

GLEnv::GLEnv(unsigned maxcount) : _initialized(false) {
    init(maxcount);
}

GLEnv::GLEnv(GLEnv &&other) {
    operator=(std::move(other));
}

GLEnv::GLEnv() : _max_count(0), _initialized(false) {}
GLEnv::~GLEnv() {
    uninit();
}

GLEnv& GLEnv::operator=(GLEnv &&other) {
    if (this != &other) {
        _stage = std::move(other._stage);
        _texarray = std::move(other._texarray);
        _glb_modelbuf = std::move(other._glb_modelbuf);
        _glb_elembuf = std::move(other._glb_elembuf);
        _glb_pos = std::move(other._glb_pos);
        _glb_scale = std::move(other._glb_scale);
        _glb_texpos = std::move(other._glb_texpos);
        _glb_texsize = std::move(other._glb_texsize);
        _glb_draw = std::move(other._glb_draw);
        _quad_offsets = other._quad_offsets;
        _quads = other._quads;
        _max_count = other._max_count;
        _count = other._count;
        _initialized = other._initialized;
        other._quad_offsets.clear();
        other._quads.clear();
        other._max_count = 0;
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
    _glb_texpos = GLUtil::GLBuffer(GL_DYNAMIC_DRAW, (max_count * 3) * sizeof(GLfloat));
    _glb_texsize = GLUtil::GLBuffer(GL_DYNAMIC_DRAW, (max_count * 2) * sizeof(GLfloat)); 
    _glb_draw = GLUtil::GLBuffer(GL_DYNAMIC_DRAW, (max_count * 1) * sizeof(GLfloat));
    _quads = std::vector<Quad>(max_count, Quad());
    _max_count = max_count;
    _count = 0;

    /* setup variables */

    // use readfile() to read shader code into strings, and then put them into an array
    std::string vert_shader_str = readfile("vert_shader.glsl");
    std::string frag_shader_str = readfile("frag_shader.glsl");
    const char *shaders[2];
    shaders[0] = vert_shader_str.c_str();
    shaders[1] = frag_shader_str.c_str();
    // provide types in array for shader program generation call
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

    // set format of attributes (model vertices, position, scale, texture position, texture size, draw flag, texarray dimensions)
    _stage.setAttribFormat(0, 4, GL_FLOAT, 0, 0);
    _stage.setAttribFormat(1, 3, GL_FLOAT, 0, 1);
    _stage.setAttribFormat(2, 3, GL_FLOAT, 0, 1);
    _stage.setAttribFormat(3, 3, GL_FLOAT, 0, 1);
    _stage.setAttribFormat(4, 2, GL_FLOAT, 0, 1);
    _stage.setAttribFormat(5, 1, GL_FLOAT, 0, 1);

    // set attribute buffer indices to 0-4
    _stage.setAttribBufferIndex(0, 0);
    _stage.setAttribBufferIndex(1, 1);
    _stage.setAttribBufferIndex(2, 2);
    _stage.setAttribBufferIndex(3, 3);
    _stage.setAttribBufferIndex(4, 4);
    _stage.setAttribBufferIndex(5, 5);

    /* set up buffers */

    // write instance model data and elements to buffers
    _glb_modelbuf.subData(sizeof(data_model), data_model, 0 * sizeof(GLfloat));
    _glb_elembuf.subData(sizeof(data_elem), data_elem, 0 * sizeof(GLfloat));

    // bind buffers to attribute buffer indices
    _stage.bindBufferToIndex(_glb_modelbuf.handle(), 0, 0, 4 * sizeof(GLfloat));
    _stage.bindBufferToIndex(_glb_pos.handle(), 1, 0, 3 * sizeof(GLfloat));
    _stage.bindBufferToIndex(_glb_scale.handle(), 2, 0, 3 * sizeof(GLfloat));
    _stage.bindBufferToIndex(_glb_texpos.handle(), 3, 0, 3 * sizeof(GLfloat));
    _stage.bindBufferToIndex(_glb_texsize.handle(), 4, 0, 2 * sizeof(GLfloat));
    _stage.bindBufferToIndex(_glb_draw.handle(), 5, 0, 1 * sizeof(GLfloat));
    _stage.bindElementBuffer(_glb_elembuf.handle());

    // use program
    _stage.use();

    // initialize texture array
    _texarray.init();
    _texarray.bind(GL_TEXTURE_2D_ARRAY);
    
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
    _glb_texpos.uninit();
    _glb_texsize.uninit();
    _glb_draw.uninit();
    _quad_offsets.clear();
    _quads.clear();
    _max_count = 0;
    _initialized = false;
}

unsigned GLEnv::genQuad(glm::vec3 pos, glm::vec3 scale, glm::vec3 texpos, glm::vec2 texsize) {
    // if number of active offsets is greater than or equal to maximum allowed count, throw
    if (_count >= _max_count)
        throw CountLimitException();

    // get a new unique offset
    unsigned offset = _quad_offsets.push();
    
    // generate quad by providing the position, texture position, and texture size buffers, and
    // specify an offset into them
    _quads[offset] = Quad(
        GLUtil::BVec3(&_glb_pos, offset * (3 * sizeof(GLfloat))),
        GLUtil::BVec3(&_glb_scale, offset * (3 * sizeof(GLfloat))),
        GLUtil::BVec3(&_glb_texpos, offset * (3 * sizeof(GLfloat))), 
        GLUtil::BVec2(&_glb_texsize, offset * (2 * sizeof(GLfloat)))
    );

    // initialize quad with parameters
    _quads[offset].bv_pos.v = pos;
    _quads[offset].bv_scale.v = scale;
    _quads[offset].bv_texpos.v = texpos;
    _quads[offset].bv_texsize.v = texsize;

    // write 1 into draw buffer, setting the draw flag for this quad
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
    _stage.uniform3ui(9, glm::uvec3(width, height, depth));
}

void GLEnv::setTexture(Image img, GLuint xoffset, GLuint yoffset, GLuint zoffset) {
    unsigned char *image_data = img.copyData();
    _texarray.subImage(0, xoffset, yoffset, zoffset, img.width(), img.height(), 1, image_data);
    delete image_data;
}

void GLEnv::setView(glm::mat4 view) {
    _stage.uniformmat4f(6, view);
}

void GLEnv::setProj(glm::mat4 proj) {
    _stage.uniformmat4f(7, proj);
}

void GLEnv::update() {
    for (unsigned i = 0; i < _quad_offsets.size(); i++)
        // only try calling update on index i if it is an active offset in _quad_offsets
        if (_quad_offsets[i])
            _quads[i].update();
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