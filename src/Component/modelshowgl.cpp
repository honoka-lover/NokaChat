#include "modelshowgl.h"

ModelShowGL::ModelShowGL(QWidget* parent)
    : QOpenGLWidget(parent),
      m_camera(new Camera(QVector3D(0.0f, 0.0f, 15.0f)))
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

    m_pbrBall = new PbrBall();
    m_pbrBall->setTexturePath("../resources/pbr/grass");
    //深度测试
    glEnable(GL_DEPTH_TEST);
    StartAnimating();
}

void ModelShowGL::paintGL()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    /*清空颜色缓存，深度缓存，模板缓存*/
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    QMatrix4x4 projection;
    //设置透视矩阵
    projection.perspective(m_camera->Zoom, float(width()) / float(height()), 0.1f, 100.0f);
    //沿着Z轴负方向平移-2
    //projection.translate(0, 0, -2);
    //绕Y轴旋转
    //projection.rotate(m_frame, 0, 1, 0);

    //观察矩阵
    QMatrix4x4 view = m_camera->GetViewMatrix();


    auto pbrModel = glm::mat4(1.0f);
    pbrModel = glm::translate(pbrModel, glm::vec3(1.0, 0.0, 2.0));
    m_pbrBall->setUniformValue("model", pbrModel);
    m_pbrBall->setUniformValue("normalMatrix", glm::transpose(glm::inverse(glm::mat3(pbrModel))));
    m_pbrBall->setUniformValue("camPos", m_camera->Position);
    m_pbrBall->setUniformValue("projection", projection);
    m_pbrBall->setUniformValue("view", view);
    m_pbrBall->Draw();

    m_textTexture->RenderText(QString("你好").toStdU32String(),0,10,40,QColor("#ffffff"));
}

void ModelShowGL::resizeGL(int width, int height)
{
    /*获取设备像素比*/
    const qreal retinaScale = devicePixelRatio();
    //设置视口
    glViewport(0, 0, width * retinaScale, height * retinaScale);
    m_textTexture->updateWindowSize(width,height);
}
