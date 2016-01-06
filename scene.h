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
    ~Scene();

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
    QOpenGLShaderProgram surfProg, lightProg, chessProg, boardProg, bezierProg;

    QOpenGLVertexArrayObject surfVAO, lightVAO, chessVAO, boardVAO, bezierVAO;
    QOpenGLBuffer
        surfVertexBuf, surfNormalBuf, surfColorBuf, surfTexcoordBuf,
        lampCubeBuf, boardVertexBuffer, bezierPoints;

    QScopedPointer<QOpenGLTexture>
        texTriangles, texTriangleBump, texBoardNormalMap;

    struct ChessObj : public OBJLoader {
        typedef OBJObject *OBJObjectPtr;
        OBJObjectPtr queen, king, tower, knight, bishop, pawn;
        QVector<OBJObjectPtr> beginOrder; // [8] // left to right, white

        void onloaded() override;
    } chess;

    struct ChessPiece {
        OBJObject* type;
        QPoint position; // (0,0): A1; (1,0): B1;
        int color; // 0 is white
    };

    int colorTurn = 0;

    struct Light {
        QVector3D pos;
        QVector3D color;
    } lights[10];
public:
    int nLights = 1;
    QVector3D lookAt;
private:

    QVector3D & light = lights[0].pos;

    QList<ChessPiece*> chessPieces; // size = 32
    QList<QList<ChessPiece*>> knights; // [color][num]

    QMatrix4x4 p, v;
    QVector3D camera;

    void prepareShaderProgram();
    void prepareVertexBuffers();

    float timeEndKnightAnimation = 0;
    struct KnightAnimation {
        float tstart = 0;
        float elapsed = 0; // s
        float duration = 1.5; // s

        float height = 5;
        enum {DEG3, DEG4, LIN} type = DEG3, preffered = DEG3;

        enum {WAIT, RUN, DONE} state = WAIT;

        Scene* scene = nullptr;
        ChessPiece* piece = nullptr;
        QPoint fr, to;
        QVector3D pos3D;
        QVector3D P[4];

        struct {
            KnightAnimation& self;
            void operator=(int x) {
                x = std::max(1,x);
                self.preffered = x % 2 == 1 ? DEG3 : DEG4;
                self.type = x % 2 == 1 ? DEG3 : DEG4;
                self.height = 2.5f * ((x + 1) / 3);
            }
        } mode;

        KnightAnimation() : mode{*this} {}

        QVector3D rightVector();

        void startTo(QPoint target);
        void update(float tnow);

        void bezier();
        void bezier3();
        void bezier4();

        QVector3D bezierDerivative();
        QVector3D bezier3Derivative();
        QVector3D bezier4Derivative();
    };

public:
    KnightAnimation anim;

    struct OnKnightAnimation {
        struct {
            OnKnightAnimation& self;
            void operator =(int v) {
                v = v % 2 == 0 ? 0 : 1;
                if(self.isRunning != v && v)
                    self.start();
                self.isRunning = v;
            }
        } setRunning;

        bool isRunning = false;

        OnKnightAnimation() : setRunning{*this} {}

        float inclinaison = 0;
        float lookAround = 0;

        void start() {
            isRunning = true;
            inclinaison = 0;
            lookAround = 0;
        }
    } onKnightAnim;

    float movementWaiting = 5;
};

#endif // SCENE_H
