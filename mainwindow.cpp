#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "utils.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->toUpLabel->saveFormat();
    ui->onGroundLabel->saveFormat();

    connect(ui->toUp, &QSlider::valueChanged, [this](int x) {
        ui->toUpLabel->format(x);
        scene()->setAngleFromUp(radians(x));
    });

    connect(ui->onGround, &QSlider::valueChanged, [this](int x) {
        ui->onGroundLabel->format(x);
        scene()->setAngleOnGround(radians(x));
    });

    QSlider* sliders[] = {ui->onGround, ui->toUp};
    IntFormatLabel* labels[] = {ui->onGroundLabel, ui->toUpLabel};

    for(int i = 0; i < 2; i++)
        labels[i]->format(sliders[i]->value());

    scene()->setAngles(radians(ui->toUp->value()), radians(ui->onGround->value()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

Scene *MainWindow::scene() {
    return ui->gl->scene();
}
