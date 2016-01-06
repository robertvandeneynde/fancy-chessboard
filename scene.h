#ifndef SCENE_H
#define SCENE_H

#include <QScopedPointer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QColor>
#include <QPainter>
#include <QVector>

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
    void applyMoveForward(float delta);
    void applyZoom(float zoom); // +- 1

public:
    float length = 3;
    float angleFromUp = radians(60);
    float angleOnGround = radians(225); // theta is 2D angle, phi is 3D

    float lightHeight = 1;
    float lightRadius = 1;
    float lightSpeed = 0.2; // turns / second
    float lightInitPos = 0; // radians
    float chessShininess = 32;

    QList<QColor> possibleColors;

    struct {
        Scene* scene = 0;
        void operator =(int x) {
            if(! scene)
                return;
            int C = scene->possibleColors.length();
            int N = C * C; // C ** lights.length()
            x = ((x-1) % N + N) % N;
            scene->lights[1].color = vColor(scene->possibleColors[x % C]);
            scene->lights[2].color = vColor(scene->possibleColors[x / C % C]);
            // x % 3 -> x / C % C -> x / C / C % C -> x / C / C / C % C
        }
    } lightColorsParam;

private:
    QOpenGLShaderProgram surfProg, lightProg, chessProg, boardProg;

    QOpenGLVertexArrayObject surfVAO, lightVAO, chessVAO, boardVAO;
    QOpenGLBuffer
        surfVertexBuf, surfNormalBuf, surfColorBuf, surfTexcoordBuf,
        lampCubeBuf, boardVertexBuffer;

    QScopedPointer<QOpenGLTexture>
        texTriangles, texTriangleBump, texBoardNormalMap;

    struct ChessObj : public OBJLoader {
        typedef OBJObject *OBJObjectPtr;
        OBJObjectPtr queen, king, tower, knight, bishop, pawn;
        QVector<OBJObjectPtr> beginOrder; // [8] // left to right, white

        void onloaded() override;
    } chess;

    struct ChessPiece {
        OBJLoader* chessType;
        QPoint position; // (0,0): A1; (1,0): B1;
        int color; // 0 is white
    };

    struct Light {
        QVector3D pos;
        QVector3D color;
    } lights[10];
public:
    int nLights = 1;
private:

    QVector3D & light = lights[0].pos;

    QVector<ChessPiece> chessPieces; // size = 32

    QMatrix4x4 p, v;
    QVector3D camera, lookAt;

    void prepareShaderProgram();
    void prepareVertexBuffers();
};

#endif // SCENE_H
