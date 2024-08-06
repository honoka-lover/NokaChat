//
// Created by honoka on 2024/8/6.
//

#ifndef VANILLA_TEXTTEXTURE_H
#define VANILLA_TEXTTEXTURE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <QOpenGLExtraFunctions>
#include <QWidget>
#include <QOpenGLShaderProgram>
#include "freetype/freetype.h"
struct CharacterID{
    uint ch;
    uint fontsize;
    bool operator == (const CharacterID id) const {
        if(ch == id.ch && fontsize == id.fontsize){
            return true;
        }
        return false;
    }

    bool operator < (const CharacterID id) const{
        if(ch < id.ch){
            return true;
        }else if(ch == id.ch){
            if(fontsize < id.fontsize)
                return true;
        }
        return false;
    }

    bool operator > (const CharacterID id) const {
        if(ch > id.ch){
            return true;
        }else if(ch == id.ch){
            if(fontsize > id.fontsize)
                return true;
        }
        return false;
    }
};

struct CharacterData {
    unsigned int TextureID; // 字符纹理的ID
    glm::ivec2   Size;      // 字符的尺寸
    glm::ivec2   Bearing;   // 字符的边界
    unsigned int Advance;   // 字符的水平位移
};


class TextTexture : protected QOpenGLExtraFunctions{
public:
    explicit TextTexture();

    ~TextTexture();
    void updateWindowSize(int width,int height);

    void setProjectMatrix(glm::mat4 matrix);

    void setModelMatrixMatrix(glm::mat4 matrix);

    void setViewMatrix(glm::mat4 matrix);

    void RenderText(std::u32string text, float x, float y, uint fontsize, QColor color);
private:
    QMap<CharacterID, CharacterData> characters; // 使用 quint32 支持 Unicode 字符

    FT_Library ft;
    FT_Face face;
    unsigned int VAO, VBO;
    QOpenGLShaderProgram m_program;
    bool freeLibLoad = false;

    void init();

    void addCharacter(uint ch,uint fontsize);

};


#endif //VANILLA_TEXTTEXTURE_H
