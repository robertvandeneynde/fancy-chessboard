#ifndef UTILS_H
#define UTILS_H

#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>
#include <QColor>
#include <QVector>

#include <algorithm>
#include <cmath>

#define loop_all(boolvari, cond, loopexpr) boolvari = true; loop_expr if(!(cond)) { boolvari = false; break; }
#define loop_any(boolvari, cond, loopexpr) boolvari = false; loop_expr if(cond) { boolvari = true; break; }

#define for_all(boolvari, cond, forexpr) boolvari = true; for(forexpr) if(!(cond)) { boolvari = false; break; }
#define for_any(boolvari, cond, forexpr) boolvari = false; for(forexpr) if(cond) { boolvari = true; break; }
#define for_sum(retvalue, expr, forexpr) retvalue = 0; for(forexpr) retvalue += (expr);
#define for_create(vec, expr, forexpr) vec.clear(); for(forexpr) vec.push_pack(expr);

// Examples
// for_all(bool even, x % 2 == 0, int i = 0; i < 10; i += 2)
// for_all(even, x % 2 == 0, int i = 0; i < 10; i += 2) // if 'even' was already declared
// for_all(even, x % 2 == 0, int x : list)
// loop_all(eve, x % 4 == 0, for(int x : list) if(i % 2 == 0))

// int x; for_sum(x, i, int i = 0; i < 5; i++)
// vector<int> vec; for_create(vec, x+1, int x : list)

#ifndef M_PI
#define M_PI 3.14159265358979323846 # gcc constant
#endif

#define M_TAU (2 * M_PI) // http://tauday.com/tau-manifesto
#define M_2PI (2 * M_PI)
#define M_TWO_PI (2 * M_PI)

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
 * @return ((float)p.x, (float)p.y)
 */
QVector2D vec2(QPoint p);

/**
 * @return (p.x, p.y)
 */
QVector2D vec2(QPointF p);

/**
 * @brief vColor(c) = c.rgb
 * @return c.rgb
 */
QVector3D vColor(QColor c);
QColor color3(QVector3D);

/**
 * @brief vColor(c) = c.rgb
 * @return c.rgb
 */
QVector4D vColorA(QColor c);
QColor color4(QVector4D);

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
 * @see <a href="http://fooplot.com/plot/ly3w7qgz5d">plot</a>
 */
double frac(double x);

/**
 * @brief continuous, to [0,1], 0 iff x is integer, up down linearly between 0 and 1, 1 iff x % 1 == 0.5
 * @see <a href="http://fooplot.com/plot/23mvx3qsbl">plot</a>
 */
double fracC(double x);

/**
 * @brief angle continuous, 0 iff x is integer, turn
 * @see <a href="http://fooplot.com/plot/9uhhtr7bur">plot</a>
 */
double linearAngle(double x);

/**
 * @brief continuous, to [0,1], 0 iff x is integer, up down like abs sinus
 * @see <a href="http://fooplot.com/plot/iitt5z8fkb">plot</a>
 */
double absSinC(double x);

/**
 * @brief continuous, 0 iff x is integer, up down like sinus, continuous derivative
 * @see <a href="http://fooplot.com/plot/nbb07kk23u">plot<a>
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

struct Matrix : QMatrix4x4 {

    Matrix& translate(float x, float y)                           { QMatrix4x4::translate(x,y);         return *this; }
    Matrix& translate(float x, float y, float z)                  { QMatrix4x4::translate(x,y,z);       return *this; }
    Matrix& translate(QVector2D v)                                { QMatrix4x4::translate(v.x(),v.y()); return *this; }
    Matrix& translate(QVector3D v)                                { QMatrix4x4::translate(v);           return *this; }
    Matrix& rotate(float angle, float x, float y, float z = 0.0f) { QMatrix4x4::rotate(angle,x,y,z);    return *this; }
    Matrix& rotate(float angle, QVector3D axis = {0,0,1})         { QMatrix4x4::rotate(angle,axis);     return *this; }
    Matrix& rotate(QQuaternion q)                                 { QMatrix4x4::rotate(q);              return *this; }
    Matrix& scale(float r)                                        { QMatrix4x4::scale(r);               return *this; }
    Matrix& scale(float x, float y)                               { QMatrix4x4::scale(x,y);             return *this; }
    Matrix& scale(float x, float y, float z)                      { QMatrix4x4::scale(x,y,z);           return *this; }
    Matrix& scale(QVector3D v)                                    { QMatrix4x4::scale(v);               return *this; }

    Matrix translated(float x, float y) const                           { return copy().translate(x,y);         }
    Matrix translated(float x, float y, float z) const                  { return copy().translate(x,y,z);       }
    Matrix translated(QVector2D v) const                                { return copy().translate(v.x(),v.y()); }
    Matrix translated(QVector3D v) const                                { return copy().translate(v);           }
    Matrix rotated(float angle, float x, float y, float z = 0.0f) const { return copy().rotate(angle,x,y,z);    }
    Matrix rotated(float angle, QVector3D axis = {0,0,1}) const         { return copy().rotate(angle,axis);     }
    Matrix rotated(QQuaternion q) const                                 { return copy().rotate(q);              }
    Matrix scaled(float r) const                                        { return copy().scale(r);               }
    Matrix scaled(float x, float y) const                               { return copy().scale(x,y);             }
    Matrix scaled(float x, float y, float z) const                      { return copy().scale(x,y,z);           }
    Matrix scaled(QVector3D v) const                                    { return copy().scale(v);               }

    Matrix copy() const { return *this; }
};

#endif // UTILS_H
