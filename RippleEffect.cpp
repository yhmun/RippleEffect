#include "RippleEffect.h"
#include "RippleTable.h"
#include <cmath>

RippleEffect::RippleEffect(QOpenGLShaderProgram *program, float w, float h, QOpenGLTexture *)
    : program(program), indexBuf(QOpenGLBuffer::IndexBuffer), distortMode(eDistortTexCoords), imgSize(w, h)
{
    // Generate VBOs
    positionBuf.create();
    texCoordBuf.create();
    indexBuf.create();

    initPositions();
    initTexCoords();
    initIndices();    
}

RippleEffect::~RippleEffect()
{
    delete[] vertices;
    delete[] verticesCopy;

    delete[] texCoords;
    delete[] texCoordsCopy;

    positionBuf.destroy();
    texCoordBuf.destroy();
    indexBuf.destroy();
}

void RippleEffect::initPositions()
{
    vertices = new Vector3D[(GRID_SIZE_X+1)*(GRID_SIZE_Y+1)];
    verticesCopy = new Vector3D[(GRID_SIZE_X+1)*(GRID_SIZE_Y+1)];

    Vector2D offset(-imgSize.x/2, -imgSize.y/2);
    Vector2D piece(imgSize.x/GRID_SIZE_X, imgSize.y/GRID_SIZE_Y);

    for(int y = 0; y <= GRID_SIZE_Y; y++)
        for(int x = 0; x <= GRID_SIZE_X; x++)
            verticesCopy[y*(GRID_SIZE_X+1)+x] = vertices[y*(GRID_SIZE_X+1)+x] = Vector3D(offset.x + x*piece.x, offset.y + (GRID_SIZE_Y-y)*piece.y, 0.f);

    positionBuf.bind();
    positionBuf.allocate(vertices, (GRID_SIZE_X+1)*(GRID_SIZE_Y+1) * sizeof(Vector3D));   
}

void RippleEffect::initTexCoords()
{
    texCoords = new Vector2D[(GRID_SIZE_X+1)*(GRID_SIZE_Y+1)];
    texCoordsCopy = new Vector2D[(GRID_SIZE_X+1)*(GRID_SIZE_Y+1)];

    for (int y = 0; y <= GRID_SIZE_Y; y++)
        for (int x = 0; x <= GRID_SIZE_X; x++)        
            texCoordsCopy[y*(GRID_SIZE_X+1)+x] = texCoords[y*(GRID_SIZE_X+1)+x] = Vector2D(x/(GLfloat)GRID_SIZE_X, (GRID_SIZE_Y-y)/(GLfloat)GRID_SIZE_Y);

    texCoordBuf.bind();
    texCoordBuf.allocate(texCoords, (GRID_SIZE_X+1)*(GRID_SIZE_Y+1) * sizeof(Vector2D));   
}

void RippleEffect::initIndices()
{
    GLushort *indices = new GLushort[(GRID_SIZE_X+1)*GRID_SIZE_Y*2];

    int idx = 0;
    for (int y = 0; y < GRID_SIZE_Y; y++)
    {
        for (int x = 0; x <= GRID_SIZE_X; x++)
        {
            indices[idx++] = y*(GRID_SIZE_X+1)+x;
            indices[idx++] = (y+1)*(GRID_SIZE_X+1)+x;
        }
    }

    indexBuf.bind();
    indexBuf.allocate(indices, GRID_SIZE_X*GRID_SIZE_Y*6 * sizeof(GLushort));

    delete[] indices;
}

