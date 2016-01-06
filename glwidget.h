#ifndef GL_WIDGET_H
#define GL_WIDGET_H

#include "scene.h"

#include <QWindow>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QtGui>

// My version beign 5.2, there is no QOpenGLWidget
#include <QGLWidget>

#include <QMainWindow>

class MyGLDrawer : public QGLWidget
{
    Q_OBJECT

public:
    MyGLDrawer(QWidget *parent = nullptr);

    Scene* getScene() { return scene.data(); }

signals:
    void paramChanged();
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

private:
    QScopedPointer<Scene> scene;
    int tick = 0;
    QPointF lastPosL, lastPosR;

    static void infoGL();
};

#endif // GL_WIDGET_H
