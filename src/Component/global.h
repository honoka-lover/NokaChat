#ifndef GLOBAL_H
#define GLOBAL_H

#include<qopenglextrafunctions.h>

#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class ModelFunction:public QOpenGLExtraFunctions
{
public:
    static ModelFunction* getInstance();

    void renderSphere();

    void renderCube();

    void renderQuad();

    // utility function for loading a 2D texture from file
    // ---------------------------------------------------
    unsigned int loadTexture(char const * path);

private:
    ModelFunction();

    ModelFunction(const ModelFunction&) = delete;            //明确拒绝
    ModelFunction& operator=(const ModelFunction&) = delete; //明确拒绝

    static ModelFunction* m_pSingleton;
};


#endif // GLOBAL_H