void RippleEffect::update()
{
    for (auto iter = ripples.begin(); iter != ripples.end(); )
    {
        if (iter->delta > iter->duration)
        {
            ripples.erase(iter);
        }
        else
        {
            iter->delta += iter->step;
            iter++;
        }      
    }

    float dx = distortMode == eDistortVertices ? imgSize.x : 1;
    float dy = distortMode == eDistortVertices ? imgSize.y : 1;
    for (int y = 1; y < GRID_SIZE_Y; y++)
    {
        for (int x = 1; x < GRID_SIZE_X; x++)
        {
            int offset = y*(GRID_SIZE_X+1)+x;

            if (distortMode == eDistortVertices)
                vertices[offset] = verticesCopy[offset];
            else
                texCoords[offset] = texCoordsCopy[offset];

            for (RippleData ripple : ripples)
            {
                int mx = x - ripple.gx;
                int my = y - ripple.gy;
                float sx = dx;
                float sy = dy;

                if (mx < 0)
                {
                    mx *= -1;
                    sx *= -1;
                }

                if (my < 0)
                {
                    my *= -1;
                    sy *= -1;
                }

                int r = ripple.delta - g_ripple_vector[mx][my].r;
                if (r < 0)
                    r = 0;
                else if (r > RIPPLE_LENGTH-1)
                    r = RIPPLE_LENGTH-1;

                float amp = 1.f - (float) ripple.delta/RIPPLE_LENGTH;
                amp *= amp;
                if (amp < 0)
                    amp = 0;

                if (distortMode == eDistortVertices)
                {
                    vertices[offset].x += g_ripple_vector[mx][my].dx * sx * g_ripple_amp[r].amplitude * amp;
                    vertices[offset].y += g_ripple_vector[mx][my].dy * sy * g_ripple_amp[r].amplitude * amp;
                }
                else
                {
                    texCoords[offset].x += g_ripple_vector[mx][my].dx * sx * g_ripple_amp[r].amplitude * amp;
                    texCoords[offset].y += g_ripple_vector[mx][my].dy * sy * g_ripple_amp[r].amplitude * amp;
                }
            }
        }
    }

    if (distortMode == eDistortVertices)
    {
        positionBuf.bind();
        positionBuf.write(0, vertices, (GRID_SIZE_X+1)*(GRID_SIZE_Y+1) * sizeof(Vector3D));
    }
    else
    {
        texCoordBuf.bind();
        texCoordBuf.write(0, texCoords, (GRID_SIZE_X+1)*(GRID_SIZE_Y+1) * sizeof(Vector2D));
    }
}

void RippleEffect::draw()
{    
    // Offset for position
    quintptr offset = 0;

    // Tell OpenGL programmable pipeline how to locate vertex position data
    positionBuf.bind();
    int vertexLocation = program->attributeLocation("a_position");
    program->enableAttributeArray(vertexLocation);
    program->setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(Vector3D));

    // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
    texCoordBuf.bind();
    int texcoordLocation = program->attributeLocation("a_texcoord");
    program->enableAttributeArray(texcoordLocation);
    program->setAttributeBuffer(texcoordLocation, GL_FLOAT, offset, 2, sizeof(Vector2D));

    // Draw cube geometry using indices from VBO 1
    indexBuf.bind();
    glDrawElements(GL_TRIANGLE_STRIP, (GRID_SIZE_X+1)*GRID_SIZE_Y*2, GL_UNSIGNED_SHORT, 0);
}

void RippleEffect::addRipple(float x, float y, int step)
{
    x += imgSize.x/2;
    y = imgSize.y - (y + imgSize.y/2);

    RippleData data =
    {
        (int) (x/imgSize.x * GRID_SIZE_X),
        (int) (y/imgSize.y * GRID_SIZE_Y),
        0,
        (int) std::sqrtf(imgSize.x*imgSize.x + imgSize.y*imgSize.y) + RIPPLE_LENGTH,
        step
    };
    ripples.push_back(data);    
}

void RippleEffect::setDistortMode(DistortMode mode)
{
    if (distortMode == eDistortVertices)
    {
        for (int y = 1; y < GRID_SIZE_Y; y++)
            for (int x = 1; x < GRID_SIZE_X; x++)
                vertices[y*(GRID_SIZE_X+1)+x] = verticesCopy[y*(GRID_SIZE_X+1)+x];

        positionBuf.bind();
        positionBuf.write(0, vertices, (GRID_SIZE_X+1)*(GRID_SIZE_Y+1) * sizeof(Vector3D));
    }
    else
    {
        for (int y = 1; y < GRID_SIZE_Y; y++)
            for (int x = 1; x < GRID_SIZE_X; x++)
                texCoords[y*(GRID_SIZE_X+1)+x] = texCoordsCopy[y*(GRID_SIZE_X+1)+x];

        texCoordBuf.bind();
        texCoordBuf.write(0, texCoords, (GRID_SIZE_X+1)*(GRID_SIZE_Y+1) * sizeof(Vector2D));
    }
    distortMode = mode;
}

float RippleEffect::getDistance(const Vector2D& a, const Vector2D& b)
{
    return std::sqrtf((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y));
}

int	RippleEffect::getMaxDistance(const Vector2D& a, const Vector2D& b)
{
    float dist = getDistance(a, Vector2D(0,0));

    float temp = getDistance(a, Vector2D(GRID_SIZE_X, 0));
    if (temp > dist)
        dist = temp;

    temp = getDistance(a, Vector2D(GRID_SIZE_X, GRID_SIZE_Y));
    if (temp > dist)
        dist = temp;

    temp = getDistance(a, Vector2D(0, GRID_SIZE_Y));
    if (temp > dist)
        dist = temp;

    return (int) (dist/GRID_SIZE_X)*b.x + RIPPLE_LENGTH/6;
}
