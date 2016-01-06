#include "scene.h"

#include "glassert.h"

#include <QtMath>
#include <QObject>
#include <QOpenGLContext>
#include <QOpenGLPaintDevice>
#include <QPainter>

#include <QVector>

#include <cmath>

#include "GL/gl.h"
#include <QRegExp>

#include "utils.h"

#include <stdexcept>
#include <fstream>
#include <QRegularExpression>
#include <QMap>
#include <QFile>

using std::min;
using std::max;

static const QVector<QVector<QVector2D>> letters = makeLetters();

Scene::Scene()
    : surfVertexBuf(QOpenGLBuffer::VertexBuffer)
    , surfColorBuf(QOpenGLBuffer::VertexBuffer)
{
    anim.scene = this;
    srand(89);
    lightColorsParam.scene = this;
    length = 3;
    angleFromUp = radians(60);
    angleOnGround = radians(225); // theta is 2D angle, phi is 3D

    lightSpeed = 1; // seconds / turn

    possibleColors = {
        Qt::white,
        "#cc3333", // cccc00
        "#339933", // 339999
        "#cccc33",
        "#3333ff"
    };

    for(int i = 0; i < 3; i++)
        lights[i].color = vColor(possibleColors[i]);

    lights[1].pos = {4, 0, 0.5};
    lights[2].pos = {-4, 0, 0.5};
}

Scene::~Scene() {
    for(ChessPiece* p : chessPieces)
        delete p;
}

static QVector2D vec2(QPoint p) {
    return {(float)p.x(), (float)p.y()};
}

static QVector2D vec2(QPointF p) {
    return {p.x(), p.y()};
}

static QString F(QString s) {
    if(s.startsWith(":"))
        s = s.mid(1);
    return "/home/robert/cours/3D/FancyChessBoard/" + s;
}

void Scene::initialize()
{
    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CULL_FACE); // default is glFrontFace​(GL_CCW);

    QString files[] = {
        F(":/textures/diag.png"),
        F(":/textures/diag-bump.png"),
        F(":/textures/normal-map.png"),
    };

    int i = 0;
    for(QScopedPointer<QOpenGLTexture>* tt : {&texTriangles, &texTriangleBump, &texBoardNormalMap}) {
        QImage image(files[i]);
        if(image.isNull()) {
            qCritical() << "Error loading texture " << files[i];
            exit(1);
        }
        tt->reset(new QOpenGLTexture(QImage(files[i])));
        ++i;
    }

    texTriangles->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    texTriangles->setMagnificationFilter(QOpenGLTexture::Linear);

    glCheckError();
    prepareShaderProgram();
    prepareVertexBuffers();
}

void Scene::update(double t)
{
    const double speed1 = 10.0; // s / tour
    const double speed2 = 15.0; // s / tour
    // theta = M_PI * sinC(t / speed1); // rad
    // angleOnGround = (t / speed2) * (2 * M_PI); // rad
    camera = lookAt + length * spherical(angleFromUp, angleOnGround);
    light = vec3(lightRadius * polar(lightInitPos + linearAngle(t * lightSpeed)), lightHeight);

    v.setToIdentity();
    v.lookAt(camera, lookAt, {0, 0, 1});

    if(t > timeEndKnightAnimation + movementWaiting && anim.state == anim.WAIT) {
        // start anim
        QList<ChessPiece*> myKnights = knights[rand() % 2];

        QList<QList<QPoint>> possib;
        QPoint ds[] = {{2,-1}, {2,1}, {1,2}, {-1,2}, {-2,1}, {-2,-1}, {-1,-2}, {1,2}};

        for(ChessPiece* knight : myKnights) {
            possib.push_back({});

            for(QPoint d : ds) {
                QPoint p = knight->position + d;
                if(0 <= p.x() && p.x() < 8 && 0 <= p.y() && p.y() < 8) {
                    bool blocked = false;
                    for(ChessPiece* c : chessPieces)
                        if(c->position == p) {
                            blocked = true;
                            break;
                        }

                    if(! blocked)
                        possib.back() << p;
                }
            }
        }

        if(possib[0].length() || possib[1].length()) {
            int n = possib[0].empty() || possib[0].length() && possib[1].length() && rand() % 2 ? 1 : 0;

            anim.tstart = t;
            anim.piece = myKnights[n];
            anim.startTo(possib[n][rand() % possib[n].length()]);
        } else {
            qDebug() << "stuck !";
            anim.state = anim.DONE;
            anim.state = anim.WAIT;
        }
    }

    if(anim.state == anim.RUN) {
        anim.update(t);
        if(anim.state == anim.DONE) {
            timeEndKnightAnimation = t;
            anim.state = anim.WAIT;
        }
    }
}

