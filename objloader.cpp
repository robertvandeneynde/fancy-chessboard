#include "objloader.h"

#include "utils.h"

#include <QFile>
#include <QDebug>
#include <QRegularExpression>

OBJObject::OBJObject()
    : bufferVertices(QOpenGLBuffer::VertexBuffer)
    , bufferTriangles(QOpenGLBuffer::IndexBuffer)
    , bufferQuads(QOpenGLBuffer::IndexBuffer)
{

}

QVector3D OBJObject::boundingBoxMin() {
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

QVector3D OBJObject::boundingBoxMax() {
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

void OBJObject::loadBuffers() {
    if(vertices.size()){
        auto& buf = bufferVertices;
        buf.create();
        buf.setUsagePattern(QOpenGLBuffer::StaticDraw);
        buf.bind();
        buf.allocate(vertices.data(), vertices.size() * sizeof(QVector3D));
    }

    if(triangles.size()) {
        auto& buf = bufferTriangles;
        buf.create();
        buf.setUsagePattern(QOpenGLBuffer::StaticDraw);
        buf.bind();
        buf.allocate(triangles.data(), triangles.size() * sizeof(GLuint));
    }

    if(quads.size()) {
        auto& buf = bufferQuads;
        buf.create();
        buf.setUsagePattern(QOpenGLBuffer::StaticDraw);
        buf.bind();
        buf.allocate(quads.data(), quads.size() * sizeof(GLuint));
    }

    if(normals.size()) {
        auto& buf = bufferNormals;
        buf.create();
        buf.setUsagePattern(QOpenGLBuffer::StaticDraw);
        buf.bind();
        buf.allocate(normals.data(), normals.size() * sizeof(QVector3D));
    }

    if(texCoord.size()) {
        auto& buf = bufferTexCoord;
        buf.create();
        buf.setUsagePattern(QOpenGLBuffer::StaticDraw);
        buf.bind();
        buf.allocate(texCoord.data(), texCoord.size() * sizeof(QVector2D));
    }
}

void OBJObject::draw() {
    if(triangles.length()) {
        bufferTriangles.bind();
        glDrawElements(GL_TRIANGLES, 3 * triangles.size(), GL_UNSIGNED_INT, 0);
    }
    if(quads.length()) {
        bufferQuads.bind();
        glDrawElements(GL_QUADS, 4 * quads.size(), GL_UNSIGNED_INT, 0);
    }
}


void OBJLoader::load(QString filename)
{
    OBJObject* object = objects[""] = new OBJObject();

    QFile file(filename);
    if(! file.open(QIODevice::ReadOnly))
        qCritical() << "Error loading " << filename;
    QTextStream in(&file);
    while(! in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(QRegularExpression("\\s+"), QString::SkipEmptyParts);

        bool skipped = true;
        if(parts.isEmpty()) {

        }
        else if(parts[0] == "v" || parts[0] == "vn" || parts[0] == "vt") {
            int M = parts[0] == "vt" ? 2 : 3;
            if(parts.length() == M+1) {
                bool ok = true;

                QVector3D vec;
                for(int i = 0; i < M; i++) {
                    bool o;
                    vec[i] = parts[i+1].toFloat(&o);
                    ok &= o;
                }

                if(ok) {
                    if(parts[0] == "v")
                        object->vertices.push_back(vec);
                    else if(parts[0] == "vn")
                        object->normals.push_back(vec);
                    else
                        object->texCoord.push_back({vec.x(), vec.y()});
                    skipped = false;
                }
            }
            /*
             * if parts[0] == "v":
             *  Colors parts.length() == 7 : color3f
             *  Colors parts.length() == 8 : color4f
             */
        } else if(parts[0] == "g" || parts[0] == "o") {
            if(parts.length() == 2) {
                QString name = parts[1]; // it is not whitespace
                int n = 0;
                while(objects.contains(name))
                    name = QString("%1_%2").arg(parts[1]).arg(++n);

                if(n != 0)
                    qWarning() << n << "th" << parts[1] << " available as " << name;
                object = objects[name] = new OBJObject();
                skipped = false;
            }
        } else if(parts[0] == "f") {
            // parts = f 1/2/3 4/5/6 7/8/9 ... | f 1//3 4//6 7//9 ... | f 1/2 4/5 7/8 | f 1 4 7
            const QRegularExpression
                    R1("^f\\s+((-?\\d+)/(-?\\d+)/(-?\\d+)){3,}\\s.*$"),
                    R2("^f\\s+((-?\\d+)//(-?\\d+)){3,}\\s.*$"),
                    R3("^f\\s+((-?\\d+)/(-?\\d+)){3,}\\s.*$"),
                    R4("^f\\s+((-?\\d+)){3,}\\s.*$");

            if(!(R1.match(line).isValid() || R2.match(line).isValid() || R3.match(line).isValid() || R4.match(line).isValid()))
                qWarning() << "Wrong format " << line << endl;

            if(1) {
                QVector<QStringList> sp(parts.length() - 1);
                for(int i = 0; i < parts.length() - 1; i++)
                    sp[i] = parts[i+1].split('/', QString::SkipEmptyParts);

                if(sp.length()) {
                    int L = sp[0].length();

                    for_all(bool ok,
                            sp[i].length() == L,
                            int i = 0; i < sp.length(); i++)

                    if(ok) {
                        ok = true;
                        QVector<int> vtx(sp.length());
                        for(int i = 0; i < sp.length(); i++) {
                            bool o;
                            vtx[i] = sp[i][0].toInt(&o);
                            ok &= o;
                        }

                        if(ok) {
                            ok = true;
                            for(int& x : vtx) {
                                x += x < 0 ? object->vertices.size() : -1; // if positive, begins at 1, if negative N + x
                                if(!(0 <= x && x < object->vertices.size()))
                                    ok = false;
                            }
                            if(ok) {
                                if(vtx.length() == 3) {
                                    // triangle
                                    for(int& x : vtx)
                                        object->triangles.append(x);
                                    skipped = false;
                                } else if(vtx.length() == 4) {
                                    // quad
                                    for(int& x : vtx)
                                        object->quads.append(x);
                                    skipped = false;
                                }
                            }
                        }
                    }
                }
            }
        }

        if(skipped && parts.length()) {
            qDebug() << "Skipping " << line;
        }
    }

    if(objects[""]->vertices.isEmpty()) {
        delete objects[""];
        objects.remove("");
    }

    onloaded();
}
