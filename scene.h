#ifndef SCENE_H
#define SCENE_H

#include <QScopedPointer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QColor>
#include <QPainter>

class Scene
{
public:
    Scene();

    void initialize();
    void update(double t); // t in seconds
    void render();
    void resize(int width, int height);

    void applyDelta(QPointF delta);
    void applyZoom(float zoom); // +- 1

    void paint(QPainter&);

private:
    QOpenGLShaderProgram mShaderProgram, mShaderProgramLight;
    QOpenGLVertexArrayObject mVAO;
    QOpenGLBuffer mVertexPositionBuffer;
    QOpenGLBuffer mVertexNormalBuffer;
    QOpenGLBuffer mVertexColorBuffer;
    QOpenGLBuffer mVertexCoordBuffer;
    QScopedPointer<QOpenGLTexture> triangles, bump;

    QMatrix4x4 p, m, v;
    QVector3D camera, light;
    float length = 3, theta = 0, phi = 0;

    void prepareShaderProgram();
    void prepareVertexBuffers();
};

#endif // SCENE_H
