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
#include <QMap>

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

    static void glCheckError();

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
            x = ((x-1) % N + N) % N; // 1-based, modulo N
            scene->lights[1].color = vColor(scene->possibleColors[x % C]);
            scene->lights[2].color = vColor(scene->possibleColors[x / C % C]);
            // f(i=0) = x % C, f(1) = x / C % C, f(2) = x / C / C % C, f(3) = x / C / C / C % C, f(i) = x / C**i % C
        }

        operator int() {
            if(! scene)
                return 0;
            int C = scene->possibleColors.length();
            int a = scene->possibleColors.indexOf(color3(scene->lights[1].color));
            int b = scene->possibleColors.indexOf(color3(scene->lights[2].color));
            if(a == -1 || b == -1)
                return 0;
            return a * C + b;
        }
    } lightColorsParam;

    struct {
        int v = 0;

        operator int(){
            return v;
        }

        void operator =(int x){
            auto N = NCUBEMAP;
            v = (x % N + N) % N;
        }
    } currentCubeMap;

private:
    QOpenGLShaderProgram surfProg, lightProg, chessProg, boardProg, bezierProg, cubeMapProg;

    QOpenGLVertexArrayObject surfVAO, lightVAO, chessVAO, boardVAO, bezierVAO, cubeMapVAO;
    QOpenGLBuffer
        surfVertexBuf, surfNormalBuf, surfColorBuf, surfTexcoordBuf,
        lampCubeBuf, boardVertexBuffer, bezierPoints, cubeMapPoints;

    static const int NCUBEMAP = 7;
    QString cubeMapFilenames[NCUBEMAP] = {
        "ame_desert/%1.png",
        "highqual/Park2/%1.jpg",
        "highqual/SwedishRoyalCastle/%1.jpg",
        "highqual/Vasa/%1.jpg",
        "highqual/Creek/%1.jpg",
        "skybox/%1.jpg",
        "lmcity/%1.png",
    };

    QScopedPointer<QOpenGLTexture>
        texTriangles, texTriangleBump, texBoardNormalMap, cubeMapTextures[NCUBEMAP];

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
    float reflectFactor = 0.2;
    float refractFactor = 0.1;
    float refractIndice = 0.2;

    float cookLambda = 0.4;
    float cookRoughness = 0.2;
    int lightingModel = 0; // PHONG BLING-PHONG COOK

private:
    QVector3D & light = lights[0].pos;

    QList<ChessPiece*> chessPieces; // size = 32
    QList<QList<ChessPiece*>> knights; // [color][num]

    QMatrix4x4 p, v;
    QVector3D camera;

    void loadTextures();
    void loadModels();
    void prepareShaderProgram();
    void prepareVertexBuffers();

    // animations
public:

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
                self.height = getHeight(x);
            }

            float getHeight(int i) {
                return 1.f * ((std::max(1,i) + 1) / 2);
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

    struct Falling {
        Scene* scene;
        float timeCutOff = 15; //s
        bool running = false;
        QList<float> positions; // of top point
        QList<float> velocities; // of top point
        QList<float> masses;

        float g = 10.0;
        float k = 60;
        float alpha = 3;
        float startingHeight = 5;

        float firstT = 0, lastT = 0;

        void start(float t) {
            positions.clear();
            velocities.clear();
            masses.clear();

            const float rho = 1;
            running = true;
            firstT = lastT = t;
            for(ChessPiece* obj : scene->chessPieces) {
                float random = rand() / (float)RAND_MAX;
                positions.append(startingHeight + random * 1.50 + obj->type->geom.size.z());
                velocities.append(0);
                masses.append(rho * obj->type->geom.size.x() * obj->type->geom.size.y() * obj->type->geom.size.z());
            }
        }

        void update(float t) {
            const float eps = 0.1;

            bool stop = true;
            float dt = t - lastT;
            auto pp = positions.begin(); auto vv = velocities.begin(); auto mm = masses.begin();
            for(ChessPiece* obj : scene->chessPieces) {
                auto& p = *pp++; auto& v = *vv++; auto& m = *mm++;
                auto H = obj->type->geom.size.z();
                float d = H - p;
                float a = d > 0 ? d * k / m - alpha * v : -g;
                v += a * dt;
                p += v * dt;
                if(!(abs(v) < eps && abs(a) < eps))
                    stop = false;
            }
            lastT = t;
            if(stop || t - firstT > timeCutOff) {
                running = false;
            }
        }
    } falling;
};

#endif // SCENE_H
