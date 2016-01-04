#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "utils.h"

#include <stdexcept>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QVector<QSlider*> sliders;
    QVector<FormatLabel*> labels;

    for(QObject* w : ui->sliders->children()) {
        sliders.push_back(dynamic_cast<QSlider*>(w));
        labels.push_back(dynamic_cast<FormatLabel*>(w));
        if(! sliders.back())
            sliders.pop_back();
        if(! labels.back())
            labels.pop_back();
    }

    for(FormatLabel* label : labels)
        label->saveFormat();

    if(sliders.size() != labels.size())
        throw std::exception();

    // link ui and scene
    auto map = [](float & value, QSlider* slider, float constant) {
        value = slider->value() * constant;
        connect(slider, &QSlider::valueChanged, [&value, constant](int x){
            value = x * constant;
        });
    };

    const float RPM = 1 / 60.0,
                ANGLE = radians(1),
                CM = 1/100.0;

    Scene* scene = getScene();
    map(scene->lightHeight, ui->lightHeight, CM);
    map(scene->lightRadius, ui->lightRadius, CM);
    map(scene->lightSpeed, ui->lightSpeed, RPM);
    map(scene->angleFromUp, ui->toUp, ANGLE);
    map(scene->angleOnGround, ui->onGround, ANGLE);
    map(scene->lightInitPos, ui->lightInitPos, ANGLE);
    map(scene->length, ui->cameraR, CM);

    auto it = labels.begin();
    for(QSlider* slider : sliders) {
        FormatLabel* label = *it++;
        label->formatInt(slider->value());
        connect(slider, &QSlider::valueChanged, label, &FormatLabel::formatInt);
    }

    connect(ui->actionShow_help, &QAction::triggered, [](){
        QMessageBox d;
        d.setText("Hello");
        d.exec();
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

Scene *MainWindow::getScene() {
    return ui->gl->getScene();
}
