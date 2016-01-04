#include "objloader.h"

#include <QFile>
#include <QDebug>
#include <QRegularExpression>

OBJObject::OBJObject()
    : bufferVertices(QOpenGLBuffer::VertexBuffer)
    , bufferTriangles(QOpenGLBuffer::IndexBuffer)
    , bufferQuads(QOpenGLBuffer::IndexBuffer)
{

}

void OBJObject::loadBuffers() {
    {
        auto& buf = bufferVertices;
        buf.create();
        buf.setUsagePattern(QOpenGLBuffer::StaticDraw);
        buf.bind();
        buf.allocate(vertices.data(), vertices.size() * sizeof(QVector3D));
    }

    {
        auto& buf = bufferTriangles;
        buf.create();
        buf.setUsagePattern(QOpenGLBuffer::StaticDraw);
        buf.bind();
        buf.allocate(triangles.data(), triangles.size() * sizeof(GLuint));
    }
    {
        auto& buf = bufferQuads;
        buf.create();
        buf.setUsagePattern(QOpenGLBuffer::StaticDraw);
        buf.bind();
        buf.allocate(quads.data(), quads.size() * sizeof(GLuint));
    }
}

void OBJObject::draw() {
    bufferTriangles.bind();
    glDrawElements(GL_TRIANGLES, 3 * triangles.size(), GL_UNSIGNED_INT, 0);
    bufferQuads.bind();
    glDrawElements(GL_QUADS, 4 * quads.size(), GL_UNSIGNED_INT, 0);
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
        else if(parts[0] == "v") {
            if(parts.length() == 4) {
                bool ok = true;

                QVector3D vec;
                for(int i = 0; i < 3; i++) {
                    bool o;
                    vec[i] = parts[i+1].toFloat(&o);
                    ok &= o;
                }

                if(ok) {
                    object->vertices.push_back(vec);
                    skipped = false;
                }
            }
            else if(parts.length() == 7) {
                // with color3f
            } else if(parts.length() == 8) {
                // with color4f
            }
        } else if(parts[0] == "vn") {

        } else if(parts[0] == "vt") {

        } else if(parts[0] == "g") {
            if(parts.length() == 2) {
                QString name = parts[1]; // it is not whitespace
                int n = 0;
                while(objects.contains(name)) {
                    ++n;
                    name = parts[1] + "_" + QString::number(n);
                }
                if(n != 0)
                    qWarning() << (n) << "th" << parts[1] << " available as " << name;
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

                    bool ok = true;
                    for(int i = 0; i < sp.length(); i++) {
                        int l = sp[i].length();
                        if(sp[i].length() != L)
                            ok = false;
                    }

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

        if(skipped && parts.length() && (parts[0] == "f" || parts[1] == "f")) {
            qDebug() << "Skipping " << line;
        }
    }

    if(objects[""]->vertices.isEmpty()) {
        delete objects[""];
        objects.remove("");
    }
}
