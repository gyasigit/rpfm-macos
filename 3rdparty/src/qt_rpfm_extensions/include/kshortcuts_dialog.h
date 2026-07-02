#ifndef KSHORTCUTS_DIALOG_H
#define KSHORTCUTS_DIALOG_H

#include "qt_subclasses_global.h"
#ifdef RPFM_NO_KDE
#include <QAction>
#include <QIcon>
#include <QKeySequence>
#include <QList>
#include <QMessageBox>
#include <QObject>
#else
#ifdef _WIN32
#include <KF6/KXmlGui/KShortcutsDialog>
#include <KF6/KTextEditor/KTextEditor/Document>
#include <KF6/KTextEditor/KTextEditor/Editor>
#include <KF6/KTextEditor/KTextEditor/View>
#else
#include <KShortcutsDialog>
#include <KTextEditor/Document>
#include <KTextEditor/Editor>
#include <KTextEditor/View>
#endif
#endif
#include <QWidget>
#include <QString>
#include <QAction>

#endif // KSHORTCUTS_DIALOG_H
