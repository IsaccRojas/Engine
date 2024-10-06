#ifndef TEXT_HPP_
#define TEXT_HPP_

#include "../include/glenv.hpp"

/*
Characters are expected to be in the order of ASCII format, starting with the space character (decimal 32).

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
struct TextConfig {
    int tex_x = 0;
    int tex_y = 0;
    int tex_z = 0;
    int tex_rows = 0;
    int tex_columns = 0;
    int text_width = 0;
    int text_height = 0;
    int text_xoff = 0;
    int text_yoff = 0;
    int spacing = 0;
    // default copy assignment/construction are fine
};

/* collection of quads to represent text */
class Text {
    GLEnv *_glenv;
    std::vector<int> _quadids;

    TextConfig _tc;

    std::string _textstr;
    glm::vec3 _pos;
    glm::vec3 _scale;
    
    bool _update;

public:
    Text(GLEnv *glenv);
    Text(Text &&other);
    Text(const Text &other) = delete;
    ~Text();

    Text &operator=(Text &&other);
    Text &operator=(const Text &other) = delete;

    void setTextConfig(TextConfig textconfig);
    void setText(const char *str);
    void setPos(glm::vec3 pos);
    void setScale(glm::vec3 scale);

    /* Writes to internal Quad data with current configuration, text, position and scale data. */
    void writeText();
};

#endif