void Scene::applyZoom(float zoom) {
    length = max(0.5, length - zoom * 0.25);
}

void Scene::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const QVector3D A1Coord = vec3(-3.5, -3.5, 0);
    const Matrix boardA1 = Matrix().translate(A1Coord);

    QVector3D camera = this->camera;

    auto X = vec3(1,0,0), Y = vec3(0,1,0), Z = vec3(0,0,1);

    auto pv = p * v;

    if(onKnightAnim.isRunning && anim.piece) {
        auto T = anim.bezier3Derivative().normalized();
        auto R = anim.rightVector();
        QMatrix4x4 vPrime;
        auto e = A1Coord + anim.pos3D + 2 * Z;
        auto d = vec3(T.toVector2D().normalized(), -1);
        QMatrix4x4 dt;

        dt.rotate(degrees(onKnightAnim.lookAround), Z);
        // dt.rotate(5 * std::sin(2 * 2 * M_PI * anim.elapsed / anim.duration), Z);
        dt.rotate(degrees(onKnightAnim.inclinaison), R);
        d = dt.mapVector(d);
        vPrime.lookAt(e, e + d, Z);
        pv = p * vPrime;
        camera = e;
    }

    // surface
    for(;;){
        break;
        auto& prog = surfProg;
        QMatrix4x4 m;
        surfVAO.bind(); // glBindVertexArray(vao)
        prog.bind();

        prog.setUniformValue("camera", camera);
        prog.setUniformValue("light", light);

        prog.setUniformValue("matrix", pv * m);
        prog.setUniformValue("normMatrix", m.normalMatrix()); // m.inverted().transposed());

        prog.setUniformValue("diag", 0); // texture unit 0
        prog.setUniformValue("bump", 1);

        texTriangles->bind(0); // texture unit 0
        texTriangleBump->bind(1);

        glDrawArrays(GL_TRIANGLES, 0, 3 * 4);

        // mVAO.release(); // glBindVertexArray(0)
        // mShaderProgram.release(); // glUseProgram(0)
    }

    // lamp
    {
        auto& prog = lightProg;
        prog.bind();
        lightVAO.bind();

        // grid and lamp
        {
            /*
            prog.setUniformValue("matrix", pv);
            glDrawArrays(GL_LINES, 6 * 3 * 4, 9 * 4);
            */

            for(int i = 0; i < nLights; i++) {
                QMatrix4x4 m;

                m.translate(lights[i].pos);
                m.scale(0.1);

                prog.setUniformValue("color", lights[i].color);
                prog.setUniformValue("matrix", pv * m);
                glDrawArrays(GL_QUADS, 0, 6 * 3 * 4);
            }
        }
    }

    // chess
    {
        auto& prog = chessProg;
        prog.bind();
        chessVAO.bind();

        prog.setUniformValue("light", light);
        prog.setUniformValue("camera", camera);
        prog.setUniformValue("shininess", chessShininess);

        for(ChessPiece* p : chessPieces) {
            auto m = boardA1;
            auto obj = p->type;

            if(anim.state == anim.RUN && anim.piece == p)
                m.translate(anim.pos3D);
            else
                m.translate(vec2(p->position));

            if(anim.state == anim.RUN && anim.piece == p)
                m.rotate(-90 + degrees(angle2D(vec2(anim.to - anim.fr))));
            else
                if(p->color == 1)
                    m.rotate(180);

            prog.setUniformValue("color", p->color);
            prog.setUniformValue("model", m);
            prog.setUniformValue("matrix", pv * m);
            prog.setUniformValue("normalMatrix", m.normalMatrix());

            obj->bufferVertices.bind();
            prog.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3); // glVertexAttribPointer(...) // one vertexPosition is 3 floats with offset 0, (stride 0)
            obj->bufferNormals.bind();
            prog.setAttributeBuffer("vertexNormal", GL_FLOAT, 0, 3);
            obj->draw();
        }
    }

    // board
    {
        auto& prog = boardProg;
        prog.bind();
        boardVAO.bind();

        prog.setUniformValue("camera", camera);
        prog.setUniformValue("nLights", nLights);
        for(int i = 0; i < nLights; i++) {
            prog.setUniformValue(("lights[" + QString::number(i) + "]").toStdString().c_str(), lights[i].pos);
            prog.setUniformValue(("lightColors[" + QString::number(i) + "]").toStdString().c_str(), lights[i].color);
        }
        prog.setUniformValue("normalMatrix", QMatrix());

        texBoardNormalMap->bind(0); // texture unit 0
        prog.setUniformValue("normalMap", 0);

        for(int i = 0; i < 8; i++) {
            for(int j = 0; j < 8; j++) {
                Matrix m = boardA1.translated(i,j);
                prog.setUniformValue("model", m);
                prog.setUniformValue("matrix", pv * m);
                prog.setUniformValue("color", (i + j) % 2);
                glDrawArrays(GL_QUADS, 0, 4);
            }
        }
    }

    // bezier
    if(anim.state == anim.RUN) {
        auto& prog = bezierProg;
        prog.bind();
        bezierVAO.bind();

        auto m = boardA1;
        prog.setUniformValue("degree", anim.type == anim.DEG3 ? 3 : 4);

        float trail = 0.60f; // [0,1]
        float b = anim.elapsed / anim.duration;
        float a = max(0.f, b - trail);
        auto R = anim.rightVector();

        // 0 0 0, 0 0 3, 2 0 3, 2 0 0
        prog.setUniformValueArray("P", anim.P, 4);
        prog.setUniformValue("matrix", pv * m);
        glDrawArrays(GL_LINE_STRIP, (int) (100 * a), (int) (100 * (b-a)));

        m.translate(0.1 * R);

        prog.setUniformValue("matrix", pv * m);
        glDrawArrays(GL_LINE_STRIP, (int) (100 * a), (int) (100 * (b-a)));

        m.translate(-0.2 * R);

        prog.setUniformValue("matrix", pv * m);
        glDrawArrays(GL_LINE_STRIP, (int) (100 * a), (int) (100 * (b-a)));
    }
}

