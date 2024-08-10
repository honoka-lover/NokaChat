#include "OpenGLWidget.h"
#include <qtimer.h>
#include <qopenglshaderprogram.h>
#include <qmatrix4x4.h>
#include <qpixmap.h>
#include <qmath.h>
#include <qopengltexture.h>
#include <qvector3d.h>
#include <qevent.h>
#include <QFile>
#include <QOpenGLShader>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qbuffer.h>
#include <iostream>

// lights
// ------
static LightData lightData[] = {
        {
                glm::vec4(-10.0f, 10.0f, 10.0f ,1.0f),
                glm::vec4(300.0f, 300.0f, 300.0f ,1.0f)},
        {
                glm::vec4(10.0f, 10.0f, 10.0f, 1.0f),
                glm::vec4(300.0f, 300.0f, 300.0f, 1.0f)},
        {
                glm::vec4(-10.0f, -10.0f, 10.0f, 1.0f),
                glm::vec4(300.0f, 300.0f, 300.0f, 1.0f),
        },
        {
                glm::vec4(10.0f, -10.0f, 10.0f, 1.0f),
                glm::vec4(300.0f, 300.0f, 300.0f, 1.0f)
        }
};

OpenGLWidget::OpenGLWidget(QWidget* parent)
        : QOpenGLWidget(parent),
          m_camera(new Camera(QVector3D(0.0f, 0.0f, 15.0f)))
{
    m_oldXPos = 0.0;
    m_oldYPos = 0.0;

    m_timer = new QTimer(this);
    QObject::connect(m_timer, &QTimer::timeout, [this]() {
        this->update();

    });

    installEventFilter(this);
    //捕获键盘输入
    this->grabKeyboard();

}

OpenGLWidget::~OpenGLWidget()
{
    m_timer->deleteLater();
}

void OpenGLWidget::StartAnimating()
{
    m_timer->start(10);
}

void OpenGLWidget::initializeGL()
{
    // 为当前环境初始化OpenGL函数
    initializeOpenGLFunctions();

    // 获取当前上下文
    context = QOpenGLContext::currentContext();
    if (!context) {
        qWarning() << "No current OpenGL context.";
    }
    // 确保上下文是调试上下文
    if (!context->format().testOption(QSurfaceFormat::DebugContext)) {
        qWarning() << "Current OpenGL context is not a debug context.";
    }
    qDebug()<<"suggest GL_KHR_debug:"<<context->hasExtension(QByteArrayLiteral("GL_KHR_debug"));
    m_GLlogger = new QOpenGLDebugLogger(this);
    connect(m_GLlogger, &QOpenGLDebugLogger::messageLogged, this, [](const QOpenGLDebugMessage& message) {
        qDebug() << "OpenGL debug message:" << message.message();
    });
    if (m_GLlogger->initialize()) {
        m_GLlogger->startLogging(QOpenGLDebugLogger::SynchronousLogging);
    }
    else {
        qWarning() << "Failed to initialize QOpenGLDebugLogger";
    }

    //加载顶点着色器程序
    // m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vCode.data());
    m_program.addCacheableShaderFromSourceFile(QOpenGLShader::Vertex,":/glsl/earth.vert");
    //加载片段着色器程序
    // m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fCode.data());
    m_program.addCacheableShaderFromSourceFile(QOpenGLShader::Fragment,":/glsl/earth.frag");
    //链接着色器程序
    m_program.link();

    m_model = new Model("../resources/model/Earth 2K/earth 2K.obj");
    // m_model = new Model("../resources/model/hutao.fbx");

    QStringList faces
            {
                    "../resources/skybox/right.jpg",
                    "../resources/skybox/left.jpg",
                    "../resources/skybox/top.jpg",
                    "../resources/skybox/bottom.jpg",
                    "../resources/skybox/front.jpg",
                    "../resources/skybox/back.jpg"
            };
    QStringList hdrPic{
            "../resources/hdr/chapel_day_8k.hdr"
    };
    m_back = new SkyBox;
//    m_back->setSource(faces,SkyBox::SkyBoxPic_6);
     m_back->setSource(hdrPic,SkyBox::HDR_MODEL);

    m_pbrBall.setTexturePath("../resources/pbr/gold");
    m_pbrBall.setCubeMap(m_back->getCubeMapID());
    m_pbrBall.setFrameBuffer(m_back->getCaptureFBO(),m_back->getCaptureRBO());
    m_pbrBall.setLightVec(4,lightData);
    m_pbrBall.moveToPos(1.0f,0.0f,2.0f);

//    light1.setTexturePath("../resources/pbr/gold");
//    light1.setCubeMap(m_back->getCubeMapID());
//    light1.setFrameBuffer(m_back->getCaptureFBO(),m_back->getCaptureRBO());
//    light1.setLightVec(4,lightData);
//    light1.moveToPos(lightData[0].position.x,lightData[0].position.y,lightData[0].position.z);

    this->m_uniformBlockIndexEarth = glGetUniformBlockIndex(this->m_program.programId(), "Matrices");
    glUniformBlockBinding(m_program.programId(), m_uniformBlockIndexEarth, 0);

    glGenBuffers(1, &m_uboMatrices);
    glBindBuffer(GL_UNIFORM_BUFFER, m_uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * 64, NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_uboMatrices, 0, 2 * 64);

    m_textTexture = new TextTexture;

    //深度测试
    glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_ALWAYS);
}

