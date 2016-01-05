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
    length = 3;
    angleFromUp = radians(60);
    angleOnGround = radians(225); // theta is 2D angle, phi is 3D

    lightSpeed = 1; // seconds / turn
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

    texTriangles.reset(new QOpenGLTexture(QImage(F(":/textures/diag.png")))); // brickwall.jpg
    texTriangleBump.reset(new QOpenGLTexture(QImage(F(":/textures/diag-bump.png")))); // brickwall_normal.jpg
    texBoardNormalMap.reset(new QOpenGLTexture(QImage(F(":/textures/normal-map.png"))));

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
    camera = dep + length * spherical(angleFromUp, angleOnGround);
    light = vec3(lightRadius * polar(lightInitPos + linearAngle(t * lightSpeed)), lightHeight);

    v.setToIdentity();
    v.lookAt(camera, dep, {0, 0, 1});
}

void Scene::applyZoom(float zoom) {
    length = max(0.5, length - zoom * 0.25);
}

void Scene::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto pv = p * v;

    // surface
    {
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

            QMatrix4x4 m;

            m.translate(light);
            m.scale(0.1);

            prog.setUniformValue("matrix", pv * m);
            glDrawArrays(GL_QUADS, 0, 6 * 3 * 4);
        }
    }

    const Matrix boardA1 = Matrix().translate(-0.5, -0.5).translate(-3, -3);

    // chess
    {
        auto& prog = chessProg;
        prog.bind();
        chessVAO.bind();

        prog.setUniformValue("light", light);

        for(int color = 0; color < 2; color++) {
            auto colorA1 = boardA1;

            if(color == 1)
                colorA1.translate(7, 7).rotate(180);

            for(int row = 0; row < 2; row++) {
                for(int i = 0; i < 8; i++) {
                    auto m = colorA1.translated(i, row);
                    OBJObject* obj = row == 0 ? chess.beginOrder[i] : chess.pawn;

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
        }
    }

    // board
    {
        auto& prog = boardProg;
        prog.bind();
        boardVAO.bind();

        prog.setUniformValue("light", light);
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
    angleOnGround += delta.x() * 0.01;
    angleFromUp = clamp<float>(angleFromUp + delta.y() * 0.01, radians(1), radians(179));
}

void Scene::applyMove(QPointF delta) {
    delta = {-delta.y(), -delta.x()};
    dep += 0.010 * vec3(QVector2D(delta).length() * polar(std::atan2(delta.y(), delta.x()) + angleOnGround), 0);
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

    glCheckError();
}
