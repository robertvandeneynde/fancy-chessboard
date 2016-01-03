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
#include <QRegularExpression>

static const QVector<QVector<QVector2D>> letters = makeLetters();

Scene::Scene()
    : mShaderProgram(),
      mVertexPositionBuffer(QOpenGLBuffer::VertexBuffer),
      mVertexColorBuffer(QOpenGLBuffer::VertexBuffer)
{
    theta = radians(75);
    phi = radians(45 + 180);
}

void Scene::initialize()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

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
    // double theta = M_PI * sinC(t / speed1); // rad
    // double phi = (t / speed2) * (2 * M_PI); // rad
    camera = spherical(length, theta, phi);
    light = to3D(1 * polar(linearAngle(t / 15)), 1);

    m.setToIdentity();
    m.lookAt(camera, {0, 0, 0}, {0, 0, 1});
}

void Scene::applyZoom(float zoom) {
    length -= zoom * 0.25;
}

void Scene::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto pmv = p * (m * v);

    //
    /*
    mShaderProgramLight.bind();
    mShaderProgramLight.setUniformValue("matrix", pmv);
    mShaderProgramLight.enableAttributeArray("vertexColor");
    */

    // mShaderProgram.release(); // fixed pipeline

    /*
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBegin(GL_TRIANGLES);
    glColor3f(1,0,0);
    glVertex3f(1,1,0);
    glVertex3f(1,1,0);
    glVertex3f(1,1,0);
    glEnd();
    */

    /*
    QVector3D a,b,c,d, e,f,g,h;
    a = { 1, 1, 1}, b = { 1,-1, 1}, c = {-1,-1, 1}, d = {-1, 1, 1},
    e = { 1, 1,-1}, f = { 1,-1,-1}, g = {-1,-1,-1}, h = {-1, 1,-1};

    QVector3D ds[] = {
        d,c,b,a, e,f,g,h,
        a,b,e,f, c,d,g,h,
        b,c,f,g, d,a,h,a
    };

    glBegin(GL_QUADS);

    for(QVector3D d : ds) {
        QVector3D l = d; // + light;
        glVertex3fv(reinterpret_cast<const GLfloat*>(&l));
    }

    glEnd();

    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    */

    mShaderProgram.bind();
    mShaderProgram.setUniformValue("camera", camera);
    mShaderProgram.setUniformValue("light", light);

    mShaderProgram.setUniformValue("matrix", pmv);
    mShaderProgram.setUniformValue("normMatrix", v.inverted().transposed());

    mVAO.bind(); // glBindVertexArray(vao)

    mShaderProgram.setUniformValue("diag", 0); // texture unit 0
    mShaderProgram.setUniformValue("bump", 1);

    triangles->bind(0); // texture unit 0
    bump->bind(1);

    glDrawArrays(GL_TRIANGLES, 0, 3 * 4);

    mVAO.release(); // glBindVertexArray(0)
    mShaderProgram.release(); // glUseProgram(0)
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
    phi += delta.x() * 0.01;
    theta = (theta + delta.y() * 0.01);
}

void Scene::prepareShaderProgram()
{
    if (!mShaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/phong.vert"))
        qCritical() << "error";

    if (!mShaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/phong.frag"))
        qCritical() << "error";

    if (!mShaderProgram.link())
        qCritical() << "error";

    if (!mShaderProgramLight.addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/light.vert"))
        qCritical() << "error";

    if (!mShaderProgramLight.addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/light.frag"))
        qCritical() << "error";

    if (!mShaderProgram.link())
        qCritical() << "error";


    glCheckError();
}

void Scene::prepareVertexBuffers()
{
    QVector3D a, b, c, d;
    QVector2D w, x, y, z;

    a = {0,0,0}, b = {0.5, 0.5, 1}, c = {0, 1, 0}, d = {1, 0, 0};

    QVector3D positionData[][3] = {
        {b,c,d}, {a,d,c}, {a,b,d}, {a,c,b}
    };

    QVector3D normalsData[4][3];
    for(int i = 0; i < 4; i++)
        normalsData[i][0] = normalsData[i][1] = normalsData[i][2] = QVector3D::crossProduct(positionData[i][2] - positionData[i][0], positionData[i][1] - positionData[i][0]).normalized();

    w = {0,0}, x = {1,0}, y = {0,1}, z = {1,1};

    QVector2D vtxCoordData[][3] = {
        {w, x, y}, {x, z, y}, {w, x, y}, {w, x, y}
    };

    a = vColor(Qt::red), b = vColor(Qt::yellow), c = vColor(Qt::green), d = vColor(Qt::blue);

    QVector3D colorData[][3] = {
        {b,c,d}, {a,d,c}, {a,b,d}, {a,c,b}
    };

    mVAO.create(); // glGenVertexArrays(1, &vao)
    mVAO.bind(); // glBindVertexArray(vao)

    mVertexPositionBuffer.create();
    mVertexPositionBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    mVertexPositionBuffer.bind();
    mVertexPositionBuffer.allocate(positionData, 3 * 4 * 3 * sizeof(float));

    mVertexColorBuffer.create();
    mVertexColorBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    mVertexColorBuffer.bind();
    mVertexColorBuffer.allocate(colorData, 3 * 4 * 3 * sizeof(float));

    mVertexCoordBuffer.create();
    mVertexCoordBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    mVertexCoordBuffer.bind();
    mVertexCoordBuffer.allocate(vtxCoordData, sizeof(vtxCoordData));

    mVertexNormalBuffer.create();
    mVertexNormalBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    mVertexNormalBuffer.bind();
    mVertexNormalBuffer.allocate(normalsData, sizeof(normalsData));

    mShaderProgram.bind();

    mVertexPositionBuffer.bind();
    mShaderProgram.enableAttributeArray("vertexPosition");
    mShaderProgram.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3);

    mVertexNormalBuffer.bind();
    mShaderProgram.enableAttributeArray("vertexNormal"); // glEnableVertexAttribArray
    mShaderProgram.setAttributeBuffer("vertexNormal", GL_FLOAT, 0, 3); // glVertexAttribPointer

    mVertexColorBuffer.bind();
    mShaderProgram.enableAttributeArray("vertexColor");
    mShaderProgram.setAttributeBuffer("vertexColor", GL_FLOAT, 0, 3);

    mVertexCoordBuffer.bind();
    mShaderProgram.enableAttributeArray("vertexCoord");
    mShaderProgram.setAttributeBuffer("vertexCoord", GL_FLOAT, 0, 2);

    glCheckError();
}
