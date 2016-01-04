#ifndef OBJLOADER_H
#define OBJLOADER_H

#include "GL/gl.h"

#include <QVector>
#include <QVector3D>
#include <QMap>
#include <QString>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include <algorithm>

struct OBJObject {
    QVector<QVector3D> vertices; // any size
    QVector<GLuint> triangles; // triangles.size() % 3 == 0
    QVector<GLuint> quads; // quads.size() % 4 == 0

    QOpenGLVertexArrayObject VAO;

    QOpenGLBuffer bufferVertices;
    QOpenGLBuffer bufferTriangles;
    QOpenGLBuffer bufferQuads;

public:
    OBJObject();

    QVector3D boundingBoxMin() {
        if(vertices.isEmpty())
            return {};
        QVector3D m = vertices.front();
        for(QVector3D const& v : vertices) {
            m[0] = std::min(m[0], v[0]);
            m[1] = std::min(m[1], v[1]);
            m[2] = std::min(m[2], v[2]);
        }
        return m;
    }

    QVector3D boundingBoxMax() {
        if(vertices.isEmpty())
            return {};
        QVector3D m = vertices.front();
        for(QVector3D const& v : vertices) {
            m[0] = std::max(m[0], v[0]);
            m[1] = std::max(m[1], v[1]);
            m[2] = std::max(m[2], v[2]);
        }
        return m;
    }

    /**
     * @brief loadBuffers
     * must be in vao
     */
    void loadBuffers();

    /**
     * @brief call bind and glDrawElements
     */
    void draw();
};

struct OBJLoader {
    QMap<QString, OBJObject*> objects;

    void load(QString filename);
    void createBuffers();
};

#endif // OBJLOADER_H
