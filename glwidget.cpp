#include "glwidget.h"

#include "scene.h"
#include "glassert.h"

#include <iostream>

#include <QOpenGLContext>
#include <QTimer>

#include <QOpenGLPaintDevice>
#include <QPainter>

static void infoGL()
{
    glCheckError();

    std::cout
        << "OpenGL infos with gl functions" << std::endl
        << "Renderer : " << glGetString(GL_RENDERER) << std::endl
        << "Vendor : " << glGetString(GL_VENDOR) << std::endl
        << "OpenGL Version : " << glGetString(GL_VERSION) << std::endl
        << "GLSL Version : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    glCheckError();
}

Window::Window(QScreen *screen) :
    QWindow(screen),
    mScene(new Scene())
{
    setSurfaceType(OpenGLSurface);

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setMajorVersion(3);
    format.setMinorVersion(0);
    format.setSamples(4);
    format.setProfile(QSurfaceFormat::CoreProfile);

    setFormat(format);
    create();

    mContext = new QOpenGLContext();
    mContext->setFormat(format);
    mContext->create();

    printContextInfos();
    initializeGl();

    resize(800, 450);

    connect(this, SIGNAL(widthChanged(int)), this, SLOT(resizeGl()));
    connect(this, SIGNAL(heightChanged(int)), this, SLOT(resizeGl()));

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateScene()));
    timer->start(20);
}

MyGLDrawer::MyGLDrawer(QWidget *parent)
    : QGLWidget(parent),
      mScene(new Scene())
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
    resize(800, 450);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateScene()));
    timer->start(20);

    setAutoFillBackground(false);
}

Window::~Window()
{
}

void Window::printContextInfos()
{
    if(!mContext->isValid())
        std::cerr << "The OpenGL context is invalid!" << std::endl;

    mContext->makeCurrent(this);

    std::cout << "Window format version is: "
              << format().majorVersion() << "."
              << format().minorVersion() << std::endl;

    std::cout << "Context format version is: "
              << mContext->format().majorVersion()
              << "." << mContext->format().minorVersion() << std::endl;
    infoGL();
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

void Window::initializeGl()
{
    mContext->makeCurrent(this);
    mScene->initialize();
}

void MyGLDrawer::initializeGL() {
    context()->makeCurrent();
    mScene->initialize();
}

void Window::paintGl()
{
    if( !isExposed() ) return;
    mContext->makeCurrent(this);
    mScene->render();

    mContext->swapBuffers(this);
}

void MyGLDrawer::paintGL() {
    mScene->render();
}

void MyGLDrawer::paintEvent(QPaintEvent * ev) {
    QGLWidget::paintEvent(ev);
}

void Window::resizeGl()
{
    mContext->makeCurrent(this);
    mScene->resize(width(), height());
}

void MyGLDrawer::resizeGL(int w, int h) {
    context()->makeCurrent();
    mScene->resize(w, h);
}

void Window::updateScene()
{
    mScene->update((tick++) * 20.0 / 1000.0);
    paintGl();
}

void MyGLDrawer::updateScene()
{
    mScene->update((tick++) * 20.0 / 1000.0);
    updateGL();
}

void Window::mouseMoveEvent(QMouseEvent *ev) {
    if(ev->buttons() & Qt::LeftButton) {
        QPointF delta = ev->pos() - lastPos;
        mScene->applyDelta(delta);
        lastPos = ev->pos();
    }
}

void MyGLDrawer::mouseMoveEvent(QMouseEvent *ev) {
    if(ev->buttons() & Qt::LeftButton) {
        QPointF delta = ev->pos() - lastPos;
        mScene->applyDelta(delta);
        lastPos = ev->pos();
    }
    updateGL();
}

void Window::mousePressEvent(QMouseEvent *ev) {
    if(ev->button() == Qt::LeftButton)
        lastPos = ev->pos();
}

void MyGLDrawer::mousePressEvent(QMouseEvent *ev) {
    if(ev->button() == Qt::LeftButton)
        lastPos = ev->pos();
    updateGL();
}


void Window::mouseReleaseEvent(QMouseEvent *ev) {

}

void MyGLDrawer::mouseReleaseEvent(QMouseEvent *ev) {

}

void Window::keyPressEvent(QKeyEvent *ev) {

}

void MyGLDrawer::keyPressEvent(QKeyEvent *ev) {

}

void Window::wheelEvent(QWheelEvent * ev) {
    mScene->applyZoom(ev->delta() / 120.0);
}

void MyGLDrawer::wheelEvent(QWheelEvent * ev) {
    mScene->applyZoom(ev->delta() / 120.0);
}
