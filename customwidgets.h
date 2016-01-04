#ifndef CUSTOMWIDGETS_H
#define CUSTOMWIDGETS_H

#include <QLabel>

class FormatLabel : public QLabel
{
    // Q_OBJECT

public:
    FormatLabel(QWidget* parent) : QLabel(parent) {}

    void setFormat(QString s) { myFormat = s; }
    void saveFormat() { setFormat(text()); }

public:
    void formatInt(int value);

private:
    QString myFormat;
};


#endif // CUSTOMWIDGETS_H
