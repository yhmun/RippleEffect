#ifndef RIPPLEEFFECT_H
#define RIPPLEEFFECT_H

#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>

class RippleEffect
{
    struct Vector2D
    {
        float x;
        float y;
        Vector2D(float x = 0, float y = 0) : x(x), y(y) { }
    };

    struct Vector3D
    {
        float x;
        float y;
        float z;
        Vector3D(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) { }
    };

    struct Point2D
    {
        int x;
        int y;
        Point2D(int x = 0, int y = 0) : x(x), y(y) { }
    };

    struct RippleData
    {
        int gx;
        int gy;
        int delta;
        int duration;
        int step;
    };

public:

    enum DistortMode
    {
        eDistortVertices,
        eDistortTexCoords
    };

    RippleEffect(QOpenGLShaderProgram *program, float w, float h, QOpenGLTexture *texure = nullptr);
    virtual ~RippleEffect();

    void draw();
    void update();

    void addRipple(float x, float y, int step = 7);

    void setDistortMode(DistortMode mode);

private:

    void initPositions();
    void initTexCoords();
    void initIndices();

    float getDistance(const Vector2D& a, const Vector2D& b);
    int	getMaxDistance(const Vector2D& a, const Vector2D& b);

    QOpenGLShaderProgram *program;
    QOpenGLBuffer positionBuf;
    QOpenGLBuffer texCoordBuf;
    QOpenGLBuffer indexBuf;

    DistortMode distortMode;

    Vector2D imgSize;
    Vector2D texSize;

    std::vector<RippleData> ripples;

    Vector3D* vertices;
    Vector3D* verticesCopy;

    Vector2D* texCoords;
    Vector2D* texCoordsCopy;
};

#endif // RIPPLEEFFECT_H
