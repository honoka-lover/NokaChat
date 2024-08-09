//
// Created by honoka on 2024/8/7.
//

#include "skybox.h"
#include <QImage>
#include "stb_image.h"
#include <iostream>

// pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
// ----------------------------------------------------------------------------------------------
static glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
static glm::mat4 captureViews[] =
        {
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
        };


SkyBox::SkyBox():
    type(HDR_MODEL),
    picPath()
{
    initializeOpenGLFunctions();

    m_skyShader = new Shader(":/glsl/sky.vert",":/glsl/sky.frag");

    m_cubeMapShader = new Shader(":/glsl/cubemap.vert",":/glsl/equirectangular_to_cubemap.frag");

    m_backgroundShader = new Shader(":/glsl/hdrBackground.vert",":/glsl/hdrBackground.frag");

    m_uniformMatrices = glGetUniformBlockIndex(m_skyShader->ID, "Matrices");
    glUniformBlockBinding(m_skyShader->ID, m_uniformMatrices, 0);
    glUniformBlockBinding(m_backgroundShader->ID, m_uniformMatrices, 0);
}

void SkyBox::setSource(QStringList path, BackType type) {
    picPath = path;
    this->type = type;

    if(picPath.empty())
        return;
    if(initialized)
        return;
    initialized = true;
    if(this->type == HDR_MODEL){
        // configure global opengl state
        // -----------------------------
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL); // set depth function to less than AND equal for skybox depth trick.

        m_backgroundShader->use();
        m_backgroundShader->setUniformValue("environmentMap", 0);

        // pbr: setup framebuffer
        // ----------------------
        glGenFramebuffers(1, &captureFBO);
        glGenRenderbuffers(1, &captureRBO);

        // pbr: load the HDR environment map
        // ---------------------------------
        stbi_set_flip_vertically_on_load(true);
        int width, height, nrComponents;
        float *data = stbi_loadf(picPath[0].toStdString().data(), &width, &height, &nrComponents, 0);
        if (data)
        {
            glGenTextures(1, &hdrTexture);
            glBindTexture(GL_TEXTURE_2D, hdrTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); // note how we specify the texture's data value to be float

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else
        {
            std::cout << "Failed to load HDR image." << std::endl;
        }

        int picSize = width > height ? height: width;
        picSize = 2048;

        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, picSize, picSize);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);



        // pbr: setup cubemap to render to and attach to framebuffer
        // ---------------------------------------------------------
        glGenTextures(1, &envCubeMap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMap);
        for (unsigned int i = 0; i < 6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, picSize, picSize, 0, GL_RGB, GL_FLOAT, nullptr);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


        // pbr: convert HDR equirectangular environment map to cubemap equivalent
        // ----------------------------------------------------------------------
        m_cubeMapShader->use();
        m_cubeMapShader->setUniformValue("equirectangularMap", 0);
        m_cubeMapShader->setUniformValue("projection", captureProjection);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, hdrTexture);

        glViewport(0, 0, picSize, picSize); // don't forget to configure the viewport to the capture dimensions.
        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        for (unsigned int i = 0; i < 6; ++i)
        {
            m_cubeMapShader->setUniformValue("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubeMap, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            ModelFunction::getInstance()->renderCube();
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }else if(this->type == SkyBoxPic_6){


        envCubeMap = loadCubeMap(path);
        ModelFunction::getInstance()->renderCube();
    }


}

unsigned int SkyBox::loadCubeMap(QStringList faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    for (unsigned int i = 0; i < faces.size(); i++)
    {
        QImage image(faces[i]);
        image = image.convertToFormat(QImage::Format_RGBA8888);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void SkyBox::Draw() {
    if(picPath.empty())
        return;
    if(type == SkyBoxPic_6){
        glDepthFunc(GL_LEQUAL);
        m_skyShader->use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMap);
        ModelFunction::getInstance()->renderCube();
    }else if(type == HDR_MODEL){
        m_backgroundShader->use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMap);
        ModelFunction::getInstance()->renderCube();
    }
}

void SkyBox::setViewMatrix(QMatrix4x4 view, GLuint m_uboMatrices) {
    glBindBuffer(GL_UNIFORM_BUFFER, m_uboMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 64, 64, view.data());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void SkyBox::setProjectionMatrix(QMatrix4x4 projection, GLuint m_uboMatrices) {
    glBindBuffer(GL_UNIFORM_BUFFER, m_uboMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, 64, projection.data());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

int SkyBox::getCubeMapID() {
    if(initialized)
        return envCubeMap;
    return -1;
}

int SkyBox::getIrradianceMapID() {
//    glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
//    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
//    for (unsigned int i = 0; i < 6; ++i)
//    {
//        irradianceShader.setMat4("view", captureViews[i]);
//        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//        renderCube();
//    }
//    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return 0;
}
