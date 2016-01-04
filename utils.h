#ifndef UTILS_H
#define UTILS_H

#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QColor>
#include <QVector>

#include <algorithm>

float radians(float degrees);
float degrees(float radians);

/**
 * @brief polar(a) = (cos(a), sin(a))
 * @return (cos(a), sin(a))
 */
QVector2D polar(float angleInRadians);

/**
 * @brief polar(l,a) = (l * cos(a), l * sin(a))
 * @return (l * cos(a), l * sin(a))
 */
QVector2D polar(float length, float angleInRadians);

/**
 * @return atan2(v.y, v.x)
 */
float angle2D(QVector2D v);

/**
 * @brief return the unit vector in spherical coordinates
 * let v = spherical(fromUp, onGround)
 * let p = projection of v on xy
 *
 * @param fromUp = angle between v and z. 0 means the +z vector
 * @param onGround = angle between x and p. 0 means on xz plane.
 * @return the unit vector of angles fromUp, onGround
 *
 * given definitions on https://en.wikipedia.org/wiki/Spherical_coordinate_system
 * fromUp   = inclinaison angle = θ-physics = φ-maths
 * onGround = azimutal angle    = φ-physics = θ-maths
 */
QVector3D spherical(float fromUp, float onGround);

/**
 * @brief spherical(length, a, b) = length * spherical(a, b)
 * @see spherical(float fromUp, float onGround)
 */
QVector3D spherical(float length, float fromUp, float onGround);

/**
 * @return (x, y, z)
 */
QVector3D vec3(float x, float y, float z);

/**
 * @brief vec3(v,z) = (v.x, v.y, z)
 * @return (v.x, v.y, z)
 */
QVector3D vec3(QVector2D v, float z);

/**
 * @brief to2DYUp(v,y) = (v.x, z, v.y)
 * @return (v.x, z, v.y)
 */
QVector3D to3DYUp(QVector2D v, float y);

/**
 * @brief vColor(c) = c.rgb
 * @return c.rgb
 */
QVector3D vColor(QColor c);

/**
 * @brief vColor(c) = c.rgb
 * @return c.rgb
 */
QVector4D vColorA(QColor c);

/**
 * @brief cross product
 */
QVector3D cross(QVector3D a, QVector3D b);

/**
 * @brief 36 letters of 15-segment led display, 26 letters then 10 digits
 * @return 36 letters of 1 x 1 size, letter is a list of QVector2D to give to GL_LINES
 */
QVector<QVector<QVector2D>> makeLetters();


/**
 * @return (1 + sin(x)) / 2
 */
double positiveSin(double x);

/**
 * @return x % 1 = x - floor(x)
 */
double frac(double x);

/**
 * @brief continuous, to [0,1], 0 iff x is integer, up down linearly between 0 and 1, 1 iff x % 1 == 0.5
 */
double fracC(double x);

/**
 * @brief angle continuous, 0 iff x is integer, turn
 */
double linearAngle(double x);

/**
 * @brief continuous, to [0,1], 0 iff x is integer, up down like abs sinus
 */
double absSinC(double x);

/**
 * @brief continuous, 0 iff x is integer, up down like sinus, continuous derivative
 */
double sinC(double x);

template <typename T>
T clamp(T x, T m, T M) {
    return std::min(std::max(x, m), M);
}

template <typename T>
T clamp(T x, std::pair<T,T> interv) {
    return clamp(x, interv.first, interv.second);
}

#endif // UTILS_H