void Scene::paint(QPainter& p) {
    p.setPen(Qt::red);
    p.setBrush(Qt::yellow);
    p.drawRect(0, 0, 100, 100);
}

void Scene::resize(int width, int height)
{
    glViewport(0, 0, width, height);
    p.setToIdentity();
    p.perspective(70, (float) width / height, 0.1, 100.0);
}

void Scene::applyDelta(QPointF delta) {
    if(onKnightAnim.isRunning) {
        onKnightAnim.lookAround += delta.x() * 0.01;
        onKnightAnim.inclinaison += delta.y() * 0.01;
    } else {
        angleOnGround += delta.x() * 0.01;
        angleFromUp = clamp<float>(angleFromUp + delta.y() * 0.01, radians(1), radians(179));
    }
}

void Scene::applyMove(QPointF delta) {
    delta = {-delta.y(), -delta.x()};
    lookAt += 0.010 * vec3(QVector2D(delta).length() * polar(std::atan2(delta.y(), delta.x()) + angleOnGround), 0);
}

void Scene::applyMoveForward(float delta) {
    QVector3D d = spherical(delta * 0.05, angleFromUp, angleOnGround);
    camera += d;
    lookAt += d;
}

void Scene::prepareShaderProgram()
{
    // add shader, calls init to generate program id (glGenProgram(&programId))
    // create the shader (glCreateShader(&shaderId, type))
    // then attach the shader (glAttachShader(programId, shaderId)

    // surface
    {
        auto& prog = surfProg;
        bool ok = true;
        ok &= prog.addShaderFromSourceFile(QOpenGLShader::Vertex, F(":/shaders/surf.vert"));
        ok &= prog.addShaderFromSourceFile(QOpenGLShader::Fragment, F(":/shaders/surf.frag"));
        ok &= prog.link(); // glLinkProgram(programId)

        if(! ok)
            qCritical() << "error in surface shader" << prog.log(), exit(1);
    }

    // lamp
    {
        auto& prog = lightProg;
        bool ok = true;
        ok &= prog.addShaderFromSourceFile(QOpenGLShader::Vertex, F(":shaders/light.vert"));
        ok &= prog.addShaderFromSourceFile(QOpenGLShader::Fragment, F(":shaders/light.frag"));
        ok &= prog.link();

        if(! ok)
            qCritical() << "error in light shader" << prog.log(), exit(1);
    }

    // chess
    {
        auto& prog = chessProg;
        bool ok = true;
        ok &= prog.addShaderFromSourceFile(QOpenGLShader::Vertex, F(":shaders/chess.vert"));
        ok &= prog.addShaderFromSourceFile(QOpenGLShader::Fragment, F(":shaders/chess.frag"));
        ok &= prog.link();

        if(! ok)
            qCritical() << "error in chess shader" << prog.log(), exit(1);
    }

    // board
    {
        auto& prog = boardProg;
        bool ok = true;
        ok &= prog.addShaderFromSourceFile(QOpenGLShader::Vertex, F(":shaders/board.vert"));
        ok &= prog.addShaderFromSourceFile(QOpenGLShader::Fragment, F(":shaders/board.frag"));
        ok &= prog.link();

        if(! ok)
            qCritical() << "error in board shader" << prog.log(), exit(1);
    }

    // bezier
    {
        auto& prog = bezierProg;
        bool ok = true;
        ok &= prog.addShaderFromSourceFile(QOpenGLShader::Vertex, F(":shaders/bezier.vert"));
        ok &= prog.addShaderFromSourceFile(QOpenGLShader::Fragment, F(":shaders/bezier.frag"));
        ok &= prog.link();

        if(! ok)
            qCritical() << "error in bezier shader" << prog.log(), exit(1);
    }

    glCheckError();
}

