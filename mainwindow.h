#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QSlider>
#include <QString>

#include "customwidgets.h"

#include "scene.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    typedef FormatLabel MyLabel;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QWidget* modal = nullptr;

    QList<int> defaultSliderValues;
    QList<QSlider*> sliders;
    QList<MyLabel*> labels;

private:
    Scene* getScene();
};

#endif // MAINWINDOW_H
