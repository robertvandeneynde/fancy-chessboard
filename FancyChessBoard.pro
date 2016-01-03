#-------------------------------------------------
#
# Project created by QtCreator 2012-12-26T06:24:06
#
#-------------------------------------------------

QT       += core gui

# Because Qt 5.2 has no QOpenGLWidget
QT += opengl

CONFIG += c++11

# Qt5
QT += widgets

TARGET = FancyChessBoard
TEMPLATE = app


SOURCES += main.cpp \
    window.cpp \
    utils.cpp \
    scene.cpp

HEADERS += \
    window.h \
    glassert.h \
    utils.h \
    scene.h

OTHER_FILES += \
    shaders/* \
    images/* \
    shaders/light.frag \
    shaders/light.vert

RESOURCES += \
    resources.qrc
