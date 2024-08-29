#include "text.hpp"

Text::Text(GLEnv *glenv) : 
    _glenv(glenv), 
    _tex_x(0), 
    _tex_y(0), 
    _tex_z(0), 
    _tex_rows(0), 
    _tex_columns(0), 
    _text_width(0), 
    _text_height(0), 
    _text_xoff(0), 
    _text_yoff(0), 
    _spacing(0),
    _update(false)
{}
Text::~Text() {
    // erase all characters from glenv
    if (_glenv)
        for (int i = 0; i < _quadids.size(); i++)
            _glenv->remove(_quadids[i]);
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
void Text::textConfig(int tex_x, int tex_y, int tex_z, int tex_rows, int tex_columns, int text_width, int text_height, int text_xoff, int text_yoff, int spacing) {
    _tex_x = tex_x;
    _tex_y = tex_y;
    _tex_z = tex_z;
    _tex_rows = tex_rows;
    _tex_columns = tex_columns;
    _text_width = text_width;
    _text_height = text_height;
    _text_xoff = text_xoff;
    _text_yoff = text_yoff;
    _spacing = spacing;
    _update = true;
}

void Text::setText(const char *str) {
    _textstr = str;
    _update = true;
}

void Text::setPos(glm::vec3 pos) {
    _pos = pos;
    _update = true;
}

void Text::update() {
    // do nothing if no changes were made to text, position or configuration
    if (!_update)
        return;
    
    if (!_glenv) {
        std::cerr << "WARN: attempt to update with null GLEnv reference in Text instance " << this << std::endl;
        // throw error maybe
        return;
    }

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
            _quadids.push_back(_glenv.genQuad(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec2(0.0f)));
    }
    
    // update IDs with new character information
    Quad *quad;
    int shift = _text_width + _text_xoff;
    for (int i = 0; i < l_str; i++) {
        quad = _glenv->get(_quadids[i]);

        // set values according to configuration and string
        quad->pos.v = _pos + glm::vec3(float(shift * i), 0.0f, 0.0f);
        quad->scale.v = glm::vec3(_text_width, _text_height, 1.0f);
        quad->texpos.v = glm::vec2(
            _tex_x + ((_text_width + _text_xoff) * (i % _tex_columns)),
            _tex_y + ((_text_height + _text_yoff) * (i / _tex_rows)),
            _tex_z
        );
        quad->texsize.v = glm::vec3(_text_width, _text_height);
    }

    _update = false;
};