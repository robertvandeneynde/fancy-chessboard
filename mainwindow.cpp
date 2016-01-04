#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "utils.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QSlider* sliders[] = {ui->onGround, ui->toUp, ui->lightSpeed};
    FormatLabel* labels[] = {ui->onGroundLabel, ui->toUpLabel, ui->lightSpeedLabel};

    for(FormatLabel* label : labels)
        label->saveFormat();

    // link ui and scene
    scene()->setLightSpeed(ui->lightSpeed->value() / 60.0);
    connect(ui->lightSpeed, &QSlider::valueChanged, [this](int x){
        scene()->setLightSpeed(x / 60.0);
    });

    scene()->setAngleFromUp(radians(ui->toUp->value()));
    connect(ui->toUp, &QSlider::valueChanged, [this](int x) {
        scene()->setAngleFromUp(radians(x));
    });

    scene()->setAngleOnGround(radians(ui->onGround->value()));
    connect(ui->onGround, &QSlider::valueChanged, [this](int x) {
        scene()->setAngleOnGround(radians(x));
    });

    auto it = labels;
    for(QSlider* slider : sliders) {
        FormatLabel* label = *it++;
        label->formatInt(slider->value());
        connect(slider, &QSlider::valueChanged, label, &FormatLabel::formatInt);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

Scene *MainWindow::scene() {
    return ui->gl->scene();
}
