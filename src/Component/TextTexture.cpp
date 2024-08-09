//
// Created by honoka on 2024/8/6.
//

#include "TextTexture.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <QMatrix4x4>
#include <iostream>

TextTexture::TextTexture(){
    initializeOpenGLFunctions();

    init();
    updateWindowSize(800,600);

    glm::mat4 matrix(1);
    setModelMatrixMatrix(matrix);
    setViewMatrix(matrix);
    setProjectMatrix(matrix);
}

void TextTexture::init() {
    m_program.addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/glsl/text.vert");
    m_program.addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/glsl/text.frag");
    m_program.link();

    m_program.bind();

    if (FT_Init_FreeType(&ft)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return;
    }

    std::string font_name = "../resources/fonts/Taipei-Sans-TC-Beta-Regular-2.ttf";
    if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return;
    }

    // disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

// configure VAO/VBO for texture quads
// -----------------------------------
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    m_program.release();

    freeLibLoad = true;
}

void TextTexture::RenderText(std::u32string text, float x, float y, uint fontsize, QColor color)
{
    if(!freeLibLoad){
        std::cout<<"freetype库初始化失败";
        return;
    }
//    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // activate corresponding render state
    m_program.bind();
    m_program.setUniformValue("textColor",color.redF(),color.greenF(),color.blueF());
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);
//    glBindTexture(GL_TEXTURE_2D, atlasTexture);
    for (auto c = text.begin(); c != text.end(); c++)
    {
        const CharacterID id = CharacterID{*c,fontsize};
        CharacterData ch;
        bool chExit = characters.contains(id);
        if(!chExit){
            if (FT_Load_Char(face, *c, FT_LOAD_RENDER)) {
                qDebug() << "Failed to load Glyph";
                continue;
            }
            addCharacter(*c,fontsize);
        }

        ch = characters[id];
        float xpos = x + ch.Bearing.x;
        float ypos = y - (ch.Size.y - ch.Bearing.y);

        float w = ch.Size.x;
        float h = ch.Size.y;
        // update VBO for each character
        float vertices[6][4] = {
                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos,     ypos,       0.0f, 1.0f },
                { xpos + w, ypos,       1.0f, 1.0f },

                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos + w, ypos,       1.0f, 1.0f },
                { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6); // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    m_program.release();
}

void TextTexture::addCharacter(uint ch,uint fontsize) {
    FT_Set_Char_Size(face, 0, fontsize * 64, 96, 96);

    if (FT_Load_Char(face, ch, FT_LOAD_RENDER))
    {
        std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
        return;
    }
    // generate texture
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
    );
    // set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // now store character for later use
    CharacterData character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)
    };
    CharacterID id{ch,fontsize};
    characters[id]=character;
}

void TextTexture::updateWindowSize(int width, int height) {
    m_program.bind();
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));
    glUniformMatrix4fv(glGetUniformLocation(m_program.programId(), "projection"), 1, GL_FALSE, &projection[0][0]);
    m_program.release();
}

TextTexture::~TextTexture() {
    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void TextTexture::setProjectMatrix(glm::mat4 matrix) {
    m_program.bind();
    glUniformMatrix4fv(glGetUniformLocation(m_program.programId(), "projection"), 1, GL_FALSE, &matrix[0][0]);
    m_program.release();
}

void TextTexture::setModelMatrixMatrix(glm::mat4 matrix) {
    m_program.bind();
    glUniformMatrix4fv(glGetUniformLocation(m_program.programId(), "model"), 1, GL_FALSE, &matrix[0][0]);
    m_program.release();
}

void TextTexture::setViewMatrix(glm::mat4 matrix) {
    m_program.bind();
    glUniformMatrix4fv(glGetUniformLocation(m_program.programId(), "view"), 1, GL_FALSE, &matrix[0][0]);
    m_program.release();
}
