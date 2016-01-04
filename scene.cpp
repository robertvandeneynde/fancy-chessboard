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

static const QVector<QVector<QVector2D>> letters = makeLetters();

Scene::Scene()
    : programSurface(),
      programLamp(),
      mVertexPositionBuffer(QOpenGLBuffer::VertexBuffer),
      mVertexColorBuffer(QOpenGLBuffer::VertexBuffer)
{
    length = 3;
    angleFromUp = radians(60);
    angleOnGround = radians(225); // theta is 2D angle, phi is 3D

    lightSpeed = 1; // seconds / turn
}

void Scene::initialize()
{
    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CULL_FACE); // default is glFrontFace​(GL_CCW);

    triangles.reset(new QOpenGLTexture(QImage(":/textures/diag.png"))); // brickwall.jpg
    bump.reset(new QOpenGLTexture(QImage(":/textures/diag-bump.png"))); // brickwall_normal.jpg

    triangles->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    triangles->setMagnificationFilter(QOpenGLTexture::Linear);

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
    length -= zoom * 0.25;
}

void Scene::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto pv = p * v;

    // surface
    {
        auto& prog = programSurface;
        QMatrix4x4 m;
        mVAO.bind(); // glBindVertexArray(vao)
        prog.bind();

        prog.setUniformValue("camera", camera);
        prog.setUniformValue("light", light);

        prog.setUniformValue("matrix", pv * m);
        prog.setUniformValue("normMatrix", m.normalMatrix()); // m.inverted().transposed());

        prog.setUniformValue("diag", 0); // texture unit 0
        prog.setUniformValue("bump", 1);

        triangles->bind(0); // texture unit 0
        bump->bind(1);

        glDrawArrays(GL_TRIANGLES, 0, 3 * 4);

        // mVAO.release(); // glBindVertexArray(0)
        // mShaderProgram.release(); // glUseProgram(0)
    }

    // lamp
    {
        auto& prog = programLamp;
        prog.bind();
        mVAOLight.bind();

        // grid and lamp
        {
            prog.setUniformValue("matrix", pv);
            glDrawArrays(GL_LINES, 6 * 3 * 4, 21 * 4);

            QMatrix4x4 m;

            m.translate(light);
            m.scale(0.1);

            prog.setUniformValue("matrix", pv * m);
            glDrawArrays(GL_QUADS, 0, 6 * 3 * 4);
        }
    }

    // chess
    {
        auto& prog = programLamp;
        mVAOChess.bind();
        /*
        QMatrix4x4 m;
        for(OBJObject* obj : chess.objects.values()) {
            m.translate(2,0,0);
            QMatrix4x4 l;
            l.scale(1/200.0);
            prog.setUniformValue("matrix", pv * m * l);
            glDrawElements(GL_TRIANGLES, obj->triangles.size(), GL_UNSIGNED_INT, obj->triangles.data());
        }
        */
        OBJObject* obj = chess.objects["RE2"];
        QMatrix4x4 m;
        m.scale(1/200.0);

        auto A = obj->boundingBoxMin(), B = obj->boundingBoxMax();
        m.translate((A + B) / -2);
        m.translate(0, 0, (B.z() - A.z()) / 2);
        prog.setUniformValue("matrix", pv * m);

        obj->draw();
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
        bool ok = true;
        auto& prog = programSurface;
        ok &= prog.addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/phong.vert");
        ok &= prog.addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/phong.frag");
        ok &= prog.link(); // glLinkProgram(programId)

        if(! ok)
            qCritical() << "error in base shader" << endl;
    }

    // lamp
    {
        auto& prog = programLamp;
        bool ok = true;
        ok &= prog.addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/light.vert");
        ok &= prog.addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/light.frag");
        ok &= prog.link();

        if(! ok)
            qCritical() << "error in light shader" << prog.log();
    }

    glCheckError();
}

