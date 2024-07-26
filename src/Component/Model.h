//
// Created by honoka on 2024/6/22.
//

#ifndef MYOPENGL_MODEL_H
#define MYOPENGL_MODEL_H

#include <QVector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <qvector4d.h>
#include <qopenglextrafunctions.h>
#include <qopenglshaderprogram.h>
#define MAX_BONE_INFLUENCE 4

using std::string;
using std::vector;
struct Vertex {
    // 位置
    QVector3D Position;
    // 法向量
    QVector3D Normal;
    // texCoords
    QVector2D TexCoords;
    // 切线
    QVector3D Tangent;
    // 副切线
    QVector3D Bitangent;
    //bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    //weights from each bone
    float m_Weights[MAX_BONE_INFLUENCE];
};

struct Texture {
    unsigned int id;
    string type;
    string path;
};

struct Mesh :public QOpenGLExtraFunctions{
    // mesh Data
    vector<Vertex>       vertices;
    vector<unsigned int> indices;
    vector<Texture>      textures;
    unsigned int VAO;

    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures);
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices);

    void Draw(QOpenGLShaderProgram* program);
private:
    // render data 
    unsigned int VBO, EBO;
    void setupMesh();
};



using std::string;
class Model:public QOpenGLExtraFunctions
{
public:
    // constructor, expects a filepath to a 3D model.
    Model(string const& path, bool gamma = false);

    // model data 
    vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    vector<Mesh>    meshes;
    string directory;
    bool gammaCorrection;

    void Draw(QOpenGLShaderProgram *program);

private:
    void loadModel(string const& path);

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode* node, const aiScene* scene);

    Mesh processMesh(aiMesh* mesh, const aiScene* scene);

    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);

    unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);
};


#endif //MYOPENGL_MODEL_H