void Scene::ChessObj::onloaded() {
    for(OBJObject* obj : objects) {
        for(QVector3D& v : obj->vertices)
            v = {v.x(), v.z(), v.y()};
        for(QVector3D& v : obj->normals)
            v = {v.x(), v.z(), v.y()};

        obj->calculateGeometry();

        QVector3D targetCenter = {0, 0, - obj->geom.size.z() / 2};
        for(QVector3D& v : obj->vertices)
            v = 1/250.0 * (v - obj->geom.center - targetCenter);

        obj->calculateGeometry();
    }

    QMapIterator<QString, OBJObject*> it(objects);
    while(it.hasNext()) {
        it.next();
        OBJObject* obj = it.value();
        auto& geom = obj->geom;

        qDebug() << it.key() << ":"
                 << "vtx:" << obj->vertices.size()
                 << "tri:" << obj->triangles.size()
                 << "quads:" << obj->quads.size()
                 << "norm:" << obj->normals.size()
                 << "texCoord:" << obj->texCoord.size()
                 << "center:" << geom.center
                 << "size:" << geom.size;

    }

    QString names[6] = {"TOWER", "KNIGHT", "BISHOP", "QUEEN", "KING", "PAWN"};
    OBJObject** pointers[] = {&tower, &knight, &bishop, &queen, &king, &pawn};

    for(int i = 0; i < 6; i++)
        if(!(*pointers[i] = objects.value(names[i], nullptr)))
            qDebug() << "Missing piece " << names[i];

    beginOrder = {tower, knight, bishop, queen, king, bishop, knight, tower};
}

