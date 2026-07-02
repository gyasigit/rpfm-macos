#ifndef KMESSAGE_WIDGET_H
#define KMESSAGE_WIDGET_H

#include "qt_subclasses_global.h"
#ifdef RPFM_NO_KDE
#include <QLabel>
#else
#ifdef _WIN32
#include <KF6/KWidgetsAddons/KMessageWidget>
#else
#include <KMessageWidget>
#endif
#endif
#include <QWidget>
#include <QString>

extern "C" QWidget* kmessage_widget_new(QWidget* widget = nullptr);
extern "C" void kmessage_widget_close(QWidget* widget = nullptr);
extern "C" bool kmessage_widget_is_closed(QWidget* widget = nullptr);
extern "C" void kmessage_widget_set_error(QWidget* widget = nullptr, QString const text = "");
extern "C" void kmessage_widget_set_warning(QWidget* widget = nullptr, QString const text = "");
extern "C" void kmessage_widget_set_info(QWidget* widget = nullptr, QString const text = "");
#endif // KMESSAGE_WIDGET_H
