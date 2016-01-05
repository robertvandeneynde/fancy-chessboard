#ifndef OBJLOADER_H
#define OBJLOADER_H

#include "GL/gl.h"

#include <QVector>
#include <QVector2D>
#include <QVector3D>
#include <QMap>
#include <QString>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include <algorithm>

struct OBJObject {
    QVector<QVector3D> vertices; // any size
    QVector<QVector3D> normals;
    QVector<QVector2D> texCoord;
    QVector<GLuint> triangles; // triangles.size() % 3 == 0
    QVector<GLuint> quads; // quads.size() % 4 == 0

    QOpenGLBuffer bufferVertices;
    QOpenGLBuffer bufferNormals;
    QOpenGLBuffer bufferTexCoord;
    QOpenGLBuffer bufferTriangles;
    QOpenGLBuffer bufferQuads;

public:
    OBJObject();

    struct Geometry {
        QVector3D min, max;
        QVector3D center, size;
    } geom;

    void calculateGeometry() {
        geom.min = boundingBoxMin();
        geom.max = boundingBoxMax();
        geom.center = 0.5 * (geom.max + geom.min);
        geom.size = geom.max - geom.min;
    }

private:
    QVector3D boundingBoxMin();
    QVector3D boundingBoxMax();

public:
    void loadBuffers();
    void draw();
};

// only work with (1 g, 2 ... n with negative)
struct OBJLoader {
    QMap<QString, OBJObject*> objects;

    void load(QString filename);
    void createBuffers();
    virtual void onloaded() {}
};

#endif // OBJLOADER_H
