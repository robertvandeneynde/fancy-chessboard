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

template <typename T, typename F>
void linear(T & value, QSlider* slider, T constant, F f) {
    general(value, slider, [constant, f](int x) {
        return f(constant * x);
    });
}

template <typename T>
void expo(T & value, QSlider* slider, T base = 2, T constant = 1) {
    general(value, slider, [base, constant](int x) {
        return constant * std::pow(base, x);
    });
}

}

MainWindow::MainWindow(QWidget* parent) :
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
                DM = 1/10.0;

    auto decimalFormat = [](QString f, int x){
        return x >= 0 ? f.arg(QString("%1.%2").arg(x/10).arg(x%10)) :
                      f.arg(QString("-%1.%2").arg((-x)/10).arg((-x)%10));
    };
    Scene* scene = getScene();

    mapvari::linear(scene->lightHeight, ui->lightHeight, DM);
    mapvari::linear(scene->lightRadius, ui->lightRadius, DM);
    mapvari::linear(scene->lookAt[0], ui->panX, DM);
    mapvari::linear(scene->lookAt[1], ui->panY, DM);
    ui->lightHeightLabel->setFunc(decimalFormat);
    ui->lightRadiusLabel->setFunc(decimalFormat);
    ui->panXLabel->setFunc(decimalFormat);
    ui->panYLabel->setFunc(decimalFormat);

    mapvari::linear(scene->lightSpeed, ui->lightSpeed, RPM);
    mapvari::linear(scene->lightInitPos, ui->lightInitPos, ANGLE);

    mapvari::linear(scene->angleFromUp, ui->toUp, ANGLE);
    mapvari::linear(scene->angleOnGround, ui->onGround, ANGLE);

    mapvari::linear(scene->length, ui->cameraR, DM);
    ui->cameraRLabel->setFunc(decimalFormat);

    mapvari::expo(scene->chessShininess, ui->shininess, 2.f);

    mapvari::linear(scene->anim.duration, ui->animDuration, DM);
    ui->animDurationLabel->setFunc(decimalFormat);

    mapvari::linear(scene->movementWaiting, ui->movementWaiting, DM);
    ui->movementWaitingLabel->setFunc(decimalFormat);

    mapvari::general(scene->lightColorsParam, ui->lightColors);
    mapvari::general(scene->anim.mode, ui->animMode);
    mapvari::general(scene->onKnightAnim.setRunning, ui->animOnKnight);

    mapvari::linear(scene->falling.alpha, ui->fallingAlpha, DM);
    ui->fallingAlphaLabel->setFunc(decimalFormat);

    mapvari::linear(scene->falling.g, ui->fallingGravity);
    mapvari::linear(scene->falling.k, ui->fallingK);
    mapvari::general(scene->falling.timeCutOff, ui->fallingMaxT, [this](int x){
        return x == ui->fallingMaxT->maximum() ? 1e6 : x;
    });
    ui->fallingMaxTLabel->setFunc([this](QString s, int x) -> QString {
        return x == ui->fallingMaxT->maximum() ? s.arg("∞") : s.arg(x);
    });

    connect(ui->buttonBoing, &QPushButton::clicked, [this, scene](){
        scene->falling.start(ui->gl->currentTime());
    });

    ui->animModeLabel->setFunc([scene](QString format, int x){
        x = std::max(1,x);
        return format.arg(x % 2 == 1 ? "p3" : "p4").arg(scene->anim.mode.getHeight(x));
    });

    auto convertAngle = [ANGLE](float rad) {
        return (((int)(rad / ANGLE)) % 360 + 360) % 360;
    };

    // link gl and ui
    connect(ui->gl, &MyGLDrawer::paramChanged, [this, scene, DM, convertAngle](){
        ui->cameraR->setValue((int)(scene->length / DM));
        ui->onGround->setValue(convertAngle(scene->angleOnGround));
        ui->toUp->setValue(convertAngle(scene->angleFromUp));
        ui->panX->setValue((int)(scene->lookAt[0]/DM));
        ui->panY->setValue((int)(scene->lookAt[1]/DM));
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
                    "<li>The pieces and the ground have <strong>shininesss</strong></li>"
                    "<li>The ground has multiple lights and a <strong>bump map</strong></li>"
                    "<li>The normals of the bmp map is an image generated by a <strong>python code</strong></li>"
                    "<li>Wait until the knights move. You will see nice <strong>bezier curves</strong>. Tab 2 can change the degree (3 points or 4 points) and the height.</li>"
                    "<li>You like movement ? Climb on the <strong>back</strong> of the moving knight ! On tab 2)</li>"
                    "<li>You missed the boing-boing apparition ? Click on <strong>boing</strong> !</li>"
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

    QStringList params = QApplication::arguments().mid(1);
    bool setdefault = true;
    if(params.length() >= 5 && params[0] == "rec") {
        int p[4];
        bool ok = true;
        for(int i = 0; i < 4; i++) {
            p[i] = params[1+i].toInt(&ok);
            if(! ok)
                break;
        }
        if(ok) {
            resize(p[2], p[3]);
            move(p[0], p[1]);
        }
        setdefault = ok;
    }
    if(setdefault) {
        showMaximized();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

Scene *MainWindow::getScene() {
    return ui->gl->getScene();
}