void Scene::prepareVertexBuffers()
{
    chess.load(F(":/models/chess-one.obj"));

    chessPieces.reserve(16);
    for(int color = 0; color < 2; color++) {
        for(int i = 0; i < 8; i++) {
            chessPieces.append(new ChessPiece {
                chess.beginOrder[i],
                {i, color == 0 ? 0 : 7},
                color
            });
        }

        knights.append({
            chessPieces[chessPieces.length() - 2],
            chessPieces[chessPieces.length() - 7]
        });

        for(int i = 0; i < 8; i++) {
            chessPieces.append(new ChessPiece {
                chess.pawn,
                {i, color == 0 ? 1 : 6},
                color
            });
        }
    }

    // surface
    {
        auto& prog = surfProg;
        auto& vao = surfVAO;
        vao.create(); // glGenVertexArrays(1, &vao)
        vao.bind(); // glBindVertexArray(vao)

        // position / normal
        {
            QVector3D a = {0,0,0}, b = {0, 1.5, 0}, c = {1, 0, 0}, d = {0.5, 0.5, 1};

            QVector3D P[4][3] = {
                {d,b,c}, {a,b,c}, {a,c,d}, {a,d,b}
            };

            QVector3D N[4][3];
            for(int i = 0; i < 4; i++)
                N[i][0] = N[i][1] = N[i][2] = cross(P[i][1] - P[i][0], P[i][2] - P[i][0]).normalized();

            {
                auto& buf = surfVertexBuf;
                buf.create(); // glGenBuffers(1, &id);
                buf.setUsagePattern(QOpenGLBuffer::StaticDraw);
                buf.bind(); // glBindBuffer(type, id); // type = (VertexBuffer = default) | (IndexBuffer) | (PixelPackBuffer) | (PixelUnpackBuffer) = GL_ARRAY_BUFFER | GL_ELEMENT_ARRAY_BUFFER | GL_PIXEL_PACK_BUFFER | GL_PIXEL_UNPACK_BUFFER
                buf.allocate(P, sizeof(P)); // glBufferData(TYPE, count, data, USAGE);
            }

            {
                auto& buf = surfNormalBuf;
                buf.create();
                buf.setUsagePattern(QOpenGLBuffer::StaticDraw);
                buf.bind();
                buf.allocate(N, sizeof(N));
            }
        }

        // color
        {
            const QVector3D
                a = vColor(Qt::red), b = vColor(Qt::yellow),
                c = vColor(Qt::green), d = vColor(Qt::blue);

            const QVector3D colorData[][3] = {
                {b,c,d}, {a,d,c}, {a,b,d}, {a,c,b}
            };

            auto& buf = surfColorBuf;
            buf.create();
            buf.setUsagePattern(QOpenGLBuffer::StaticDraw);
            buf.bind();
            buf.allocate(colorData, sizeof(colorData));
        }

        // tex coord
        {
            const QVector2D a = {0,0}, b = {1,0}, c = {0,1}, d = {1,1};

            const QVector2D vtxCoordData[][3] = {
                {a, b, c}, {b, d, c}, {a, b, c}, {a, b, c}
            };

            auto& buf = surfTexcoordBuf;
            buf.create();
            buf.setUsagePattern(QOpenGLBuffer::StaticDraw);
            buf.bind();
            buf.allocate(vtxCoordData, sizeof(vtxCoordData));
        }

        prog.bind(); // glUseProgram(&id) // use shader program

        surfVertexBuf.bind(); // glBindBuffer(type, id); in shader program
        prog.enableAttributeArray("vertexPosition"); // glEnableVertexAttribArray(location(name))
        prog.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3); // glVertexAttribPointer(...) // one vertexPosition is 3 floats with offset 0, (stride 0)

        // setAttributeBuffer(location, type, offset, tupleSize, stride)
        // = glVertexAttribPointer(location, tupleSize, type, GL_TRUE, stride, reinterpret_cast<const void *>(offset));
        // GL_TRUE means normalized, see https://www.opengl.org/sdk/docs/man/html/glVertexAttribPointer.xhtml

        surfNormalBuf.bind();
        prog.enableAttributeArray("vertexNormal");
        prog.setAttributeBuffer("vertexNormal", GL_FLOAT, 0, 3);

        surfColorBuf.bind();
        prog.enableAttributeArray("vertexColor");
        prog.setAttributeBuffer("vertexColor", GL_FLOAT, 0, 3);

        surfTexcoordBuf.bind();
        prog.enableAttributeArray("vertexCoord");
        prog.setAttributeBuffer("vertexCoord", GL_FLOAT, 0, 2);

        vao.release();
    } // surface

    // lamp
    {
        auto& prog = lightProg;
        prog.bind();

        auto& vao = lightVAO;
        vao.create();
        vao.bind();

        /*
        const int rev[4] = {0,1,1,0};
        const QVector2D down[4] = {{0,0}, {1,0}, {1,1}, {0,1}};
        QVector3D P[6][4];
        QVector3D N[6];

        for(int i = 0; i < 6; i++) {
            for(int j = 0; j < 4; j++) {
                P[i][j] = i < 4 ?
                    vec3(down[(i + j % 2) % 4], rev[j / 2]) :
                    vec3(down[i == 4 ? 3 - j : j], i - 4);
            }
            N[i] = QVector3D::crossProduct(P[i][1] - P[i][0], P[i][2] - P[i][0]);
        }
        */


        QVector3D ds[3][6][4];
        for(int i = 0; i < 3; i++) {
            for(int j = 0; j < 6; j++) {
                ds[i][j][0] = spherical(M_PI * (i+0) / 3, 2 * M_PI * (j+0) / 6);
                ds[i][j][1] = spherical(M_PI * (i+0) / 3, 2 * M_PI * (j+1) / 6);
                ds[i][j][2] = spherical(M_PI * (i+1) / 3, 2 * M_PI * (j+1) / 6);
                ds[i][j][3] = spherical(M_PI * (i+1) / 3, 2 * M_PI * (j+0) / 6);
            }
        }

        QVector3D grids[9][4];
        for(int i = 0; i < 9; i++) {
            grids[i][0] = {i-4, -4, 0};
            grids[i][1] = {i-4, +4, 0};
            grids[i][2] = {-4, i-4, 0};
            grids[i][3] = {+4, i-4, 0};
        }

        auto& buf = lampCubeBuf;
        buf.create();
        buf.setUsagePattern(QOpenGLBuffer::StaticDraw);
        buf.bind();
        buf.allocate(sizeof(ds) + sizeof(grids));
        buf.write(0, ds, sizeof(ds));
        buf.write(sizeof(ds), grids, sizeof(grids));

        prog.enableAttributeArray("vertexPosition");
        prog.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3);

        vao.release();
    }

    // chess
    {
        auto& prog = chessProg;
        auto& vao = chessVAO;

        prog.bind();
        vao.create();
        vao.bind();

        for(OBJObject* obj : chess.objects.values())
            obj->loadBuffers();

        prog.enableAttributeArray("vertexPosition");
        prog.enableAttributeArray("vertexNormal");

        vao.release();
    } // lamp

    // board
    {
        QVector3D data[] = {
            {0,0,0},
            {1,0,0},
            {1,1,0},
            {0,1,0},
        };

        for(QVector3D& v : data)
            v -= {0.5, 0.5, 0};

        auto& prog = boardProg;
        auto& vao = boardVAO;

        vao.create();
        vao.bind();
        prog.bind();

        auto& buf = boardVertexBuffer;

        buf.create();
        buf.setUsagePattern(QOpenGLBuffer::StaticDraw);
        buf.bind();
        buf.allocate(data, sizeof(data));

        prog.enableAttributeArray("vertexPosition");
        prog.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3);
        // prog.enableAttributeArray("vertexNormal");

        vao.release();
    }

    // bezier
    {
        float times[100];
        for(int i = 0; i < 100; i++)
            times[i] = i / 100.0;

        auto& vao = bezierVAO;
        auto& prog = bezierProg;

        vao.create();
        vao.bind();
        prog.bind();

        auto& buf = bezierPoints;

        buf.create();
        buf.setUsagePattern(QOpenGLBuffer::StaticDraw);
        buf.bind();
        buf.allocate(times, sizeof(times));
        prog.enableAttributeArray("t");
        prog.setAttributeBuffer("t", GL_FLOAT, 0, 1);

        vao.release();
    }

    glCheckError();
}

