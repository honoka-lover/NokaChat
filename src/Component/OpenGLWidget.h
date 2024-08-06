#pragma once
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <qopengltexture.h>
#include <qvector3d.h>
#include "Camera.h"
#include "model.h"
#include "QOpenGLBuffer"
#include <QOpenGLDebugLogger>
#include "TextTexture.h"
class QOpenGLShaderProgram;
class OpenGLWidget : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
    Q_OBJECT
public:
    explicit OpenGLWidget(QWidget* parent = 0);
    ~OpenGLWidget();

public:
    void StartAnimating();
protected:
    /*初始化OpenGL资源和状态*/
    void initializeGL();
    /*渲染OpenGL场景*/
    void paintGL();
    /*设置OpenGL的视口、投影等。每次部件大小改变时都会调用该函数*/
    void resizeGL(int width, int height);

    bool eventFilter(QObject* watched, QEvent* event);

private:
    unsigned int loadCubemap(vector<std::string> faces);

private:
    //GLint m_posAttr = 0;
    //GLint m_colAttr = 0;

    //默认着色器
    QOpenGLShaderProgram m_program;

    //背景着色器
    QOpenGLShaderProgram m_skyProgram;

    int m_frame = 0;

    QTimer* m_timer;

    bool m_mousePressed = false;

    Camera *m_camera;
    Model* m_model;

    //QOpenGLTexture *m_texture = nullptr;
    QOpenGLTexture* m_skyTexture = nullptr;
    QOpenGLBuffer m_vbo;

    QVector<float> m_vertPoints,m_texturePoints,m_normalPoints;

    float m_oldXPos, m_oldYPos;

    GLuint m_skyVBO, m_skyVAO;

    unsigned int cubemapTexture;

    GLuint m_uniformBlockIndexEarth, m_uniformBlockIndexSky;
    GLuint m_uboMatrices;

    TextTexture *m_textTexture;

    //打印openGL的debug报错
    QOpenGLDebugLogger* m_GLlogger;
    QOpenGLContext* context;
};

