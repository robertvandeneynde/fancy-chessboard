#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QSlider>

#include "customwidgets.h"

#include "scene.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QWidget* modal = nullptr;

    QList<int> defaultSliderValues;
    QList<QSlider*> sliders;
    QList<FormatLabel*> labels;

private:
    Scene* getScene();
};

#endif // MAINWINDOW_H
