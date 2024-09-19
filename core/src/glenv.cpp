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
        _quad_ids = other._quad_ids;
        _quads = other._quads;
        _max_count = other._max_count;
        _count = other._count;
        _initialized = other._initialized;
        other._quad_ids.clear();
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

    // write instance model data and elements to buffers and bind element buffer to OpenGL system
    _glb_modelbuf.subData(sizeof(data_model), data_model, 0 * sizeof(GLfloat));
    _glb_elembuf.subData(sizeof(data_elem), data_elem, 0 * sizeof(GLfloat));
    GLUtil::setElementBuffer(_glb_elembuf.handle());

    // bind buffers to attribute buffer indices
    _stage.bindBufferToIndex(_glb_modelbuf.handle(), 0, 0, 4 * sizeof(GLfloat));
    _stage.bindBufferToIndex(_glb_pos.handle(), 1, 0, 3 * sizeof(GLfloat));
    _stage.bindBufferToIndex(_glb_scale.handle(), 2, 0, 3 * sizeof(GLfloat));
    _stage.bindBufferToIndex(_glb_texpos.handle(), 3, 0, 3 * sizeof(GLfloat));
    _stage.bindBufferToIndex(_glb_texsize.handle(), 4, 0, 2 * sizeof(GLfloat));
    _stage.bindBufferToIndex(_glb_draw.handle(), 5, 0, 1 * sizeof(GLfloat));

    // initialize texture array
    _texarray.init();

    // use program
    _stage.use();
    
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
    _quad_ids.clear();
    _quads.clear();
    _max_count = 0;
    _initialized = false;
}

unsigned GLEnv::genQuad(glm::vec3 pos, glm::vec3 scale, glm::vec3 texpos, glm::vec2 texsize) {
    // if number of active IDs is greater than or equal to maximum allowed count, throw
    if (_count >= _max_count)
        throw CountLimitException();

    // get a new unique ID
    unsigned id = _quad_ids.push();
    
    // generate quad by providing the position, texture position, and texture size buffers, and
    // use id to specify an offset into them
    _quads[id] = Quad(
        GLUtil::BVec3(&_glb_pos, id * (3 * sizeof(GLfloat))),
        GLUtil::BVec3(&_glb_scale, id * (3 * sizeof(GLfloat))),
        GLUtil::BVec3(&_glb_texpos, id * (3 * sizeof(GLfloat))), 
        GLUtil::BVec2(&_glb_texsize, id * (2 * sizeof(GLfloat)))
    );

    // initialize quad with parameters
    _quads[id].bv_pos.v = pos;
    _quads[id].bv_scale.v = scale;
    _quads[id].bv_texpos.v = texpos;
    _quads[id].bv_texsize.v = texsize;

    // write 1 into draw buffer, setting the draw flag for this quad
    GLfloat draw = 1.0f;
    _glb_draw.subData(sizeof(GLfloat), &draw, id * (1 * sizeof(GLfloat)));

    _count++;
    return id;
}

void GLEnv::remove(unsigned id) {
    if (id >= _quad_ids.size())
        throw std::out_of_range("Index out of range");

    // call _quad_ids to make the ID usable again
    _quad_ids.remove(id);

    // unset the draw flag for this specific offset, zeroing out its instance
    GLfloat draw = 0.0f;
    _glb_draw.subData(sizeof(GLfloat), &draw, id * (1 * sizeof(GLfloat)));

    _count--;
}

void GLEnv::setTexArray(GLuint width, GLuint height, GLuint depth) {
    _texarray.alloc(1, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, width, height, depth);
    glUniform3ui(9, width, height, depth);
}

void GLEnv::setTexture(Image img, GLuint xoffset, GLuint yoffset, GLuint zoffset) {
    unsigned char *image_data = img.copyData();
    _texarray.subImage(0, xoffset, yoffset, zoffset, img.width(), img.height(), 1, image_data);
    delete image_data;
}

void GLEnv::setView(glm::mat4 view) {
    glUniformMatrix4fv(6, 1, GL_FALSE, glm::value_ptr(view));
}

void GLEnv::setProj(glm::mat4 proj) {
    glUniformMatrix4fv(7, 1, GL_FALSE, glm::value_ptr(proj));
}

void GLEnv::update() {
    for (unsigned i = 0; i < _quad_ids.size(); i++)
        // only try calling update on index i if it is an active ID in _quad_ids
        if (_quad_ids[i])
            _quads[i].update();
}

void GLEnv::drawQuads() {
    // draw a number of instances equal to the number of IDs in system, active or not, using the vertices in the element buffer
    // (Instances with inactive IDs will be zeroed out per the draw flag)
    GLUtil::renderInst(GL_TRIANGLES, 6, _quad_ids.size());
}

Quad *GLEnv::getQuad(unsigned id) {
    if (id >= _quad_ids.size())
        throw std::out_of_range("Index out of range");

    if (_quad_ids.at(id))
        return &_quads[id];
    
    throw InactiveIDException();
}

std::vector<unsigned> GLEnv::getIDs() { return _quad_ids.getUsed(); }

bool GLEnv::hasID(unsigned id) { return _quad_ids.at(id); }

bool GLEnv::getInitialized() { return _initialized; }