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
#include "skybox.h"
#include "global.h"
#include "pbrball.h"
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
    //默认着色器
    QOpenGLShaderProgram m_program;

    QTimer* m_timer;
    float m_oldXPos, m_oldYPos;
    bool m_mousePressed = false;

    GLuint m_uniformBlockIndexEarth;
    GLuint m_uboMatrices;

    //打印openGL的debug报错
    QOpenGLDebugLogger* m_GLlogger;
    QOpenGLContext* context;

    Camera *m_camera;
    Model* m_model;
    TextTexture *m_textTexture;
    SkyBox *m_back;
    PbrBall m_pbrBall;
};