QVector3D Scene::KnightAnimation::rightVector() {
    return vec3(polar(angle2D(vec2(to - fr)) - M_PI/2), 0);
}

void Scene::KnightAnimation::startTo(QPoint target) {
    state = RUN;
    fr = piece->position;
    to = target;
    if(scene->onKnightAnim.isRunning)
        scene->onKnightAnim.start();

    if(type == DEG3) {
        P[0] = vec3(vec2(fr), 0);
        P[2] = vec3(vec2(to), 0);
        P[1] = vec3((vec2(fr) + vec2(to)) * 0.5, height);
    } else if(type == DEG4) {
        P[0] = P[1] = vec3(vec2(fr), 0);
        P[3] = P[2] = vec3(vec2(to), 0);
        P[1][2] = P[2][2] = height;
    }
}

void Scene::KnightAnimation::update(float tnow) {
    elapsed = tnow - tstart;
    bezier();
    if(elapsed > duration) {
        state = DONE;
        piece->position = to;
    }
}

void Scene::KnightAnimation::bezier() {
    if(type == DEG3)
        bezier3();
    else if(type == DEG4)
        bezier4();
}

QVector3D Scene::KnightAnimation::bezierDerivative() {
    if(type == DEG3)
        return bezier3Derivative();
    else if(type == DEG4)
        return bezier4Derivative();
    return {};
}

void Scene::KnightAnimation::bezier3() {
    float t = elapsed / duration;
    float u = 1 - t;
    pos3D = u*u*P[0] + 2*u*t * P[1] + t*t * P[2];
}

QVector3D Scene::KnightAnimation::bezier3Derivative() {
    float t = elapsed / duration;
    float u = 1 - t;
    return 2*u * (P[1]-P[0]) + 2*t * (P[2]-P[1]);
}

void Scene::KnightAnimation::bezier4() {
    float t = elapsed / duration;
    float u = 1 - t;
    float t2 = t*t, u2 = u*u;
    pos3D = u*u2*P[0] + 3*u2*t * P[1] + 3*u*t2 * P[2] + t*t2 * P[3];
}

QVector3D Scene::KnightAnimation::bezier4Derivative() {
    float t = elapsed / duration;
    float u = 1 - t;
    return 3*u*u*(P[1]-P[0]) + 6*u*t * (P[2]-P[1]) + 3*t*t * (P[3]-P[2]);
}
