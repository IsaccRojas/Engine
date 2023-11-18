#include "glenv.hpp"

// _______________________________________ Quad _______________________________________

Quad::Quad(BVec3 position, BVec3 quadscale, BVec3 textureposition, BVec2 texturesize) : pos(position), scale(quadscale), texpos(textureposition), texsize(texturesize) {}
Quad::Quad(const Quad &other) {
    pos = other.pos;
    scale = other.scale;
    texpos = other.texpos;
    texsize = other.texsize;
}
Quad::Quad() {}
Quad::~Quad() {}

Quad& Quad::operator=(const Quad& other) {
    pos = other.pos;
    scale = other.scale;
    texpos = other.texpos;
    texsize = other.texsize;
    return *this;
};


void Quad::update() {
    pos.update();
    scale.update();
    texpos.update();
    texsize.update();
}

// _______________________________________ GLEnv _______________________________________

GLEnv::GLEnv(int maxcount) :
    _maxcount(maxcount),
    _glb_modelbuf(GL_STATIC_DRAW, 16 * sizeof(GLfloat)), 
    _glb_elembuf(GL_STATIC_DRAW, 6 * sizeof(GLuint)), 
    _glb_pos(GL_DYNAMIC_DRAW, (maxcount * 3) * sizeof(GLfloat)),
    _glb_scale(GL_DYNAMIC_DRAW, (maxcount * 3) * sizeof(GLfloat)),
    _glb_texpos(GL_DYNAMIC_DRAW, (maxcount * 3) * sizeof(GLfloat)), 
    _glb_texsize(GL_DYNAMIC_DRAW, (maxcount * 2) * sizeof(GLfloat)), 
    _glb_draw(GL_DYNAMIC_DRAW, (maxcount * 1) * sizeof(GLfloat)),
    _quads(maxcount, Quad())
{
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
    _stage.program(shaders, types, 2);
    _stage.use();
    // set up attributes with instancing (model vertices, position, scale, texture position, texture size, draw flag)
    _stage.formatattrib(0, 4, GL_FLOAT, 0, 0);
    _stage.formatattrib(1, 3, GL_FLOAT, 0, 1);
    _stage.formatattrib(2, 3, GL_FLOAT, 0, 1);
    _stage.formatattrib(3, 3, GL_FLOAT, 0, 1);
    _stage.formatattrib(4, 2, GL_FLOAT, 0, 1);
    _stage.formatattrib(5, 1, GL_FLOAT, 0, 1);
    // bind attribute to buffer storage locations 0-4
    _stage.bindattrib(0, 0);
    _stage.bindattrib(1, 1);
    _stage.bindattrib(2, 2);
    _stage.bindattrib(3, 3);
    _stage.bindattrib(4, 4);
    _stage.bindattrib(5, 5);

    /* set up buffers */

    // write instance model data and elements to buffers, bind model buffer to storage location 0, 
    // and bind element buffer to OpenGL system
    _glb_modelbuf.subdata(sizeof(data_model), data_model, 0 * sizeof(GLfloat));
    _glb_elembuf.subdata(sizeof(data_elem), data_elem, 0 * sizeof(GLfloat));
    _glb_modelbuf.bindindex(0, 0, 4 * sizeof(GLfloat));
    _stage.element(_glb_elembuf.handle());

    // bind position, texture position, texture size and draw flag buffers to storage locations 1-4
    _glb_pos.bindindex(1, 0, 3 * sizeof(GLfloat));
    _glb_scale.bindindex(2, 0, 3 * sizeof(GLfloat));
    _glb_texpos.bindindex(3, 0, 3 * sizeof(GLfloat));
    _glb_texsize.bindindex(4, 0, 2 * sizeof(GLfloat));
    _glb_draw.bindindex(5, 0, 1 * sizeof(GLfloat));
}
GLEnv::~GLEnv() {}

void GLEnv::settexarray(GLuint width, GLuint height, GLuint depth) {
    _texarray.alloc(1, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, width, height, depth);
}

void GLEnv::settexture(Image img, GLuint xoffset, GLuint yoffset, GLuint zoffset) {
    _texarray.subimage(0, xoffset, yoffset, zoffset, img.width(), img.height(), 1, img.copydata());
}

void GLEnv::setviewport(GLint x, GLint y, GLint width, GLint height) {
    glViewport(x, y, width, height);
}

void GLEnv::setview(glm::mat4 view) {
    glUniformMatrix4fv(6, 1, GL_FALSE, glm::value_ptr(view));
}

void GLEnv::setproj(glm::mat4 proj) {
    glUniformMatrix4fv(7, 1, GL_FALSE, glm::value_ptr(proj));
}

int GLEnv::genQuad(glm::vec3 pos, glm::vec3 scale, glm::vec3 texpos, glm::vec2 texsize) {
    // if number of active IDs is greater than or equal to maximum allowed count, return -1
    if (_ids.fillsize() >= _maxcount)
        return -1;

    // get a new unique ID
    int id = _ids.push();
    
    // generate quad by providing the position, texture position, and texture size buffers, and
    // use id to specify an offset into them
    _quads[id] = Quad(
        BVec3(&_glb_pos, id * (3 * sizeof(GLfloat))),
        BVec3(&_glb_scale, id * (3 * sizeof(GLfloat))),
        BVec3(&_glb_texpos, id * (3 * sizeof(GLfloat))), 
        BVec2(&_glb_texsize, id * (2 * sizeof(GLfloat)))
    );

    // initialize quad with parameters
    _quads[id].pos.v = pos;
    _quads[id].scale.v = scale;
    _quads[id].texpos.v = texpos;
    _quads[id].texsize.v = texsize;

    // write 1 into draw buffer, setting the draw flag for this quad
    GLfloat draw = 1.0f;
    _glb_draw.subdata(sizeof(GLfloat), &draw, id * (1 * sizeof(GLfloat)));

    return id;
}

Quad *GLEnv::get(int i) {
    return &_quads[i];
}

void GLEnv::update() {
    for (int i = 0; i < _ids.size(); i++)
        // only try calling update on index i if it is an active ID in _ids
        if (_ids[i])
            _quads[i].update();
}

int GLEnv::erase(int id) {
    // if attempting to erase an id from empty system, return -1
    if (_ids.empty())
        return -1;

    // call _ids to make the ID usable again
    _ids.erase_at(id);

    // unset the draw flag for this specific offset, zeroing out its instance
    GLfloat draw = 0.0f;
    _glb_draw.subdata(sizeof(GLfloat), &draw, id * (1 * sizeof(GLfloat)));

    return 0;
}

void GLEnv::draw() {
    // draw a number of instances equal to the number of IDs in system, active or not, using the element buffer
    // (Instances with inactive IDs will be zeroed out per the draw flag)
    _stage.renderinst(6, _ids.size());
}

std::vector<int> GLEnv::getids() {
    return _ids.getused();
}