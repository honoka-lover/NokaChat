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

float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
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

unsigned int OpenGLWidget::loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    for (unsigned int i = 0; i < faces.size(); i++)
    {
        QImage image(QString::fromStdString(faces[i]));
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
    ////着色器程序位置属性
    //m_posAttr = m_program->attributeLocation("aPos");
    ////断言是否成功
    //Q_ASSERT(m_posAttr != -1);
    ////着色器程序颜色属性
    //m_colAttr = m_program->attributeLocation("aTexCoords");
    ////断言是否成功
    //Q_ASSERT(m_colAttr != -1);

    m_model = new Model("../resources/model/Earth 2K/earth 2K.obj");

    //QString path = QCoreApplication::applicationDirPath();
    //QImage picture(path + "/../../model/textures/Diffuse_2K.png");
    //picture = picture.convertToFormat(QImage::Format_RGBA8888);
    //m_texture = new QOpenGLTexture(picture);
    //m_texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    //m_texture->setMinMagFilters(QOpenGLTexture::Nearest,QOpenGLTexture::Linear);


    m_skyProgram.addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/glsl/sky.vert");
    m_skyProgram.addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/glsl/sky.frag");
    m_skyProgram.link();

    glGenBuffers(1, &m_skyVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_skyVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glGenVertexArrays(1, &m_skyVAO);
    glBindVertexArray(m_skyVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    std::vector<std::string> faces
    {
        "../resources/skybox/right.jpg",
        "../resources/skybox/left.jpg",
        "../resources/skybox/top.jpg",
        "../resources/skybox/bottom.jpg",
        "../resources/skybox/front.jpg",
        "../resources/skybox/back.jpg"
    };
    cubemapTexture = loadCubemap(faces);

    this->m_uniformBlockIndexEarth = glGetUniformBlockIndex(this->m_program.programId(), "Matrices");
    this->m_uniformBlockIndexSky = glGetUniformBlockIndex(this->m_skyProgram.programId(), "Matrices");

    glUniformBlockBinding(m_program.programId(), m_uniformBlockIndexEarth, 0);
    glUniformBlockBinding(m_skyProgram.programId(), m_uniformBlockIndexSky, 0);

    glGenBuffers(1, &m_uboMatrices);
    glBindBuffer(GL_UNIFORM_BUFFER, m_uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * 64, NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_uboMatrices, 0, 2 * 64);

   
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

    m_program.setUniformValue("cameraPos;", m_camera->Position);
    m_model->Draw(&m_program);
    m_program.release();


    glDepthFunc(GL_LEQUAL);
    m_skyProgram.bind();
    view = QMatrix4x4(m_camera->GetViewMatrix3x3());
    //m_skyProgram.setUniformValue("view", view);
    glBindBuffer(GL_UNIFORM_BUFFER, m_uboMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 64, 64, view.data());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    glBindVertexArray(m_skyVAO);
    //m_skyTexture->bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthFunc(GL_LESS);
    glBindVertexArray(0);
    m_skyProgram.release();
}

void OpenGLWidget::resizeGL(int width, int height)
{
    /*获取设备像素比*/
    const qreal retinaScale = devicePixelRatio();
    //设置视口
    glViewport(0, 0, width * retinaScale, height * retinaScale);

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
