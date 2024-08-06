#ifndef MODELSHOWGL_H
#define MODELSHOWGL_H
#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include "TextTexture.h"
#include <QTimer>
class ModelShowGL : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
    Q_OBJECT
public:
    ModelShowGL(QWidget* parent = 0);

public:
    void StartAnimating();
protected:
    /*初始化OpenGL资源和状态*/
    void initializeGL();
    /*渲染OpenGL场景*/
    void paintGL();
    /*设置OpenGL的视口、投影等。每次部件大小改变时都会调用该函数*/
    void resizeGL(int width, int height);

private:
    TextTexture *m_textTexture;
    QTimer* m_timer;
};

#endif // MODELSHOWGL_H
