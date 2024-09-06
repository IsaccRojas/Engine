#include "../include/glenv.hpp"

// throws if initialized is true
void _checkInitialized(bool initialized) {
    if (initialized)
        throw InitializedException();
}

// throws if initialized is false
void _checkUninitialized(bool initialized) {
    if (!initialized)
        throw UninitializedException();
}

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
        _ids = other._ids;
        _quads = other._quads;
        _max_count = other._max_count;
        _count = other._count;
        _initialized = other._initialized;
        other._ids.clear();
        other._quads.clear();
        other._max_count = 0;
        other._initialized = false;
    }
    return *this;
}


void GLEnv::init(unsigned max_count) {
    _checkInitialized(_initialized);

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
    _stage.use();

    // set up attributes with instancing (model vertices, position, scale, texture position, texture size, draw flag, texarray dimensions)
    GLUtil::formatAttrib(0, 4, GL_FLOAT, 0, 0);
    GLUtil::formatAttrib(1, 3, GL_FLOAT, 0, 1);
    GLUtil::formatAttrib(2, 3, GL_FLOAT, 0, 1);
    GLUtil::formatAttrib(3, 3, GL_FLOAT, 0, 1);
    GLUtil::formatAttrib(4, 2, GL_FLOAT, 0, 1);
    GLUtil::formatAttrib(5, 1, GL_FLOAT, 0, 1);

    // bind attribute to buffer storage locations 0-4
    GLUtil::bindAttrib(0, 0);
    GLUtil::bindAttrib(1, 1);
    GLUtil::bindAttrib(2, 2);
    GLUtil::bindAttrib(3, 3);
    GLUtil::bindAttrib(4, 4);
    GLUtil::bindAttrib(5, 5);

    /* set up buffers */

    // write instance model data and elements to buffers, bind model buffer to storage location 0, 
    // and bind element buffer to OpenGL system
    _glb_modelbuf.subData(sizeof(data_model), data_model, 0 * sizeof(GLfloat));
    _glb_elembuf.subData(sizeof(data_elem), data_elem, 0 * sizeof(GLfloat));
    _glb_modelbuf.bindIndex(0, 0, 4 * sizeof(GLfloat));

    GLUtil::setElementBuffer(_glb_elembuf.handle());

    // bind position, texture position, texture size and draw flag buffers to storage locations 1-4
    _glb_pos.bindIndex(1, 0, 3 * sizeof(GLfloat));
    _glb_scale.bindIndex(2, 0, 3 * sizeof(GLfloat));
    _glb_texpos.bindIndex(3, 0, 3 * sizeof(GLfloat));
    _glb_texsize.bindIndex(4, 0, 2 * sizeof(GLfloat));
    _glb_draw.bindIndex(5, 0, 1 * sizeof(GLfloat));

    // initialize texture array
    _texarray.init();
    
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
    _ids.clear();
    _quads.clear();
    _max_count = 0;
    _initialized = false;
}

void GLEnv::setTexArray(GLuint width, GLuint height, GLuint depth) {
    _checkUninitialized(_initialized);

    _texarray.alloc(1, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, width, height, depth);
    glUniform3ui(9, width, height, depth);
}

void GLEnv::setTexture(Image img, GLuint xoffset, GLuint yoffset, GLuint zoffset) {
    _checkUninitialized(_initialized);

    _texarray.subImage(0, xoffset, yoffset, zoffset, img.width(), img.height(), 1, img.copyData());
}

void GLEnv::setView(glm::mat4 view) {
    _checkUninitialized(_initialized);

    glUniformMatrix4fv(6, 1, GL_FALSE, glm::value_ptr(view));
}

void GLEnv::setProj(glm::mat4 proj) {
    _checkUninitialized(_initialized);

    glUniformMatrix4fv(7, 1, GL_FALSE, glm::value_ptr(proj));
}

int GLEnv::genQuad(glm::vec3 pos, glm::vec3 scale, glm::vec3 texpos, glm::vec2 texsize) {
    _checkUninitialized(_initialized);

    // if number of active IDs is greater than or equal to maximum allowed count, throw
    if (_count >= _max_count)
        throw CountLimitException();

    // get a new unique ID
    int id = _ids.push();
    
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

Quad *GLEnv::get(int i) {
    _checkUninitialized(_initialized);
    if (i < 0 || i >= _ids.size())
        throw std::out_of_range("Index out of range");

    if (_ids.at(i))
        return &_quads[i];
    
    throw InactiveIDException();
}

void GLEnv::update() {
    _checkUninitialized(_initialized);

    for (unsigned i = 0; i < _ids.size(); i++)
        // only try calling update on index i if it is an active ID in _ids
        if (_ids[i])
            _quads[i].update();
}

void GLEnv::remove(int i) {
    _checkUninitialized(_initialized);
    if (i < 0 || i >= _ids.size())
        throw std::out_of_range("Index out of range");

    // call _ids to make the ID usable again
    _ids.remove(i);

    // unset the draw flag for this specific offset, zeroing out its instance
    GLfloat draw = 0.0f;
    _glb_draw.subData(sizeof(GLfloat), &draw, i * (1 * sizeof(GLfloat)));

    _count--;
}

void GLEnv::draw() {
    _checkUninitialized(_initialized);

    // draw a number of instances equal to the number of IDs in system, active or not, using the element buffer
    // (Instances with inactive IDs will be zeroed out per the draw flag)
    GLUtil::renderInst(6, _ids.size());
}

std::vector<int> GLEnv::getIDs() {
    _checkUninitialized(_initialized);

    return _ids.getUsed();
}

bool GLEnv::getInitialized() { return _initialized; }