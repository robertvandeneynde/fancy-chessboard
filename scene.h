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
#include "objloader.h"

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

public:
    float length = 3;
    float angleFromUp = radians(60);
    float angleOnGround = radians(225); // theta is 2D angle, phi is 3D

    float lightHeight = 1;
    float lightRadius = 1;
    float lightSpeed = 0.2; // turns / second
    float lightInitPos = 0; // radians

private:
    QOpenGLShaderProgram programSurface, programLamp, programChess;
    QOpenGLVertexArrayObject mVAO, mVAOLight, mVAOChess;
    QOpenGLBuffer mVertexPositionBuffer, mVertexNormalBuffer, mVertexColorBuffer, mVertexCoordBuffer,
                  lampCubeBuffer;

    QScopedPointer<QOpenGLTexture> triangles, bump;

    struct ChessObj : public OBJLoader {
        typedef OBJObject *O[2];
        // by color
        O queens, kings, towers, knights, bishops, pawns;
    } chess;

    QMatrix4x4 p, v;
    QVector3D camera, dep, light;

    void prepareShaderProgram();
    void prepareVertexBuffers();
};

#endif // SCENE_H
