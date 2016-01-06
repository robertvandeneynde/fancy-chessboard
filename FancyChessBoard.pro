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
    utils.cpp \
    scene.cpp \
    glwidget.cpp \
    mainwindow.cpp \
    objloader.cpp \
    customwidgets.cpp

HEADERS += \
    glassert.h \
    utils.h \
    scene.h \
    glwidget.h \
    mainwindow.h \
    objloader.h \
    customwidgets.h

OTHER_FILES += \
    shaders/* \
    images/* \
    shaders/light.frag \
    shaders/light.vert \
    shaders/chess.frag \
    shaders/chess.vert \
    shaders/board.frag \
    shaders/board.vert

# RESOURCES += \
#     resources.qrc

FORMS += \
    mainwindow.ui

QMAKE_CXXFLAGS += \
    -Wno-unused-variable \
    -Wno-unused-parameter

