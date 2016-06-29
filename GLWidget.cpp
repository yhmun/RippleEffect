#include "GLWidget.h"
#include <QMouseEvent>

GLWidget::GLWidget(QWidget *parent) : QOpenGLWidget(parent), ripple(nullptr), speed(7), idxTexture(0)
{
    for (int i = 0; i < 3; i++)
        textures[i] = nullptr;
}

GLWidget::~GLWidget()
{
    // Make sure the context is current when deleting the texture
    // and the buffers.
    makeCurrent();
    for (int i = 0; i < 3; i++)
        delete textures[i];
    delete ripple;
    doneCurrent();
}

void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    initShaders();
    initTextures();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);
    // Enable back face culling
    glEnable(GL_CULL_FACE);

    ripple = new RippleEffect(&program, 512, 512);
    timer.start(12, this);
}

void GLWidget::resizeGL(int w, int h)
{
    projection.setToIdentity();
    projection.ortho(0, w, 0, h, -1, 1000);
}

void GLWidget::paintGL()
{
    // Clear color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    textures[idxTexture]->bind();

    // Calculate model view transformation
    QMatrix4x4 matrix;
    matrix.translate(this->size().width()/2,  this->size().height()/2, 0);

    // Set modelview-projection matrix
    program.setUniformValue("mvp_matrix", projection * matrix);

    // Use texture unit 0 which contains cube.png
    program.setUniformValue("texture", 0);

    // Draw Ripple
    ripple->draw();
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{        
    float x = event->localPos().x() - this->size().width()/2;
    float y = this->size().height()/2 - event->localPos().y();
    ripple->addRipple(x, y, speed);
}

void GLWidget::mouseReleaseEvent(QMouseEvent *)
{

}

void GLWidget::timerEvent(QTimerEvent *)
{
    ripple->update();
    update();
}

void GLWidget::initShaders()
{
    // Compile vertex shader
    if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vshader.glsl"))
        close();

    // Compile fragment shader
    if (!program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fshader.glsl"))
        close();

    // Link shader pipeline
    if (!program.link())
        close();

    // Bind shader pipeline for use
    if (!program.bind())
        close();
}

void GLWidget::initTextures()
{
    const char* files[] = {
        ":/textures/Underwater-Fish-Wallpaper.jpg",
        ":/textures/water_water_0056_01.jpg",
        ":/textures/stones-770264.jpg"
    };

    // Load image
    for (int i = 0; i < 3; i++)
    {
        textures[i] = new QOpenGLTexture(QImage(files[i]).mirrored());

        // Set nearest filtering mode for texture minification
        textures[i]->setMinificationFilter(QOpenGLTexture::Nearest);

        // Set bilinear filtering mode for texture magnification
        textures[i]->setMagnificationFilter(QOpenGLTexture::Linear);

        // Wrap texture coordinates by repeating
        // f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
        textures[i]->setWrapMode(QOpenGLTexture::Repeat);
    }
}

void GLWidget::setSpeed(int value)
{
    speed = value;
}

void GLWidget::setTexture(int value)
{
    idxTexture = value;
}

void GLWidget::setDistort(int value)
{
    ripple->setDistortMode(value == 0 ? RippleEffect::eDistortVertices : RippleEffect::eDistortTexCoords);
}
