#ifndef SCENE_H
#define SCENE_H

#include <QScopedPointer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QColor>
#include <QPainter>

#include "utils.h"

class Scene
{
public:
    Scene();

    void initialize();
    void update(double t); // t in seconds
    void render();
    void resize(int width, int height);

    void paint(QPainter&);

public slots:
    void applyDelta(QPointF delta);
    void applyMove(QPointF delta);
    void applyZoom(float zoom); // +- 1

public slots:
    void setLength(float x) { length = 3; }
    void setLightSpeed(float x) { lightSpeed = x; }
    void setAngleFromUp(float angleFromUp) { this->angleFromUp = angleFromUp; }
    void setAngleOnGround(float angleOnGround) { this->angleOnGround = angleOnGround; }

public:
    float getLength() const { return length; }
    float getLightSpeed() const { return lightSpeed; }
    float getAngleFromUp() const { return angleFromUp; }
    float getAngleOnGround() const { return angleOnGround; }

private:
    float length = 3;
    float angleFromUp = radians(60);
    float angleOnGround = radians(225); // theta is 2D angle, phi is 3D

    float lightSpeed = 0.2; // turns / second

private:
    QOpenGLShaderProgram mShaderProgram, mShaderProgramLamp;
    QOpenGLVertexArrayObject mVAO, mVAOLight;
    QOpenGLBuffer mVertexPositionBuffer, mVertexNormalBuffer, mVertexColorBuffer, mVertexCoordBuffer,
                  lampCubeBuffer;

    QScopedPointer<QOpenGLTexture> triangles, bump;

    QMatrix4x4 p, m;
    QVector3D camera, dep, light;

    void prepareShaderProgram();
    void prepareVertexBuffers();
};

#endif // SCENE_H
