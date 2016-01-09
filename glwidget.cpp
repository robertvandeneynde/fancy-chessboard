#include "glwidget.h"

#include "scene.h"

#include <iostream>

#include <QOpenGLContext>
#include <QTimer>

#include <QOpenGLPaintDevice>
#include <QPainter>

MyGLDrawer::MyGLDrawer(QWidget *parent)
    : QGLWidget(parent),
      scene(new Scene())
{
    QGLFormat format;
    format.setDepthBufferSize(24);
    // format.setMajorVersion(3);
    // format.setMinorVersion(0);
    format.setSamples(4);
    // format.setProfile(QSurfaceFormat::CoreProfile);

    context()->setFormat(format);
    context()->create();

    printContextInfos();
    resize(1000, 700);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateScene()));
    timer->start(20);

    setAutoFillBackground(false);
}

void MyGLDrawer::infoGL()
{
    Scene::glCheckError();

    std::cout
        << "OpenGL infos with gl functions" << std::endl
        << "Renderer : " << glGetString(GL_RENDERER) << std::endl
        << "Vendor : " << glGetString(GL_VENDOR) << std::endl
        << "OpenGL Version : " << glGetString(GL_VERSION) << std::endl
        << "GLSL Version : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    Scene::glCheckError();
}

void MyGLDrawer::printContextInfos() {
    QGLContext* mContext = context();

    if(!mContext->isValid())
        std::cerr << "The OpenGL context is invalid!" << std::endl;

    std::cout << "Window format version is: "
              << format().majorVersion() << "."
              << format().minorVersion() << std::endl;

    std::cout << "Context format version is: "
              << mContext->format().majorVersion()
              << "." << mContext->format().minorVersion() << std::endl;
    makeCurrent();
    infoGL();
}

void MyGLDrawer::initializeGL() {
    context()->makeCurrent();
    scene->initialize();
}

void MyGLDrawer::paintGL() {
    scene->render();
}

void MyGLDrawer::paintEvent(QPaintEvent * ev) {
    QGLWidget::paintEvent(ev);
}

void MyGLDrawer::resizeGL(int w, int h) {
    context()->makeCurrent();
    scene->resize(w, h);
}

float MyGLDrawer::currentTime() {
    return tick * 20.0 / 1000.0;
}

void MyGLDrawer::updateScene()
{
    scene->update(currentTime());
    tick++;
    updateGL();
}

void MyGLDrawer::mouseMoveEvent(QMouseEvent *ev) {
    if(ev->buttons() & Qt::LeftButton) {
        QPointF delta = ev->pos() - lastPosL;
        scene->applyDelta(delta);
        lastPosL = ev->pos();
    }

    if(ev->buttons() & Qt::RightButton) {
        QPointF delta = ev->pos() - lastPosR;
        scene->applyMove(delta);
        lastPosR = ev->pos();
    }


    if(ev->buttons() & Qt::MiddleButton) {
        /*
        float delta = ev->pos().y() - lastPosM.y();
        scene->applyMoveForward(delta);
        lastPosM = ev->pos();
        */
    }

    updateGL();
}

void MyGLDrawer::mousePressEvent(QMouseEvent *ev) {
    if(ev->button() == Qt::LeftButton)
        lastPosL = ev->pos();
    if(ev->button() == Qt::RightButton)
        lastPosR = ev->pos();
    if(ev->button() == Qt::MiddleButton)
        lastPosM = ev->pos();
    updateGL();
}

void MyGLDrawer::mouseReleaseEvent(QMouseEvent *ev) {
    emit paramChanged();
}

void MyGLDrawer::keyPressEvent(QKeyEvent *ev) {

}

void MyGLDrawer::wheelEvent(QWheelEvent * ev) {
    scene->applyZoom(ev->delta() / 120.0);
    emit paramChanged();
}
