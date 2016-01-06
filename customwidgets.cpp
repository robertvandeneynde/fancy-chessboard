#include "customwidgets.h"

#include <QDebug>


void FormatLabel::formatInt(int x) {
    setText(withFunc ? func(myFormat, x) : myFormat.arg(x));
}

void FormatLabel::mousePressEvent(QMouseEvent *ev) {
    emit clicked();
}
