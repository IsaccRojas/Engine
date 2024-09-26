#include "../include/text.hpp"

Text::Text(GLEnv *glenv) : 
    _glenv(glenv), 
    _tc(TextConfig{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}),
    _pos(glm::vec3(0.0f)),
    _scale(glm::vec3(1.0f)),
    _update(false)
{}

Text::Text(Text &&other) { operator=(std::move(other)); }

Text::~Text() {
    // erase all characters from glenv
    if (_glenv)
        for (unsigned i = 0; i < _quadids.size(); i++)
            _glenv->remove(_quadids[i]);
}

Text &Text::operator=(Text &&other) {
    if (this != &other) {
        _glenv = other._glenv;
        _quadids = other._quadids;
        _tc = other._tc;
        _textstr = other._textstr;
        _pos = other._pos;
        _scale = other._scale;
        _update = other._update;
        other._glenv = nullptr;
        other._quadids.clear();
        other._tc = TextConfig{};
        other._pos = glm::vec3(0.0f);
        other._scale = glm::vec3(0.0f);
        other._update = false;
    }
    return *this;
}

/*
Text is expected to be in the order of ASCII format, starting with the space character (decimal 32).

- tex_x - X coordinate in the texture space the character sheet is located
- tex_y - Y coordinate in the texture space the character sheet is located
- tex_z - Z coordinate in the texture space the character sheet is located
- tex_rows - number of rows in sheet
- tex_columns - number of columns in sheet
- text_width - width of a single character
- text_height - height of a single character
- text_xoff - horizontal spacing between characters in sheet
- text_yoff - vertical spacing between characters in sheet
- spacing - space to place between characters in final text
*/
void Text::setTextConfig(TextConfig textconfig) {
    _tc = textconfig;
    _update = true;
}

void Text::setText(const char *str) {
    if (_textstr == str)
        return;
    
    _textstr = str;
    _update = true;
}

void Text::setPos(glm::vec3 pos) {
    if (_pos == pos)
        return;
    
    _pos = pos;
    _update = true;
}

void Text::setScale(glm::vec3 scale) {
    if (_scale == scale)
        return;
    
    _scale = scale;
    _update = true;
}

void Text::update() {
    // do nothing if no changes were made to text, position or configuration
    if (!_update)
        return;
    
    if (!_glenv)
        throw std::runtime_error("WARN: attempt to update Text with null GLEnv reference");

    int l_ids = _quadids.size();
    int l_str = _textstr.size();

    // can only be true if previous generation has occurred
    if (l_str < l_ids) {
        // erase extra characters from glenv
        for (int i = l_str; i < l_ids; i++)
            _glenv->remove(_quadids[i]);
        
        // create new vector with previous IDs
        std::vector<int> _newids;
        for (int i = 0; i < l_str; i++)
            _newids.push_back(_quadids[i]);
        _quadids = _newids;

    } else {
        // push new IDs to vector (TODO: assign proper values on generation)
        for (int i = l_ids; i < l_str; i++)
            _quadids.push_back(_glenv->genQuad(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec4(1.0f), glm::vec3(0.0f), glm::vec2(0.0f), GLE_RECT));
    }
    
    // set position start to be half-way leftward across complete text width, to center the text
    Quad *quad;
    int shift = (_tc.text_width * _scale.x) + _tc.spacing;
    float pos_start = ((l_str * (_tc.text_width * _scale.x)) + ((l_str - 1) * _tc.spacing)) * -0.5f;
    
    // update IDs with new character information
    for (int i = 0; i < l_str; i++) {
        quad = _glenv->getQuad(_quadids[i]);
        int charpos = int(_textstr[i]) - 32;

        // set values according to configuration and string
        quad->bv_pos.v = _pos + glm::vec3(pos_start + float(shift * i), 0.0f, 0.0f);
        quad->bv_scale.v = glm::vec3(_tc.text_width, _tc.text_height, 0.0f) * _scale;
        quad->bv_texpos.v = glm::vec3(
            _tc.tex_x + ((_tc.text_width + _tc.text_xoff) * (charpos % _tc.tex_columns)),
            _tc.tex_y + ((_tc.text_height + _tc.text_yoff) * int(charpos / _tc.tex_columns)),
            _tc.tex_z
        );
        quad->bv_texsize.v = glm::vec2(_tc.text_width, _tc.text_height);
    }

    _update = false;
};