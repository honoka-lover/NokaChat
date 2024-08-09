//
// Created by honoka on 2024/8/7.
//

#ifndef VANILLA_SKYBOX_H
#define VANILLA_SKYBOX_H

#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <QMatrix4x4>
#include "shader.h"
#include "global.h"
class SkyBox :public QOpenGLExtraFunctions{
public:
    enum BackType{
        HDR_MODEL,
        SkyBoxPic_6,
        SkyBoxPic_1
    };

    SkyBox();

    void setSource(QStringList path,BackType type);

    void Draw();

    void setViewMatrix(QMatrix4x4 view, GLuint m_uboMatrices);

    void setProjectionMatrix(QMatrix4x4 projection, GLuint m_uboMatrices);

    int getCubeMapID();

    int getIrradianceMapID();

    uint getCaptureFBO(){
        return captureFBO;
    }

    uint getCaptureRBO(){
        return captureRBO;
    }
private:
    Shader *m_skyShader,*m_cubeMapShader,*m_backgroundShader;

    QStringList picPath;

    BackType type;

    GLuint captureFBO, captureRBO, envCubeMap, m_uniformMatrices;

    unsigned int cubeMapTexture, hdrTexture;

    unsigned int loadCubeMap(QStringList faces);

    int initialized = false;
};


#endif //VANILLA_SKYBOX_H