void OpenGLWidget::paintGL()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    /*清空颜色缓存，深度缓存，模板缓存*/
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    //绑定着色器程序
    m_program.bind();

    //纹理
    //m_texture->bind();

    QMatrix4x4 projection;
    //设置透视矩阵
    projection.perspective(m_camera->Zoom, float(width()) / float(height()), 0.1f, 100.0f);
    //沿着Z轴负方向平移-2
    //projection.translate(0, 0, -2);
    //绕Y轴旋转
    //projection.rotate(m_frame, 0, 1, 0);
    //为着色器程序传入当前变换
    //m_program.setUniformValue("projection", projection);
    glBindBuffer(GL_UNIFORM_BUFFER, m_uboMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, 64, projection.data());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    //观察矩阵
    QMatrix4x4 view = m_camera->GetViewMatrix();
    //m_program.setUniformValue("view", view);
    glBindBuffer(GL_UNIFORM_BUFFER, m_uboMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 64, 64, view.data());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);


    QMatrix4x4 model;
    model.translate(0,0,0);
    model.scale(1,1,1);
    m_program.setUniformValue("model", model);

    m_program.setUniformValue("cameraPos", m_camera->Position);
//    m_model->Draw(&m_program);
    m_program.release();

    m_back->setViewMatrix(QMatrix4x4(m_camera->GetViewMatrix3x3()),m_uboMatrices);


    m_pbrBall.setUniformValue("camPos", m_camera->Position);
    m_pbrBall.setUniformValue("projection", projection);
    m_pbrBall.setUniformValue("view", view);
    m_pbrBall.Draw();


    //背景盒
    m_back->Draw();
    //文字
    m_textTexture->RenderText(QString("你好").toStdU32String(),0,10,40,QColor("#ff0000"));
}

void OpenGLWidget::resizeGL(int width, int height)
{
    /*获取设备像素比*/
    const qreal retinaScale = devicePixelRatio();
    //设置视口
    glViewport(0, 0, width * retinaScale, height * retinaScale);
    m_textTexture->updateWindowSize(width,height);
}

bool OpenGLWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == this && event->type() == QMouseEvent::MouseMove) {
        if (m_mousePressed) {
            QMouseEvent* e = (QMouseEvent*)event;
            QPointF pos = e->globalPosition();//相对于整个界面的全局坐标
            //QPoint pos = e->pos();//相对于传入对象obj的相对坐标
            float xpos = pos.x();
            float ypos = pos.y();
            float xoffset = xpos - m_oldXPos;
            float yoffset = m_oldYPos - ypos; // reversed since y-coordinates go from bottom to top
            m_oldXPos = xpos;
            m_oldYPos = ypos;

            m_camera->ProcessMouseMovement(xoffset, yoffset);
            //qDebug() << "鼠标移动";
        }
    }
    else if (watched == this && event->type() == QMouseEvent::MouseButtonPress) {
        QMouseEvent* e = (QMouseEvent*)event;
        QPointF pos = e->globalPosition();//相对于整个界面的全局坐标
        //QPoint pos = e->pos();//相对于传入对象obj的相对坐标
        m_mousePressed = true;
        m_oldXPos = pos.x();
        m_oldYPos = pos.y();

        //qDebug() << "鼠标按下";
    }
    else if (watched == this && event->type() == QMouseEvent::MouseButtonRelease) {
        m_mousePressed = false;
    }
    else if (watched == this && event->type() == QEvent::Wheel) {
        QWheelEvent* e = (QWheelEvent*)event;
        //qDebug() << e->angleDelta().y();
        m_camera->ProcessMouseScroll(e->angleDelta().y()/120);
    }
    else if (watched == this && event->type() == QEvent::KeyPress) {
        QKeyEvent* e = (QKeyEvent*)event;
        //qDebug() << "键盘按下";
        switch(e->key()) {
            case Qt::Key_W:
                m_camera->ProcessKeyboard(FORWARD, 1);
                //qDebug() << "W按下";
                break;
            case Qt::Key_A:
                m_camera->ProcessKeyboard(LEFT, 1);
                break;
            case Qt::Key_S:
                m_camera->ProcessKeyboard(BACKWARD, 1);
                break;
            case Qt::Key_D:
                m_camera->ProcessKeyboard(RIGHT, 1);
                break;
        }
    }

    return false;
}
