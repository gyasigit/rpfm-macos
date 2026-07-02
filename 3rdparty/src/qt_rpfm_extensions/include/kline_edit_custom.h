#ifndef KLINE_EDIT_CUSTOM_H
#define KLINE_EDIT_CUSTOM_H

#include "qt_subclasses_global.h"
#ifdef RPFM_NO_KDE
#include <QLineEdit>
#else
#ifdef _WIN32
#include <KF6/KCompletion/KLineEdit>
#else
#include <KLineEdit>
#endif
#endif
#include <QWidget>
#include <QColor>

extern "C" void kline_edit_configure(QWidget* view = nullptr);

#endif // KLINE_EDIT_CUSTOM_H
