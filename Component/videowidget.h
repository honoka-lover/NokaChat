#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

#include <QApplication>
#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLFunctions>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QOpenGLExtraFunctions>
class VideoWidget : public QOpenGLWidget, protected QOpenGLExtraFunctions {
    Q_OBJECT

public:
    VideoWidget(QWidget* parent = nullptr);
    ~VideoWidget();


public slots:
    void setFrame(QSharedPointer<AVFrame> frame);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    QOpenGLShaderProgram* program;
    QOpenGLTexture* textureY;
    QOpenGLTexture* textureU;
    QOpenGLTexture* textureV;
    GLuint textureUniformY;
    GLuint textureUniformU;
    GLuint textureUniformV;
    AVFrame* currentFrame;
    QMutex frameMutex;

    GLuint vao;
    GLuint vbo;
    GLuint ebo;

    AVFormatContext *m_avFormatCxt;

    int videoWidth;
    int videoHeight;

    void initializeShader();
    void initializeTextures();
    void updateTextures();
    void updateVertices(int width, int height, int videoWidth, int videoHeight);
};

#endif // VIDEOWIDGET_H
