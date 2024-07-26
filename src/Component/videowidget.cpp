#include "Component/videowidget.h"
#include <QMatrix4x4>
#include <QDebug>
#include <QKeyEvent>
#include <QEvent>
#include <QMouseEvent>

VideoWidget::VideoWidget(QWidget* parent):
    QOpenGLWidget(parent),
    program(nullptr),
    textureY(nullptr),
    textureU(nullptr),
    textureV(nullptr),
    currentFrame(nullptr),
    m_VideoTool(nullptr)
{
    m_avFormatCxt = nullptr;
    videoWidth=0;
    videoHeight = 0;

    installEventFilter(this);
    //捕获键盘输入
    this->grabKeyboard();
    this->setMouseTracking(true);
    m_VideoTool = new VideoTool(this);
}

VideoWidget::~VideoWidget() {
    if (program) delete program;
    if (textureY) delete textureY;
    if (textureU) delete textureU;
    if (textureV) delete textureV;
    if (currentFrame) av_frame_free(&currentFrame);

}

void VideoWidget::setAllTime(int64_t time)
{
    if(m_VideoTool){
        m_VideoTool->setAllTime(time);
    }
}

void VideoWidget::initializeGL() {
    initializeOpenGLFunctions();

    // 初始化着色器
    initializeShader();

    // 初始化纹理
    initializeTextures();

    // 初始化顶点缓冲区对象（VBO）和顶点数组对象（VAO）
    GLfloat vertices[] = {
        // 顶点坐标   // 纹理坐标
        -1.0f, -1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 1.0f,
        1.0f,  1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 0.0f
    };
    GLuint indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void VideoWidget::initializeShader() {
    program = new QOpenGLShaderProgram();
    program->addShaderFromSourceCode(QOpenGLShader::Vertex,
                                     R"(
        #version 330 core
        layout (location = 0) in vec2 vertex;
        layout (location = 1) in vec2 texCoord;
        out vec2 TexCoord;
        void main() {
            gl_Position = vec4(vertex, 0.0, 1.0);
            TexCoord = texCoord;
        }
        )");
    program->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                     R"(
        #version 330 core
        in vec2 TexCoord;
        out vec4 FragColor;
        uniform sampler2D textureY;
        uniform sampler2D textureU;
        uniform sampler2D textureV;
        void main() {
            vec3 yuv,rgb;
            yuv.x = texture(textureY, TexCoord).r;
            yuv.y = texture(textureU, TexCoord).r - 0.5;
            yuv.z = texture(textureV, TexCoord).r - 0.5;
            rgb = mat3(1,       1,      1,
                       0, -.34414, 1.772,
                       1.402, -.71414,      0) * yuv;
            FragColor = vec4(rgb, 1.0);
        }
        )");
    program->link();
    program->bind();

    textureUniformY = program->uniformLocation("textureY");
    textureUniformU = program->uniformLocation("textureU");
    textureUniformV = program->uniformLocation("textureV");
}

void VideoWidget::initializeTextures() {
    textureY = new QOpenGLTexture(QOpenGLTexture::Target2D);
    textureY->create();
    textureY->setMinificationFilter(QOpenGLTexture::Linear);
    textureY->setMagnificationFilter(QOpenGLTexture::Linear);
    textureY->setWrapMode(QOpenGLTexture::ClampToEdge);

    textureU = new QOpenGLTexture(QOpenGLTexture::Target2D);
    textureU->create();
    textureU->setMinificationFilter(QOpenGLTexture::Linear);
    textureU->setMagnificationFilter(QOpenGLTexture::Linear);
    textureU->setWrapMode(QOpenGLTexture::ClampToEdge);

    textureV = new QOpenGLTexture(QOpenGLTexture::Target2D);
    textureV->create();
    textureV->setMinificationFilter(QOpenGLTexture::Linear);
    textureV->setMagnificationFilter(QOpenGLTexture::Linear);
    textureV->setWrapMode(QOpenGLTexture::ClampToEdge);
}

void VideoWidget::updateTextures() {
    if (currentFrame) {
        int width = currentFrame->width;
        int height = currentFrame->height;

        glActiveTexture(GL_TEXTURE0);
        textureY->bind();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width,height,0,GL_RED, GL_UNSIGNED_BYTE,currentFrame->data[0]);
        // textureY->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, QSize(width, height), currentFrame->data[0]);
        program->setUniformValue(textureUniformY, 0);

        glActiveTexture(GL_TEXTURE1);
        textureU->bind();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width/2, height/2, 0,GL_RED, GL_UNSIGNED_BYTE,currentFrame->data[1]);
        // textureU->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, QSize(width / 2, height / 2), currentFrame->data[1]);
        program->setUniformValue(textureUniformU, 1);

        glActiveTexture(GL_TEXTURE2);
        textureV->bind();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width/2,height/2,0,GL_RED, GL_UNSIGNED_BYTE,currentFrame->data[2]);
        // textureV->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, QSize(width / 2, height / 2), currentFrame->data[2]);
        program->setUniformValue(textureUniformV, 2);
    }
}

void VideoWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
    if(m_VideoTool){
        m_VideoTool->setGeometry(0,h-70,w,70);
    }
    if(videoHeight == 0&& videoHeight == 0){

        return;
    }
    updateVertices(w, h, videoWidth, videoHeight);   
}

void VideoWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT);

    if (currentFrame) {
        updateTextures();

        program->bind();
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        program->release();
    }
}

void VideoWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    qDebug()<<"全屏";
    emit fullScreen();
}

bool VideoWidget::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == this && event->type() == QEvent::KeyPress){
        QKeyEvent* e = (QKeyEvent*)event;
        switch(e->key()){
        case Qt::Key_Space:
            qDebug()<<"暂停";
            break;
        }
    }
    return false;
}

// void VideoWidget::resizeEvent(QResizeEvent *event)
// {
//     if(m_VideoTool){
//         m_VideoTool->setGeometry(0,this->height()-65,this->width(),60);
//     }
// }

void VideoWidget::setFrame(QSharedPointer<AVFrame> frame) {
    QMutexLocker locker(&frameMutex);
    if (currentFrame) {
        av_frame_free(&currentFrame);
    }
    currentFrame = av_frame_clone(frame.data());

    if(videoWidth != frame->width || videoHeight != frame->height) {
        videoWidth = frame->width;
        videoHeight = frame->height;
        updateVertices(width(), height(), videoWidth, videoHeight);
    }
    update();
}

void VideoWidget::VideoWidget::updateVertices(int width, int height,
                                              int videoWidth, int videoHeight) {
    float videoAspect = static_cast<float>(videoWidth) / videoHeight;
    float widgetAspect = static_cast<float>(width) / height;

    float scaleW = 1.0f;
    float scaleH = 1.0f;

    if (widgetAspect > videoAspect) {
        scaleW = videoAspect / widgetAspect;
    } else {
        scaleH = widgetAspect / videoAspect;
    }

    GLfloat vertices[] = {// 顶点坐标   // 纹理坐标
                          -scaleW, -scaleH, 0.0f,   1.0f,   scaleW, -scaleH,
                          1.0f,    1.0f,    scaleW, scaleH, 1.0f,   0.0f,
                          -scaleW, scaleH,  0.0f,   0.0f};
    GLuint indices[] = {0, 1, 2, 2, 3, 0};

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
                          (GLvoid *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
                          (GLvoid *)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}
