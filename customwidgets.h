#ifndef CUSTOMWIDGETS_H
#define CUSTOMWIDGETS_H

#include <QLabel>

class IntFormatLabel : public QLabel
{
public:
    IntFormatLabel(QWidget* parent = 0) : QLabel(parent) {}

    void saveFormat() {
        myFormat = text();
    }

    void format(int x) {
        setText(myFormat.arg(x));
    }

private:
    QString myFormat;
};

#endif // CUSTOMWIDGETS_H
