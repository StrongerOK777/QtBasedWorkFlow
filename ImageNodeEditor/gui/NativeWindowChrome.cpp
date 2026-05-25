#include "gui/NativeWindowChrome.h"

#include <QtGlobal>

#ifndef Q_OS_MACOS

namespace NativeWindowChrome {

void configure(QWidget* window)
{
    Q_UNUSED(window);
}

}

#endif
