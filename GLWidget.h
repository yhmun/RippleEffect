#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QBasicTimer>
#include "RippleEffect.h"

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit GLWidget(QWidget *parent = 0);
    virtual ~GLWidget();

protected:
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();

    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void timerEvent(QTimerEvent *e);

    void initShaders();
    void initTextures();

private:
    QOpenGLShaderProgram program;
    QMatrix4x4 projection;

    QOpenGLTexture *texture;
    RippleEffect *ripple;
    QBasicTimer timer;

signals:

public slots:
};

#endif // GLWIDGET_H
