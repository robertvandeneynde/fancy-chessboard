#include "utils.h"

#include <cmath>
#include <QtMath>
#include <QRegularExpression>

#define M_TWO_PI 2 * M_PI

QVector2D polar(float angleInRadians) {
    return {std::cos(angleInRadians), std::sin(angleInRadians)};
}

QVector2D polar(float length, float angleInRadians) {
    return length * polar(angleInRadians);
}

QVector3D spherical(float fromUp, float onGround) {
    // fromUp is the angle between v and z. 0 means the +z vector.
    // let p = projection of v on xy plane. onGround is the angle from the x axis to p. 0 means on xz plane.
    return {std::sin(fromUp) * std::cos(onGround), std::sin(fromUp) * std::sin(onGround), std::cos(fromUp)};
}

QVector3D spherical(float length, float fromUp, float onGround) {
    return length * spherical(fromUp, onGround);
}

float radians(float degrees) {
    return qDegreesToRadians(degrees);
}

float degrees(float radians) {
    return qRadiansToDegrees(radians);
}

float angle2D(QVector2D v) {
    return std::atan2(v.y(), v.x());
}

QVector3D vec3(float x, float y, float z) {
    return {x, y, z};
}

QVector3D vec3(QVector2D v, float z) {
    return {v.x(), v.y(), z};
}

QVector3D vColor(QColor c) {
    return {c.redF(), c.greenF(), c.blueF()};
}

QVector4D vColorA(QColor c) {
    return {c.redF(), c.greenF(), c.blueF(), c.alphaF()};
}

QVector<QVector2D> fromString(QString s) {
    s = s.toLower();
    if(! QRegularExpression("\\s*[abcdefhi]+(\\s+[abcdefhi]+)*\\s*").match(s).isValid())
        throw std::domain_error("wrong string format");

    static const QVector2D bases[9] = {
        {0, 0}, {0.5, 0}, {1, 0},
        {0, 0.5}, {0.5, 0.5}, {1, 0.5},
        {0, 1}, {0.5, 1}, {1, 1}
    };

    auto toVec = [&bases](QChar c) -> QVector2D {
        return bases[c.toLatin1() - 'a'];
    };

    QVector<QVector2D> res;
    for(QString p : s.split(QRegularExpression("\\s+"), QString::SkipEmptyParts)) {
        QString::Iterator it = p.begin();
        QVector2D last = toVec(*it++);
        while(it != p.end()) {
            res.push_back(last);
            last = toVec(*it++);
            res.push_back(last);
        }
    }
    return res;
}

static const QVector<QString> lettersData = {
    "gdabcfi fed", // A
    "abcfihg beh ef", // B
    "cbadghi", // C
    "abcfihg beh", // D
    "cbadghi ed", // E
    "cbadg ed", // F
    "cbadhife", // G
    "adefc gd if", // H
    "abehg bc hi", // I
    "abehg bc", // J
    "adec dg ei", // K
    "adghi", // L
    "gdaecfi", // M
    "gdaeifc", // N
    "abcfihgd", // O
    "gdabcfi", // P
    "ifcbadghie", // Q
    "gdabcfei de", // R
    "cbadefihg", // S
    "abc beh", // T
    "adghifc", // U
    "adgec", // V
    "adgeifc", // W
    "aei gec", // X
    "aec eh", // Y
    "abceghi", // Z
    "abcfihg", // 0
    "heb", // 1
    "abcfedghi", // 2
    "abcfihg ef", // 3
    "adefi cf", // 4
    "cbadefihg", // 5
    "cbadefihg", // 6
    "abcfi ef", // 7
    "abcfihgda aei gec", // 8
    "abcfeda fihg", // 9
};

QVector<QVector<QVector2D>> makeLetters()
{
    QVector< QVector<QVector2D> > res;
    for(QString s : lettersData)
        res.push_back(fromString(s));
    return res;
}

double positiveSin(double x) {
    return (1 + std::sin(x)) / 2;
}

double frac(double x) {
    return x - std::floor(x);
}

double fracC(double x) {
    auto y = frac(x);
    return std::min(1-y, y);
}

double linearAngle(double x) {
    return frac(x) * (2 * M_PI);
}

double absSinC(double x) {
    return std::abs(std::sin(x * 2 * M_PI));
}

double sinC(double x) {
    return positiveSin(x * 2 * M_PI);
}

QVector3D cross(QVector3D a, QVector3D b) {
    return QVector3D::crossProduct(a,b);
}
