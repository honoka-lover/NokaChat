#include "modelshowgl.h"

ModelShowGL::ModelShowGL(QWidget* parent)
    : QOpenGLWidget(parent)
{
    m_timer = new QTimer(this);
    QObject::connect(m_timer, &QTimer::timeout, [this]() {
        this->update();

    });
}

void ModelShowGL::StartAnimating()
{
     m_timer->start(10);
}

void ModelShowGL::initializeGL()
{
    initializeOpenGLFunctions();

    m_textTexture = new TextTexture;

    //深度测试
    glEnable(GL_DEPTH_TEST);
    StartAnimating();
}

void ModelShowGL::paintGL()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    /*清空颜色缓存，深度缓存，模板缓存*/
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    m_textTexture->RenderText(QString("aaaaaaaa").toStdU32String(),25.0f, 25.0f, 1.0f,QColor("white"));
}

void ModelShowGL::resizeGL(int width, int height)
{
    /*获取设备像素比*/
    const qreal retinaScale = devicePixelRatio();
    //设置视口
    glViewport(0, 0, width * retinaScale, height * retinaScale);
    m_textTexture->updateWindowSize(width,height);
}
