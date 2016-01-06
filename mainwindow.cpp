#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "utils.h"

#include <stdexcept>
#include <QMessageBox>

static void addChildren(QList<QSlider*> & sliders, QList<FormatLabel*> & rawLabels, QWidget* widget) {
    for(QObject* o : widget->children()) {
        if(QWidget* w = dynamic_cast<QWidget*>(o)) {
            sliders.push_back(dynamic_cast<QSlider*>(w));
            rawLabels.push_back(dynamic_cast<FormatLabel*>(w));
            if(! sliders.back())
                sliders.pop_back();
            if(! rawLabels.back())
                rawLabels.pop_back();
            addChildren(sliders, rawLabels, w);
        }
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tabWidget->setCurrentIndex(0);

    QList<FormatLabel*> rawLabels;

    addChildren(sliders, rawLabels, ui->controls);

    QList<FormatLabel*> labels;
    for(QSlider* slider : sliders) {
        FormatLabel* savedLabel = nullptr;
        QMutableListIterator<FormatLabel*> it(rawLabels);
        while(it.hasNext()) {
            FormatLabel* curLabel = it.next();
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

    for(FormatLabel* label : labels)
        label->saveFormat();

    // link ui and scene
    auto map = [](float & value, QSlider* slider, float constant) {
        value = slider->value() * constant;
        connect(slider, &QSlider::valueChanged, [&value, constant](int x){
            value = x * constant;
        });
    };

    auto mapInt = [](int & value, QSlider* slider, int constant) {
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
    map(scene->lightInitPos, ui->lightInitPos, ANGLE);

    map(scene->angleFromUp, ui->toUp, ANGLE);
    map(scene->angleOnGround, ui->onGround, ANGLE);
    map(scene->length, ui->cameraR, CM);

    auto convertAngle = [ANGLE](float rad) {
        return (((int)(rad / ANGLE)) % 360 + 360) % 360;
    };

    // link gl and ui
    connect(ui->gl, &MyGLDrawer::paramChanged, [this, scene, CM, convertAngle](){
        ui->cameraR->setValue((int)(scene->length / CM));
        ui->onGround->setValue(convertAngle(scene->angleOnGround));
        ui->toUp->setValue(convertAngle(scene->angleFromUp));
    });

    mapInt(scene->nLights, ui->nLights, 1);

    auto it = labels.begin();
    for(QSlider* slider : sliders) {
        FormatLabel* label = *it++;
        label->formatInt(slider->value());
        connect(slider, &QSlider::valueChanged, label, &FormatLabel::formatInt);
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
                    "<li><strong>Left click</strong> to turn the camera.</li>"
                    "<li><strong>Right click</strong> to translate the point the camera is looking at.</li>"
                    "<li>There are multiple parameters in <strong>multiples tabs</strong>, hover the parameter name with the mouse to get a verbose explanation.</li>"
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
