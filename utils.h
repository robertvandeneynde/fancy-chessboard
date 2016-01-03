#ifndef UTILS_H
#define UTILS_H

#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QColor>
#include <QVector>

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
 * @brief return the unit vector in spherical coordinates
 * let v = spherical(fromUp, onGround)
 * let p = projection of v on xy
 * @param fromUp = angle between v and z. 0 means the +z vector
 * @param onGround = angle between x and p. 0 means on xz plane.
 * @return the unit vector of angles fromUp, onGround
 */
QVector3D spherical(float fromUp, float onGround);

/**
 * @brief spherical(length, a, b) = length * spherical(a, b)
 * @see spherical(float fromUp, float onGround)
 */
QVector3D spherical(float length, float fromUp, float onGround);

float radians(float degrees);
float degrees(float radians);

/**
 * @brief to3D(v,z) = (v.x, v.y, z)
 * @return (v.x, v.y, z)
 */
QVector3D to3D(QVector2D v, float z);

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
 * @brief 36 letters of 15-segment led display, 26 letters then 10 digits
 * @return 36 letters of 1 x 1 size, letter is a list of QVector2D to give to GL_LINES
 */
QVector< QVector<QVector2D> > makeLetters();


/**
 * @return (1 + sin(x)) / 2
 */
double positiveSin(double x);

/**
 * @return x % 1 = x - floor(x)
 */
double frac(double x);

/**
 * @brief continuous, 0 iff x is integer, up down linearly between 0 and 1, 1 iff x % 1 == 0.5
 */
double fracC(double x);

/**
 * @brief angle continuous, 0 iff x is integer, turn
 */
double linearAngle(double x);

/**
 * @brief continuous, 0 iff x is integer, up down like abs sinus
 */
double absSinC(double x);

/**
 * @brief continuous, 0 iff x is integer, up down like sinus, continuous derivative
 */
double sinC(double x);

#endif // UTILS_H
