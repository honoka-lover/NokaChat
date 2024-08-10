#include "pbrball.h"
#include <filesystem>
#include <QDir>
#include <QMap>
#include <QDebug>

glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
glm::mat4 captureViews[] =
        {
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
        };


PbrBall::PbrBall() {
    initializeOpenGLFunctions();

    shader = new Shader(":/glsl/pbr.vert", ":/glsl/pbr.frag");
    irradianceShader = new Shader(":/glsl/cubemap.vert", ":/glsl/irradiance_convolution.frag");
    prefilterShader = new Shader(":/glsl/cubemap.vert", ":/glsl/prefilter.frag");
    brdfShader = new Shader(":/glsl/brdf.vert", ":/glsl/brdf.frag");
    shader->use();
    shader->setUniformValue("irradianceMap", 0);
    shader->setUniformValue("prefilterMap", 1);
    shader->setUniformValue("brdfLUT", 2);
    shader->setUniformValue("albedoMap", 3);
    shader->setUniformValue("normalMap", 4);
    shader->setUniformValue("metallicMap", 5);
    shader->setUniformValue("roughnessMap", 6);
    shader->setUniformValue("aoMap", 7);

}

bool PbrBall::setTexturePath(QString path)
{
    QMap<QString,QString> picMap;
    QDir dir(path);
    if (!dir.exists()) {
        qDebug() << "Directory does not exist.";
        return false;
    }

    QFileInfoList files = dir.entryInfoList(QDir::Files);
    for (const QFileInfo &fileInfo : files) {
        picMap.insert(fileInfo.fileName(),fileInfo.filePath());
    }

    for(const QString &str : picMap.keys()){
        if(str.contains("albedo")){
            albedoMap = ModelFunction::getInstance()->loadTexture(picMap.value(str).toStdString().c_str());
        }else if(str.contains("normal") || str.contains("Normal")){
            normalMap = ModelFunction::getInstance()->loadTexture(picMap.value(str).toStdString().c_str());
        }else if(str.contains("rough")){
            roughnessMap = ModelFunction::getInstance()->loadTexture(picMap.value(str).toStdString().c_str());
        }else if(str.contains("metallic")){
            metallicMap = ModelFunction::getInstance()->loadTexture(picMap.value(str).toStdString().c_str());
        }else if(str.contains("ao")){
            aoMap = ModelFunction::getInstance()->loadTexture(picMap.value(str).toStdString().c_str());
        }else{
            continue;
        }
        picMap.remove(str);
    }

    if(albedoMap == -1 || normalMap == -1 || roughnessMap == -1 || metallicMap == -1 || aoMap == -1){
        qDebug() << "纹理图片不完整.";
        return false;
    }
    return true;
}

void PbrBall::setUniformValue(const std::string &name, const QMatrix4x4 &matrix) {
    shader->use();
    shader->setUniformValue(name,matrix);
}

void PbrBall::setUniformValue(const std::string &name, const QMatrix3x3 &matrix) {
    shader->use();
    shader->setUniformValue(name,matrix);
}

void PbrBall::Draw() {
    shader->use();

    // bind pre-computed IBL data
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

// rusted iron
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, albedoMap);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, normalMap);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, metallicMap);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, roughnessMap);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, aoMap);

    ModelFunction::getInstance()->renderSphere();
}

void PbrBall::setUniformValue(const std::string &name, const glm::mat3 &matrix) {
    shader->use();
    shader->setUniformValue(name,matrix);
}

void PbrBall::setUniformValue(const std::string &name, const glm::mat4 &matrix) {
    shader->use();
    shader->setUniformValue(name,matrix);
}

void PbrBall::setUniformValue(const std::string &name, const QVector3D &matrix) {
    shader->use();
    shader->setUniformValue(name,matrix);
}

void PbrBall::setCubeMap(uint textureID) {
    envCubeMap = textureID;
}

void PbrBall::setFrameBuffer(uint captureFBO, uint captureRBO) {
    glGenTextures(1, &irradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    irradianceShader->use();
    irradianceShader->setUniformValue("environmentMap", 0);
    irradianceShader->setUniformValue("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMap);

    glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (unsigned int i = 0; i < 6; ++i)
    {
        irradianceShader->setUniformValue("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ModelFunction::getInstance()->renderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // pbr: create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale.
    // --------------------------------------------------------------------------------
    glGenTextures(1, &prefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // be sure to set minification filter to mip_linear
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // pbr: run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
    // ----------------------------------------------------------------------------------------------------
    prefilterShader->use();
    prefilterShader->setUniformValue("environmentMap", 0);
    prefilterShader->setUniformValue("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMap);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    unsigned int maxMipLevels = 5;
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
    {
        // reisze framebuffer according to mip-level size.
        unsigned int mipWidth = static_cast<unsigned int>(128 * std::pow(0.5, mip));
        unsigned int mipHeight = static_cast<unsigned int>(128 * std::pow(0.5, mip));
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        prefilterShader->setUniformValue("roughness", roughness);
        for (unsigned int i = 0; i < 6; ++i)
        {
            prefilterShader->setUniformValue("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            ModelFunction::getInstance()->renderCube();
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // pbr: generate a 2D LUT from the BRDF equations used.
    // ----------------------------------------------------
    glGenTextures(1, &brdfLUTTexture);

    // pre-allocate enough memory for the LUT texture.
    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
    // be sure to set wrapping mode to GL_CLAMP_TO_EDGE
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

    glViewport(0, 0, 512, 512);
    brdfShader->use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ModelFunction::getInstance()->renderQuad();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PbrBall::setLightVec(int lightNums, LightData* data) {
    shader->use();
    shader->setUniformValue("lightVecNums", lightNums);
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    int bufferSize = 32 * lightNums;
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, data, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo); //与着色器中binding值绑定，这里设置是0

}

void PbrBall::moveToPos(float x, float y, float z) {
    shader->use();
    auto pbrModel = glm::mat4(1.0f);
    pbrModel = glm::translate(pbrModel, glm::vec3(x, y, z));
    shader->setUniformValue("model", pbrModel);
    shader->setUniformValue("normalMatrix", glm::transpose(glm::inverse(glm::mat3(pbrModel))));
}
