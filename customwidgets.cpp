#include "customwidgets.h"

void FormatLabel::formatInt(int x) {
    setText(myFormat.arg(x));
}
