#ifndef WINDOW_H
#define WINDOW_H

#include "scene.h"

#include <QWindow>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QtGui>

// My version beign 5.2, there is no QOpenGLWidget
#include <QGLWidget>

class QOpenGLContext;

class Window : public QWindow
{
    Q_OBJECT
public:
    explicit Window(QScreen *screen = 0);
    ~Window();
    
signals:
    
public slots:

protected slots:
    void resizeGl();
    void paintGl();
    void updateScene();

protected:
    void mouseMoveEvent(QMouseEvent * ev) override;
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;
    void keyPressEvent(QKeyEvent *ev) override;
    void wheelEvent(QWheelEvent *ev) override;

private:
    void initializeGl();
    void printContextInfos();

    QOpenGLContext *mContext;
    QScopedPointer<Scene> mScene;
    int tick = 0;
    QPointF lastPos;
};

class MyGLDrawer : public QGLWidget
{
    Q_OBJECT

public:
    MyGLDrawer(QWidget *parent = nullptr);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void printContextInfos();

protected slots:
    void updateScene();

protected:
    void mouseMoveEvent(QMouseEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void keyPressEvent(QKeyEvent *) override;
    void wheelEvent(QWheelEvent *) override;

    void paintEvent(QPaintEvent *) override;

public:
    QScopedPointer<Scene> mScene;
    int tick = 0;
    QPointF lastPos;
};

#endif // WINDOW_H
