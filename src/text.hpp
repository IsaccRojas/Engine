#ifndef TEXT_HPP_
#define TEXT_HPP_

#include "../core/include/entity.hpp"

/* collection of quads to represent text */
class Text {
    GLEnv *_glenv;
    std::vector<int> _quadids;

    int _tex_x;
    int _tex_y;
    int _tex_z;
    int _tex_rows;
    int _tex_columns;
    int _text_width;
    int _text_height;
    int _text_xoff;
    int _text_yoff;
    int _spacing;

    std::string _textstr;
    glm::vec3 _pos;
    
    bool _update;
public:
    Text(GLEnv *glenv);
    ~Text();

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
    void textConfig(int tex_x, int tex_y, int tex_z, int tex_rows, int tex_columns, int text_width, int text_height, int text_xoff, int text_yoff, int spacing);

    void setText(const char *str);
    void setPos(glm::vec3 pos);
    void update();
};

#endif