void Scene::prepareVertexBuffers()
{
    chess.load(":/models/chess.obj");

    /*
    chess.objects["RE2"]->vertices = {{-1000, 1000, 0}, {-1000, 1100, 0}, {-800, 1100, 0}, {-800, 1000, 0}, {-800, 1000, 100}, {-800, 1100, 100}, {-900, 1050, 150}};
    chess.objects["RE2"]->triangles = {0,1,2, 0,2,3, 2,3,4, 2,5,4, 4,5,6};
    */

    for(auto key : chess.objects.keys()) {
        OBJObject* obj = chess.objects.value(key);
        auto m = obj->boundingBoxMin();
        auto M = obj->boundingBoxMax();
        qDebug() << key << ":" << m << M << (m+M)/2 << (M-m) << obj->triangles.size();
        for(QVector3D& v : obj->vertices)
            v = {v.x(), v.z(), v.y()};
    }

    QString names[6][2] = {{"TORRE1", "TORRE2"}, {"CAVALIERE1", "CAVALIERE2"}, {"ALFIERE1", "ALFIERE2"}, {"REGINA1", "REGINA2"}, {"RE1", "RE2"}, {"PEDONE1", "PEDONE2"}};

    for(int i = 0; i < 6; i++)
    for(int j = 0; j < 2; j++)
        if(! chess.objects.contains(names[i][j]))
            qCritical() << "Missing pieces", exit(1);

    for(int i = 0; i < 2; i++) {
        chess.towers[i] = chess.objects.value(names[0][i]);
        chess.knights[i] = chess.objects.value(names[1][i]);
        chess.bishops[i] = chess.objects.value(names[2][i]);
        chess.queens[i] = chess.objects.value(names[3][i]);
        chess.kings[i] = chess.objects.value(names[4][i]);
        chess.pawns[i] = chess.objects.value(names[5][i]);
    }


    // surface
    {
        mVAO.create(); // glGenVertexArrays(1, &vao)
        mVAO.bind(); // glBindVertexArray(vao)

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
                auto& buf = mVertexPositionBuffer;
                buf.create(); // glGenBuffers(1, &id);
                buf.setUsagePattern(QOpenGLBuffer::StaticDraw);
                buf.bind(); // glBindBuffer(type, id); // type = (VertexBuffer = default) | (IndexBuffer) | (PixelPackBuffer) | (PixelUnpackBuffer) = GL_ARRAY_BUFFER | GL_ELEMENT_ARRAY_BUFFER | GL_PIXEL_PACK_BUFFER | GL_PIXEL_UNPACK_BUFFER
                buf.allocate(P, sizeof(P)); // glBufferData(TYPE, count, data, USAGE);
            }

            {
                auto& buf = mVertexNormalBuffer;
                buf.create();
                buf.setUsagePattern(QOpenGLBuffer::StaticDraw);
                buf.bind();
                buf.allocate(N, sizeof(N));
            }
        }

        // color
        {
            const QVector3D a = vColor(Qt::red), b = vColor(Qt::yellow), c = vColor(Qt::green), d = vColor(Qt::blue);

            const QVector3D colorData[][3] = {
                {b,c,d}, {a,d,c}, {a,b,d}, {a,c,b}
            };

            auto& buf = mVertexColorBuffer;
            buf.create();
            buf.setUsagePattern(QOpenGLBuffer::StaticDraw);
            buf.bind();
            buf.allocate(colorData, sizeof(colorData));
        }

        // tex coord
        {
            const QVector2D w = {0,0}, x = {1,0}, y = {0,1}, z = {1,1};

            const QVector2D vtxCoordData[][3] = {
                {w, x, y}, {x, z, y}, {w, x, y}, {w, x, y}
            };

            auto& buf = mVertexCoordBuffer;
            buf.create();
            buf.setUsagePattern(QOpenGLBuffer::StaticDraw);
            buf.bind();
            buf.allocate(vtxCoordData, sizeof(vtxCoordData));
        }

        auto& prog = programSurface;
        prog.bind(); // glUseProgram(&id) // use shader program

        mVertexPositionBuffer.bind(); // glBindBuffer(type, id); in shader program
        prog.enableAttributeArray("vertexPosition"); // glEnableVertexAttribArray(location(name))
        prog.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3); // glVertexAttribPointer(...) // one vertexPosition is 3 floats with offset 0, (stride 0)

        // setAttributeBuffer(location, type, offset, tupleSize, stride)
        // = glVertexAttribPointer(location, tupleSize, type, GL_TRUE, stride, reinterpret_cast<const void *>(offset));
        // GL_TRUE means normalized, see https://www.opengl.org/sdk/docs/man/html/glVertexAttribPointer.xhtml

        mVertexNormalBuffer.bind();
        prog.enableAttributeArray("vertexNormal");
        prog.setAttributeBuffer("vertexNormal", GL_FLOAT, 0, 3);

        mVertexColorBuffer.bind();
        prog.enableAttributeArray("vertexColor");
        prog.setAttributeBuffer("vertexColor", GL_FLOAT, 0, 3);

        mVertexCoordBuffer.bind();
        prog.enableAttributeArray("vertexCoord");
        prog.setAttributeBuffer("vertexCoord", GL_FLOAT, 0, 2);
        mVAO.release();
    } // surface

    // lamp
    {
        mVAOLight.create();
        mVAOLight.bind();

        auto& prog = programLamp;
        prog.bind();

        /*
        const QVector3D
            a = { 1, 1, 1}, b = { 1,-1, 1}, c = {-1,-1, 1}, d = {-1, 1, 1},
            e = { 1, 1,-1}, f = { 1,-1,-1}, g = {-1,-1,-1}, h = {-1, 1,-1};

        const QVector3D ds[][4] = {
            {a,d,c,b}, {e,a,b,f}, {h,e,f,g}, {d,h,g,c}, // front, right, back, left
            {e,h,d,a}, {b,c,g,f}, // top, bottom
        };
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

        QVector3D grids[21][4];
        for(int i = 0; i < 21; i++) {
            grids[i][0] = {i-10, -10, 0};
            grids[i][1] = {i-10, +10, 0};
            grids[i][2] = {-10, i-10, 0};
            grids[i][3] = {+10, i-10, 0};
        }

        auto& buf = lampCubeBuffer;
        buf.create();
        buf.setUsagePattern(QOpenGLBuffer::StaticDraw);
        buf.bind();
        buf.allocate(sizeof(ds) + sizeof(grids));
        buf.write(0, ds, sizeof(ds));
        buf.write(sizeof(ds), grids, sizeof(grids));

        prog.enableAttributeArray("vertexPosition");
        prog.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3);

        mVAOLight.release();
    }

    // chess
    {
        auto& prog = programLamp;
        prog.bind();

        mVAOChess.create();
        mVAOChess.bind();

        /*
        for(OBJObject* obj : chess.objects.values())
            obj->loadBuffers();
        */
        chess.objects["RE2"]->loadBuffers();

        prog.enableAttributeArray("vertexPosition");
        prog.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3);

        mVAOChess.release();
    } // lamp

    glCheckError();
}
