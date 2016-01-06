#ifndef CUSTOMWIDGETS_H
#define CUSTOMWIDGETS_H

#include <QLabel>

class FormatLabel : public QLabel {
    Q_OBJECT

public:
    FormatLabel(QWidget* parent = 0) : QLabel(parent) {}

    void setFormat(QString s) { myFormat = s; }
    void saveFormat() { setFormat(text()); }

signals:
    void clicked();

public:
    void formatInt(int value);

protected:
    void mousePressEvent(QMouseEvent* ev) override;

private:
    QString myFormat;
};

/*
class FormatLabel : public QLabel
{
    Q_OBJECT

public:
    FormatLabel(QWidget* parent) : QLabel(parent) {}

    void setFormat(QString s) { myFormat = s; }
    void saveFormat() { setFormat(text()); }

signals:
    void clicked();

public:
    void formatInt(int value);

protected:
    void mousePressEvent(QMouseEvent *ev);
private:
    QString myFormat;
};
*/


#endif // CUSTOMWIDGETS_H
