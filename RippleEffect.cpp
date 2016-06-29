#include "RippleEffect.h"
#include "RippleTable.h"
#include <cmath>

RippleEffect::RippleEffect(QOpenGLShaderProgram *program, float w, float h)
    : program(program), indexBuf(QOpenGLBuffer::IndexBuffer), size(w, h)
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

    positionBuf.destroy();
    texCoordBuf.destroy();
    indexBuf.destroy();
}

void RippleEffect::initPositions()
{
    vertices = new Vector3D[(GRID_SIZE_X+1)*(GRID_SIZE_Y+1)];
    verticesCopy = new Vector3D[(GRID_SIZE_X+1)*(GRID_SIZE_Y+1)];

    Vector2D offset(-size.x/2, -size.y/2);
    Vector2D piece(size.x/GRID_SIZE_X, size.y/GRID_SIZE_Y);

    for(int y = 0; y <= GRID_SIZE_Y; y++)
        for(int x = 0; x <= GRID_SIZE_X; x++)
            verticesCopy[y*(GRID_SIZE_X+1)+x] = vertices[y*(GRID_SIZE_X+1)+x] = Vector3D(offset.x + x*piece.x, offset.y + (GRID_SIZE_Y-y)*piece.y, 0.f);

    positionBuf.bind();
    positionBuf.allocate(vertices, (GRID_SIZE_X+1)*(GRID_SIZE_Y+1) * sizeof(Vector3D));   
}

void RippleEffect::initTexCoords()
{
    Vector2D* texCoords = new Vector2D[(GRID_SIZE_X+1)*(GRID_SIZE_Y+1)];

    for (int y = 0; y <= GRID_SIZE_Y; y++)
        for (int x = 0; x <= GRID_SIZE_X; x++)        
            texCoords[y*(GRID_SIZE_X+1)+x] = Vector2D(x/(GLfloat)GRID_SIZE_X, (GRID_SIZE_Y-y)/(GLfloat)GRID_SIZE_Y);

    texCoordBuf.bind();
    texCoordBuf.allocate(texCoords, (GRID_SIZE_X+1)*(GRID_SIZE_Y+1) * sizeof(Vector2D));

    delete[] texCoords;
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

    for (int y = 1; y < GRID_SIZE_Y; y++)
    {
        for (int x = 1; x < GRID_SIZE_X; x++)
        {
            int offset = y*(GRID_SIZE_X+1)+x;
            vertices[offset] = verticesCopy[offset];

            for (RippleData ripple : ripples)
            {
                int mx = x - ripple.gx;
                int my = y - ripple.gy;
                float sx = size.x;
                float sy = size.y;

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

                vertices[offset].x += g_ripple_vector[mx][my].dx * sx * g_ripple_amp[r].amplitude * amp;
                vertices[offset].y += g_ripple_vector[mx][my].dy * sy * g_ripple_amp[r].amplitude * amp;
            }
        }
    }

    positionBuf.bind();
    positionBuf.write(0, vertices, (GRID_SIZE_X+1)*(GRID_SIZE_Y+1) * sizeof(Vector3D));
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
    x += size.x/2;
    y = size.y - (y + size.y/2);

    RippleData data =
    {
        (int) (x/size.x * GRID_SIZE_X),
        (int) (y/size.y * GRID_SIZE_Y),
        0,
        (int) std::sqrtf(size.x*size.x + size.y*size.y) + RIPPLE_LENGTH,
        step
    };
    ripples.push_back(data);    
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
