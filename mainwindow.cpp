#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "utils.h"

#include <stdexcept>
#include <QMessageBox>
#include <functional>

template <typename T>
static void addChildren(QList<QSlider*> & sliders, QList<T*> & rawLabels, QWidget* widget) {
    for(QObject* o : widget->children()) {
        if(QWidget* w = dynamic_cast<QWidget*>(o)) {
            sliders.push_back(dynamic_cast<QSlider*>(w));
            rawLabels.push_back(dynamic_cast<T*>(w));
            if(! sliders.back())
                sliders.pop_back();
            if(! rawLabels.back())
                rawLabels.pop_back();
            addChildren(sliders, rawLabels, w);
        }
    }
}

namespace mapvari {

template <typename T>
void general(T & value, QSlider* slider) {
    value = slider->value();
    QObject::connect(slider, &QSlider::valueChanged, [&value](int x){
        value = x;
    });
}

template <typename T, typename F>
void general(T & value, QSlider* slider, F f) {
    value = f(slider->value());
    QObject::connect(slider, &QSlider::valueChanged, [&value, f](int x){
        value = f(x);
    });
}

template <typename T>
void linear(T & value, QSlider* slider, T constant = 1) {
    general(value, slider, [constant](int x) {
        return constant * x;
    });
}

template <typename T>
void expo(T & value, QSlider* slider, T base = 2, T constant = 1) {
    general(value, slider, [base, constant](int x) {
        return constant * std::pow(base, x);
    });
}

}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tabWidget->setCurrentIndex(0);

    QList<MyLabel*> rawLabels;

    addChildren(sliders, rawLabels, ui->controls);

    QList<MyLabel*> labels;
    for(QSlider* slider : sliders) {
        MyLabel* savedLabel = nullptr;
        QMutableListIterator<MyLabel*> it(rawLabels);
        while(it.hasNext()) {
            MyLabel* curLabel = it.next();
            if(curLabel->objectName().startsWith(slider->objectName())) {
                if(savedLabel == nullptr) {
                    savedLabel = curLabel;
                    it.remove();
                } else {
                    throw std::invalid_argument("too much labels for " + savedLabel->objectName().toStdString());
                }
            }
        }
        if(savedLabel == nullptr)
            throw std::invalid_argument("no label for " + slider->objectName().toStdString());
        labels.append(savedLabel);
        defaultSliderValues.append(slider->value());
    }

    if(rawLabels.size())
        throw std::invalid_argument("there exist some labels without slider");

    if(sliders.size() != labels.size())
        throw std::invalid_argument("label size != slider size");

    for(MyLabel* label : labels)
        label->saveFormat();

    // link ui and scene

    const float RPM = 1 / 60.0,
                ANGLE = radians(1),
                CM = 1/100.0;

    Scene* scene = getScene();

    mapvari::linear(scene->lightHeight, ui->lightHeight, CM);
    mapvari::linear(scene->lightRadius, ui->lightRadius, CM);
    mapvari::linear(scene->lightSpeed, ui->lightSpeed, RPM);
    mapvari::linear(scene->lightInitPos, ui->lightInitPos, ANGLE);

    mapvari::linear(scene->angleFromUp, ui->toUp, ANGLE);
    mapvari::linear(scene->angleOnGround, ui->onGround, ANGLE);
    mapvari::linear(scene->length, ui->cameraR, CM);
    mapvari::expo(scene->chessShininess, ui->shininess, 2.f);
    mapvari::general(scene->lightColorsParam, ui->lightColors);

    auto convertAngle = [ANGLE](float rad) {
        return (((int)(rad / ANGLE)) % 360 + 360) % 360;
    };

    // link gl and ui
    connect(ui->gl, &MyGLDrawer::paramChanged, [this, scene, CM, convertAngle](){
        ui->cameraR->setValue((int)(scene->length / CM));
        ui->onGround->setValue(convertAngle(scene->angleOnGround));
        ui->toUp->setValue(convertAngle(scene->angleFromUp));
    });

    mapvari::linear(scene->nLights, ui->nLights, 1);

    auto it = labels.begin();
    for(QSlider* slider : sliders) {
        auto label = *it++;
        int def = slider->value();
        label->formatInt(def);
        connect(slider, &QSlider::valueChanged, label, &MyLabel::formatInt);
        connect(label, &MyLabel::clicked, [slider, def](){
            slider->setValue(def);
        });
    }

    connect(ui->defaultButton, &QPushButton::clicked, [this](){
        QListIterator<int> it(defaultSliderValues);
        for(QSlider* s : sliders)
            s->setValue(it.next());
    });

    connect(ui->helpButton, &QPushButton::clicked, [this](){

        if(modal) {
            modal->close();
            modal = nullptr;
        } else {
            QMessageBox* msgBox = new QMessageBox( this );
            modal = msgBox;
            msgBox->setAttribute( Qt::WA_DeleteOnClose ); //makes sure the msgbox is deleted automatically when closed
            msgBox->setStandardButtons( QMessageBox::Ok );
            msgBox->setWindowTitle( tr("Error") );
            msgBox->setText(
                "<h1>Fancy chessboard</h1>"
                "<p>By Robert VANDEN EYNDE</p>"
                "<ul>"
                    "<li><strong>Left click</strong> and move to turn the camera (the camera is in spherical coordinates).</li>"
                    "<li><strong>Right click</strong> and move to translate the point the camera is looking at.</li>"
                    "<li><strong>Wheel</strong> to zoom in/out.</li>"
                    "<li>There are multiple parameters in <strong>multiples tabs</strong></li>"
                    "<li>Use the <strong>wheel</strong> on a slider to change by 1 step</li>"
                    "<li><strong>Hover</strong> the parameter name with the mouse to get a verbose explanation.</li>"
                    "<li><strong>Click</strong> the parameter name to reset to default value</li>"
                "</ul>"
            );
            // msgBox->setModal( false );
            connect(msgBox, &QWidget::destroyed, [this](){
                modal = nullptr;
            });
            msgBox->setWindowModality(Qt::NonModal);
            msgBox->show();
        }
    });

    showMaximized();
}

MainWindow::~MainWindow()
{
    delete ui;
}

Scene *MainWindow::getScene() {
    return ui->gl->getScene();
}
