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
#include "Component/videotool.h"
class VideoWidget : public QOpenGLWidget, protected QOpenGLExtraFunctions {
    Q_OBJECT
signals:
    void fullScreen();
    void exitFullScreen();
    void pauseScreen(bool);
    void sendVolumn(int);
    void updateTime(int64_t);
public:
    VideoWidget(QWidget* parent = nullptr);
    ~VideoWidget();

    void setAllTime(int64_t time);
public slots:
    void setFrame(QSharedPointer<AVFrame> frame);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mouseDoubleClickEvent(QMouseEvent *event) override;

    // void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject* watched,QEvent*event) override;
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

    VideoTool *m_VideoTool;

    bool isPlay = false;

    Qt::WindowFlags windowFlag;

    int videoWidth;
    int videoHeight;

    void initializeShader();
    void initializeTextures();
    void updateTextures();
    void updateVertices(int width, int height, int videoWidth, int videoHeight);
};

#endif // VIDEOWIDGET_H
