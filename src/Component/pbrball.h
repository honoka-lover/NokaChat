#ifndef PBRBALL_H
#define PBRBALL_H

#include <QMatrix3x3>
#include "shader.h"
#include "global.h"
#include "vector"
#include "string"
#include "iostream"
class PbrBall:public QOpenGLExtraFunctions
{
public:
    PbrBall();

    bool setTexturePath(QString path);

    void setCubeMap(uint textureID);

    void setLightVec(int lightNums, LightData* data);

    void setFrameBuffer(uint FBO,uint RBO);

    void moveToPos(float x,float y, float z);

    void setUniformValue(const std::string &name, const QMatrix4x4 &matrix);
    void setUniformValue(const std::string &name, const QMatrix3x3 &matrix);
    void setUniformValue(const std::string &name, const glm::mat3 &matrix);
    void setUniformValue(const std::string &name, const glm::mat4 &matrix);
    void setUniformValue(const std::string &name, const QVector3D &matrix);
    void Draw();
private:
    Shader *shader,*irradianceShader,*prefilterShader, *brdfShader;

    unsigned int albedoMap = -1, normalMap = -1, metallicMap = -1, roughnessMap = -1, aoMap = -1;
    GLuint irradianceMap = -1, prefilterMap = -1, brdfLUTTexture = -1, ssbo = -1;

    GLuint envCubeMap = -1;
};

#endif // PBRBALL_H
